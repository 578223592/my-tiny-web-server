// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "CurrentThread.h"
#include "Logging.h"
#include "Thread.h"
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
using namespace std;

//暂时理解：一个EventLoop就是一个reactor响应模型的具体实现
class EventLoop {
 public:
  typedef std::function<void()> Functor;
  EventLoop();
  ~EventLoop();
  void loop();
  void quit();
  // 如果当前线程就是创建此EventLoop的线程 就调⽤callback(关闭连接 EpollDel) 否则就放⼊等待执⾏函数区
  void runInLoop(Functor&& cb);
  // 把此函数放⼊等待执⾏函数区 如果当前是跨线程 或者正在调⽤等待的函数则唤醒
  void queueInLoop(Functor&& cb);
  /**
   * 判断当前线程是否是创建EventLoop的线程
   * @return
   */
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  void assertInLoopThread() { assert(isInLoopThread()); }
  void shutdown(shared_ptr<Channel> channel) { shutDownWR(channel->getFd()); }
  void removeFromPoller(shared_ptr<Channel> channel) {
    // shutDownWR(channel->getFd());
    poller_->epoll_del(channel);
  }
  void updatePoller(shared_ptr<Channel> channel, int timeout = 0) {
    poller_->epoll_mod(channel, timeout);
  }
  void addToPoller(shared_ptr<Channel> channel, int timeout = 0) {
    poller_->epoll_add(channel, timeout);
  }

 private:
  // 声明顺序 wakeupFd_ > pwakeupChannel_
  bool looping_;
  shared_ptr<Epoll> poller_;
  int wakeupFd_;
  bool quit_;
  bool eventHandling_;
  mutable MutexLock mutex_;
  std::vector<Functor> pendingFunctors_;
  bool callingPendingFunctors_;
  const pid_t threadId_;
  shared_ptr<Channel> pwakeupChannel_;

  void wakeup();
  void handleRead();
  void doPendingFunctors();
  void handleConn();
};
