// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"

//Poller 类的作⽤就是负责监听⽂件描述符事件是否触发以及返回发⽣事件的⽂件描述符以及具体事件。所以⼀个
//        Poller 对象对应⼀个 IO 多路复⽤模块。在 muduo 中，⼀个 EventLoop 对应⼀个 Poller 。
class Epoll {
 public:
  Epoll();
  ~Epoll();
  void epoll_add(SP_Channel request, int timeout);
  void epoll_mod(SP_Channel request, int timeout);
  void epoll_del(SP_Channel request);
  std::vector<std::shared_ptr<Channel>> poll();
  std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
  void add_timer(std::shared_ptr<Channel> request_data, int timeout);
  int getEpollFd() { return epollFd_; }
  void handleExpired();

 private:
  static const int MAXFDS = 100000;
  int epollFd_;
  std::vector<epoll_event> events_;   // epoll_wait()返回的活动事件都放在这个数组
//  std::shared_ptr<Channel> fd2chan_[MAXFDS];   //没用map保存，直接用的数组，感觉不是特别优雅，有一些内存浪费
  std::unordered_map<int,std::shared_ptr<Channel>> channelMap_;  //替代fd2chan_
  std::shared_ptr<HttpData> fd2http_[MAXFDS];   //持有用来保证HttpData计数不为0，不被析构，也是因此本类负责HttpData的销毁
  std::unordered_map<int,std::shared_ptr<HttpData>> httpMap_;
  TimerManager timerManager_;
};