// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <unistd.h>

#include <deque>
#include <memory>
#include <queue>

#include "../base/include/MutexLock.h"
#include "../base/include/noncopyable.h"
#include "HttpData.h"

class HttpData;

class TimerNode {
 public:
  TimerNode(std::shared_ptr<HttpData> requestData, int timeout);
  /**
   * 定时器到期，这里要负责关闭连接。
   */
  ~TimerNode();
  TimerNode(TimerNode &tn);
  void update(int timeout);
  /**
   * 判断一个节点是否应该超时，如果超时则应该被删除，那么返回false，并设置deleted_为true
   * @return 该节点是否应该被删除
   */
  bool isValid();
  void clearReq();
  void setDeleted() { deleted_ = true; }
  bool isDeleted() const { return deleted_; }  //只有isvalid函数和clearReq会将 clearReq设置为true
  size_t getExpTime() const { return expiredTime_; }

 private:
  bool deleted_;
  size_t expiredTime_; //定时器的过期时间
  std::shared_ptr<HttpData> SPHttpData;
};

struct TimerCmp {
  bool operator()(std::shared_ptr<TimerNode> &a,
                  std::shared_ptr<TimerNode> &b) const {
    return a->getExpTime() > b->getExpTime();
  }
};

class TimerManager {
 public:
  TimerManager();
  ~TimerManager();
  void addTimer(const std::shared_ptr<HttpData>& SPHttpData, int timeout);
  void handleExpiredEvent();

 private:
  typedef std::shared_ptr<TimerNode> SPTimerNode;
  std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp>
      timerNodeQueue;
  // MutexLock lock;
};