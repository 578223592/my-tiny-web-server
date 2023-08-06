// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include "Timer.h"
#include <functional>
#include <memory>
#include <string>
#include <sys/epoll.h>
#include <unordered_map>

class EventLoop;
class HttpData;

class Channel {
private:
  /**
   * CallBack是一个typedef
   */
  typedef std::function<void()>
      CallBack; // std::function 是 C++
                // 标准库中的一个模板类，用于封装可以被调用的任意可调用对象（函数、函数指针、成员函数指针等）。它提供了一种通用的方式来存储、传递和使用可调用对象，类似于函数指针的概念，但更加灵活和类型安全。
  EventLoop *loop_;
  int fd_;
  __uint32_t events_;
  // epoll收到的事件
  __uint32_t revents_;
  __uint32_t lastEvents_;

  // 方便找到上层持有该Channel的对象，weak_ptr相当于一个观测者
  //  如果不是weak_ptr可能会有循环引用的问题
  std::weak_ptr<HttpData> holder_;

private:
  int parse_URI();
  int parse_Headers();
  int analysisRequest();

  CallBack readHandler_;
  CallBack writeHandler_;
  CallBack errorHandler_;
  CallBack connHandler_;

public:
  Channel(EventLoop *loop);
  Channel(EventLoop *loop, int fd);
  ~Channel();
  int getFd();
  void setFd(int fd);

  void setHolder(std::shared_ptr<HttpData> holder) { holder_ = holder; }
  std::shared_ptr<HttpData> getHolder() {
    std::shared_ptr<HttpData> ret(holder_.lock());
    return ret;
  }

  void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
  void setWriteHandler(CallBack &&writeHandler) {
    writeHandler_ = writeHandler;
  }
  void setErrorHandler(CallBack &&errorHandler) {
    errorHandler_ = errorHandler;
  }
  void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }

  void handleEvents() {
    events_ = 0;
    // 触发挂起事件 并且没触发可读事件
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
      events_ = 0;
      return;
    }
    // 触发错误事件
    if (revents_ & EPOLLERR) {
      if (errorHandler_)
        errorHandler_();
      events_ = 0;
      return;
    }
    // 触发可读事件 | ⾼优先级可读 | 对端（客户端）关闭连接
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      handleRead();
    }
    // 触发可写事件
    if (revents_ & EPOLLOUT) {
      handleWrite();
    }
    handleConn();
  }
  void handleRead();
  void handleWrite();
  void handleError(int fd, int err_num, std::string short_msg);
  void handleConn();

  void setRevents(__uint32_t ev) { revents_ = ev; }

  void setEvents(__uint32_t ev) { events_ = ev; }
  __uint32_t &getEvents() { return events_; }

  bool EqualAndUpdateLastEvents() { // 这个什么作用呢？？？
    bool ret = (lastEvents_ == events_);
    lastEvents_ = events_;
    return ret;
  }

  __uint32_t getLastEvents() { return lastEvents_; }
};

typedef std::shared_ptr<Channel> SP_Channel;