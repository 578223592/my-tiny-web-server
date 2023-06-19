// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include "Condition.h"
#include "MutexLock.h"
#include "Thread.h"
#include "noncopyable.h"
#include "EventLoop.h"


//对 EventLoop 进⾏封装，让每个 Thread 中运⾏ EventLoop 的 Loop。
class EventLoopThread : noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();
  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
};