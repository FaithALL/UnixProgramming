# UnixProgramming

> 这是我学习Unix(主要是linux)系统编程与网络编程的笔记，附上部分源码

### 系统编程

* 进程间通信：
  * 匿名管道`pipe`，命名管道(FIFO)`mkfifo`，UNIX域套接字
* 文件I/O
  * 基本I/O：`open`，`read`，`write`，`lseek`，`close`，`dup2/dup`，`sync`，`fcntl`
  * 高级I/O：分散读集中写`readv/writev`，存储映射`mmap/munmap`，linux零拷贝`sendfile/splice/tee`
  * 标准I/O
* 文件其他操作
  * 获取文件信息：`stat`
  * 访问权限：`access`，`umask`，`chmod`
  * 所有者：`chown`
  * 目录操作：`mkdir`，`opendir`，`readdir`，`closedir`，`rmdir`
  * 文件操作：`truncate`，`rename`(目录也适用)
  * 硬链接：`link`，`unlink`
  * 符号链接：`symlink`，`readlink`

### 网络编程

* socket
  * 4个字节序转换函数`htonl`、`htons` 、`ntohl`、`ntohs`(n：network     h：host)

  * 3种常用socket地址格式`sockaddr`、`sockaddr_in`、`sockaddr_in6`

  * 2个ip地址转换(书写型和计算机使用型)函数`inet_ntop`、`inet_pton` (适用ipv4和ipv6)

  * tcp服务端：`socket`、`bind`、`listen`、`accept`、`close/shutdown`

    tcp客户端：`socket`、`connect`、`close/shutdown`

  * 4组读写函数`read/write`、`recv/send`、`recvfrom/sendto`、`recvmsg/sendmsg`

  * 获取socket绑定地址`getsockname`、`getpeername`

  * socket选项`setsockopt`、`getsockopt`

  * 带外数据`sockatmark`

  * 地址查询函数`getaddrinfo`、`freeaddrinfo`，`getnameinfo`

