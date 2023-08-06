# WebBench

Webbench是一个在linux下使用的非常简单的网站压测工具。它使用fork()模拟多个客户端同时访问我们设定的URL，测试网站在压力下工作的性能，最多可以模拟3万个并发连接去测试网站的负载能力。

## 依赖

ctags

## 编译：

    sudo make && sudo make install PREFIX=your_path_to_webbench

或者

```
#cd  your_path_to_webbench
./build.sh
```

## 命令行选项：

运行教程：[Web性能压力测试工具之WebBench详解 - 简书](https://www.jianshu.com/p/97e4d312e83d)

```
webbench -c 1000 -t 60 http://192.168.80.157/phpinfo.php
```



| 短参    | 长参数                   | 作用             |
| ----- |:---------------------:| --------------:|
| -f    | --force               | 不需要等待服务器响应     |
| -r    | --reload              | 发送重新加载请求       |
| -t    | --time <sec>          | 运行多长时间，单位：秒"   |
| -p    | --proxy <server:port> | 使用代理服务器来发送请求   |
| -c    | --clients <n>         | 创建多少个客户端，默认1个" |
| -9    | --http09              | 使用 HTTP/0.9    |
| -1    | --http10              | 使用 HTTP/1.0 协议 |
| -2    | --http11              | 使用 HTTP/1.1 协议 |
|       | --get                 | 使用 GET请求方法     |
|       | --head                | 使用 HEAD请求方法    |
|       | --options             | 使用 OPTIONS请求方法 |
|       | --trace               | 使用 TRACE请求方法   |
| -?/-h | --help                | 打印帮助信息         |
| -V    | --version             | 显示版本号          |

## ubuntu安装

如果你运行命令提示找不到rpc/types.h，如下：

```bash
(base) ➜  WebBench git:(main) ✗ ./build.sh                                                          
cc -Wall -ggdb -W -O   -c -o webbench.o webbench.c
webbench.c:22:10: fatal error: rpc/types.h: 没有那个文件或目录
   22 | #include <rpc/types.h>
      |          ^~~~~~~~~~~~~
compilation terminated.
make: *** [<内置>：webbench.o] 错误 1

```

网上很多命令都不行，需要执行

```
sudo apt install libntirpc-dev
sudo ln -s /usr/include/tirpc/rpc/types.h /usr/include/rpc
sudo ln -s /usr/include/tirpc/netconfig.h /usr/include


```

参考：[链接](https://askubuntu.com/questions/1360945/fatal-error-rpc-rpc-h-no-such-file-or-directory)和[centos8安装webbench遇到的错误 webbench.c:22:10: 致命错误：rpc/types.h：没有那个文件或目录_webbench.c:21:10: fatal error: rpc/types.h: 没有那个文件_dxgzg的博客-CSDN博客](https://blog.csdn.net/dxgzg/article/details/120913307)、[RPC 问题记录_rpc unsupported protocol_Sensente的博客-CSDN博客](https://blog.csdn.net/sensente/article/details/127480795)
