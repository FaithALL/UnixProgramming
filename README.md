# UnixProgramming

> 这是我学习Unix(主要是linux)系统编程与网络编程的笔记，附上部分源码

### 系统编程

### 网络编程

* 4个字节序转换函数`htonl`、`htons` 、`ntohl`、`ntohs`(n：network     h：host)

* 3种常用socket地址格式`sockaddr`、`sockaddr_in`、`sockaddr_in6`

* 2个ip地址转换(书写型和计算机使用型)函数`inet_ntop`、`inet_pton` (适用ipv4和ipv6)

* tcp服务端：`socket`、`bind`、`listen`、`accept`、`close/shutdown`

  tcp客户端：`socket`、`connect`、`close/shutdown`

* 4组读写函数`read/write`、`recv/send`、`recvfrom/sendto`、`recvmsg/sendmsg`

* 获取socket绑定地址`getsockname`、`getpeername`

* socket选项`setsockopt`、`getsockopt`

* 带外数据`sockatmark`

* 地址查询函数`getaddrinfo`、`getnameinfo`

