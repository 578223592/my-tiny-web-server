// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "EventLoopThread.h"

#include <functional>

EventLoopThread::EventLoopThread()
    : loop_(nullptr),
      exiting_(false),
      thread_([this] { threadFunc(); }, "EventLoopThread"),
      mutex_(),
      cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();  //开始执行创建的时候传入的函数，对EventLoop来说是&EventLoopThread::threadFunc, ，即开始监听
  {
    MutexLockGuard lock(mutex_);
    // 一直等到threadFun在Thread里真正跑起来
    //本质上就是等到函数真正跑起来，因为初始化loop为null，真正跑起来的时候才会在EventLoopThread中创建loop，即在EventLoopThread::threadFunc()中
    while (loop_ == nullptr) cond_.wait();
  }
  return loop_;
}
//子线程的作用就是不断的loop
//创建loop 并开始loop
void EventLoopThread::threadFunc() {
  EventLoop loop;

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();
  // assert(exiting_);
  loop_ = nullptr;
}