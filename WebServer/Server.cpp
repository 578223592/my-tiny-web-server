// @Author swx
// @Email
#include "./include/Server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>

#include "Logging.h"
#include "Util.h"
Server::Server(EventLoop *loop, int threadNum, int port)
    : loop_(loop),
      threadNum_(threadNum),
      eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
      started_(false),
      acceptChannel_(new Channel(loop_)),
      port_(port),
      listenFd_(socket_bind_listen(port)) {   //尽量不要让列表依赖于顺序
  init_MemoryPool();
  acceptChannel_->setFd(listenFd_);  //
  handle_for_sigpipe();
  if (setSocketNonBlocking(listenFd_) < 0) {  //设置成et之后要注意设置socket为非阻塞，否则会导致卡死的问题
    perror("set socket non block failed");
    abort();
  }
}

void Server::start() {
  eventLoopThreadPool_->start();      //这里面就已经开启线程池了
  // acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);  //设置epollin和边缘触发模式
  acceptChannel_->setReadHandler([this] { handNewConn(); });
  acceptChannel_->setConnHandler([this] { handThisConn(); });
  loop_->addToPoller(acceptChannel_, 0);   //就是说为什么要搞一个初始化，然后又搞一个run呢，因为run之后程序才真正开始执行，比如这里
  //一旦addToPoller之后就会开始监听，服务器就开始运行了，因此这两者可以分开
  started_ = true;
}

void Server::handNewConn() {
  struct sockaddr_in client_addr{};
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;
  while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {
    EventLoop *loop = eventLoopThreadPool_->getNextLoop();  //分发给子线程的loop
    LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);
    // cout << "new connection" << endl;
    // cout << inet_ntoa(client_addr.sin_addr) << endl;
    // cout << ntohs(client_addr.sin_port) << endl;
    /*
    // TCP的保活机制默认是关闭的
    int optval = 0;
    socklen_t len_optval = 4;
    getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
    cout << "optval ==" << optval << endl;
    */
    // 限制服务器的最大并发连接数
    if (accept_fd >= MAXFDS) {
      close(accept_fd);
      continue;
    }
    // 设为非阻塞模式
    if (setSocketNonBlocking(accept_fd) < 0) {
      LOG << "Set non block failed!";
      // perror("Set non block failed!");
      return;
    }
    setSocketNodelay(accept_fd);  //关闭tcp的nagle算法
    // setSocketNoLinger(accept_fd);
//    setSocketNodelay 函数用于设置TCP连接是否启用Nagle算法。Nagle算法是一种优化TCP网络传输的算法，通过延迟发送小数据包来减少网络传输的次数，从而提高传输效率。但是，这种算法会导致网络延迟增加，特别是在小数据量传输时，会导致网络性能明显下降。因此，在某些情况下，关闭Nagle算法可以提高网络传输的效率。一般而言，在数据量较大的情况下（如传输文件），开启Nagle算法可以减少网络传输次数，提高效率；而在数据量较小的情况下（如传输短消息），关闭Nagle算法可以提高网络传输效率。所以，具体是否开启Nagle算法要根据具体场景做出合理的决策。
//    setSocketNoLinger 函数用于设置socket连接关闭的方式，它的作用是控制调用close关闭socket的方式。当启用SO_LINGER选项时，close函数会阻塞一段时间（由SO_LINGER选项指定的时间），以等待数据发送或接收完成。如果timeout时间到了仍未完成，那么close会强制关闭socket，这样可以保证数据正常传输。如果不启用SO_LINGER选项，则close函数会立即返回，但数据可能没有发送或接收完成。不过，在一般情况下，SO_LINGER选项并不推荐在accept套接字中被使用，因为这会导致在一个套接字关闭时，内核仍的无法释放相应的所有资源，从而导致内存泄漏问题。

//    shared_ptr<HttpData>  req_info = make_shared<HttpData>(loop, accept_fd);
    shared_ptr<HttpData> req_info(newElement<HttpData>(loop, accept_fd),deleteElement<HttpData>);
//    shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
    req_info->getChannel()->setHolder(req_info);   //todo 这里好像有循环引用的问题
    loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));   //为何调用这个就可以把http事件注册到子reactor（线程）呢
  }
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}