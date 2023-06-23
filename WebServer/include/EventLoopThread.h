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
  /**
   * //开始执行监听，会和  void threadFunc();  联动，void threadFunc();里面会创建loop
   * 而这里会等待loop创建才会开始loop
   * @return
   */
  EventLoop* startLoop();

 private:
   /**
    * 这个函数会传递给thread_，调用Thread::start()真正会执行的其实就是这个函数
    */
  void threadFunc();
  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
};