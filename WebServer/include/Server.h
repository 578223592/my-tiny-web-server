// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <memory>
#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

class Server {
 public:
  Server(EventLoop *loop, int threadNum, int port);
  ~Server() = default;
  EventLoop *getLoop() const {
    return loop_;
  }
  void start();
  void handNewConn();
  void handThisConn() {
    loop_->updatePoller(acceptChannel_);
  }

 private:
  EventLoop *loop_;
  int threadNum_;
  std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
  bool started_;
  /**
   * 主reactor监听的外部连接对应的channel，即listenFd_对应的channel
   */
  std::shared_ptr<Channel> acceptChannel_;
  int port_;
  int listenFd_;
  static const int MAXFDS = 100000;
};