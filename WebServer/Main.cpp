// @Author swx
// @Email
#include <getopt.h>

#include <string>

#include "base/include/Logging.h"
#include "include/EventLoop.h"
#include "include/Server.h"

int main(int argc, char *argv[]) {
  int threadNum = 4;
  int port = 80;
  std::string logPath = "./WebServer.log";

  // parse args
  int opt = -1;
  const char *str = "t:l:p:";
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 't': {
        threadNum = atoi(optarg);
        break;
      }
      case 'l': {
        logPath = optarg;
        if (logPath.size() < 2 || optarg[0] != '/') {
          printf("logPath should start with \"/\"\n");
          abort();
        }
        break;
      }
      case 'p': {
        port = atoi(optarg);
        break;
      }
      default:
        break;
    }
  }
  Logger::setLogFileName(logPath);
// STL库在多线程上应用
#ifndef _PTHREADS
  LOG << "_PTHREADS is not defined !";
#endif
  EventLoop mainLoop;
  Server myHTTPServer(&mainLoop, threadNum, port);
  myHTTPServer.start();  //把事件注册到loop里面，并开始监听，虽然开始监听对应的文件描述符，但是并没有开始epoll_wait对应的事件，因此下面要loop
  mainLoop.loop();    //开启loop
  return 0;
}
