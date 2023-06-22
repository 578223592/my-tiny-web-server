// @Author Lin Ya
// @Email xxbbb@vip.qq.com


#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "EventLoop.h"
#include "Logging.h"
#include "Util.h"

using namespace std;

__thread EventLoop* t_loopInThisThread = 0;

int createEventfd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      poller_(new Epoll()),
      wakeupFd_(createEventfd()),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      pwakeupChannel_(new Channel(this, wakeupFd_)) {    //todo wakeupFd对应的channel自然是pwakeupChannel_，既然有了wakeUpchannel，自然就不用保存wakeUpFd了
  if (t_loopInThisThread) {
    // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this
    // thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
  pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
  pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
  poller_->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleConn() {
  // poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET |
  // EPOLLONESHOT), 0);
  updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() {
  // wakeupChannel_->disableAll();
  // wakeupChannel_->remove();
  close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = readn(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb) {
  if (isInLoopThread())
    cb();
  else
    queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb) {
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.emplace_back(std::move(cb));
  }
// isInLoopThread()出现的原因是因为主线程(主reactor)持有线程池，线程池里面存放的EventLoop，
// !isInLoopThread() ==true 则说明当前正在主线程上
  if (!isInLoopThread() || callingPendingFunctors_) wakeup();
  //函数检查当前线程是否为事件循环所在的线程（即判断是否在事件循环线程中调用），如果不是或者当前线程正在调用待
  // 处理对象 (callingPendingFunctors_ 为真)，则调用 wakeup 函数来唤醒事件循环线程。
}
//. epoll_wait阻塞 等待就绪事件(没有注册其他fd时，可以通过event_fd来异步唤醒)
// 处理每个就绪事件
// 执⾏正在等待的函数(fd注册到epoll内核事件表)
// 处理超时事件，到期了就从定时器⼩根堆中删除
void EventLoop::loop() {
  assert(!looping_);
  assert(isInLoopThread());
  looping_ = true;
  quit_ = false;
  // LOG_TRACE << "EventLoop " << this << " start looping";
  std::vector<SP_Channel> ret;
  while (!quit_) {
    // cout << "doing" << endl;
    ret.clear();
    // 1、epoll_wait阻塞 等待就绪事件
    ret = poller_->poll(); //取出事件
    eventHandling_ = true;
    // 2、处理每个就绪事件(不同channel绑定了不同的callback)
    for (auto& it : ret) it->handleEvents(); //⽤活动事件的回调函数
    eventHandling_ = false;
    // 3、执⾏正在等待的函数(fd注册到epoll内核事件表)
    doPendingFunctors();
    // 4、处理超时事件 到期了就从定时器⼩根堆中删除(定时器析构会EpollDel掉fd)
    poller_->handleExpired();
  }
  looping_ = false;

}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) functors[i]();
  callingPendingFunctors_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}