// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "./include/Timer.h"

#include <sys/time.h>
#include <unistd.h>

#include <queue>
#include <utility>

TimerNode::TimerNode(std::shared_ptr<HttpData> requestData, int timeout)
    : deleted_(false), SPHttpData(std::move(requestData)) {
  struct timeval now{};
  gettimeofday(&now, nullptr);
  // 以毫秒计
  expiredTime_ =
      (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}
TimerNode::~TimerNode() {
  if (SPHttpData) SPHttpData->handleClose();
}

TimerNode::TimerNode(TimerNode &tn)
    : expiredTime_(0), SPHttpData(tn.SPHttpData) {}

void TimerNode::update(int timeout) {
  struct timeval now{};
  gettimeofday(&now, nullptr);
  expiredTime_ =
      (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::isValid() {
  struct timeval now{};
  gettimeofday(&now, nullptr);
  size_t timeInterval = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
  if (timeInterval < expiredTime_) {
    return true;
  } else {
    this->setDeleted();
    return false;
  }
}

void TimerNode::clearReq() {   //todo:待查看什么时候会调用这个，即为什么定时器要跟httpData分离。
  SPHttpData.reset(); //重置后，该对象的引用计数会减一
  this->setDeleted();
}

TimerManager::TimerManager() {}
TimerManager::~TimerManager() {}

void TimerManager::addTimer(const std::shared_ptr<HttpData>& SPHttpData, int timeout) {
  SPTimerNode new_node(new TimerNode(SPHttpData, timeout));
  timerNodeQueue.push(new_node);
  SPHttpData->linkTimer(new_node);
}

/* 处理逻辑是这样的~
因为(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为deleted的时间节点，会延迟到它(1)超时 或
(2)它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
(1) 第一个好处是不需要遍历优先队列，省时。
(2)
第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
*/

void TimerManager::handleExpiredEvent() {
  // MutexLockGuard locker(lock);
  while (!timerNodeQueue.empty()) {
    SPTimerNode ptimer_now = timerNodeQueue.top();
    if (ptimer_now->isDeleted() || !ptimer_now->isValid()){
      timerNodeQueue.pop();
    }else {
      break;
    }
  }
}