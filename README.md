# UnixProgramming

> 这是我学习Unix(主要是linux)系统编程与网络编程的笔记，附上部分源码

### 系统编程

* 进程
  
  * 创建`fork`
  * 执行`exec`
  * 销毁`wait/waitpid(SIGCHLD)`
  * 进程间通信：
    * 匿名管道`pipe`，命名管道(FIFO)`mkfifo`，UNIX域套接字`socketpair`
    * System V IPC(还有一组POSIX接口，更易用)
      * 信号量`semget`、`semop`、`semctl` 
      * 消息队列`msgget`、`msgsnd/msgrcv`、`msgctl`
      * 共享内存`shmget`、`shmat/shmdt`、`shmctl`
    * 信号`kill`
  
* 线程

  * 线程模型：内核线程、用户线程、双层调度
  * 线程API
    * 创建`pthread_create`
    * 初始化`pthread_once`
    * 结束`pthread_exit`
    * 回收`pthread_join`
    * 取消`pthread_cancel`、`pthread_setcancelstate`、`pthread_setcanceltype`
    * 线程属性`pthread_attr_init`、`pthread_attr_destroy`、`pthread_attr_*`
  * 线程同步
    * 互斥锁`pthread_mutex_*`、读写锁、自旋锁
    * 条件变量`pthread_cond_*`
    * 屏障
    * POSIX信号量`sem_*`
  * 线程与信号`pthread_sigmask`、`sigwait`、`pthread_kill`
  * 线程和进程`pthread_atfork`

* I/O复用
  
  > select poll轮询，epoll回调
  >
  > select poll索引就绪文件描述符复杂度为O(n)，epoll为O(1)
  >
  > epoll适用于连接数量多，活动连接少的情况
  >
  
  * select
  * poll
  * epoll：`epoll_create`，`epoll_ctl`，`epoll_wait`
  
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
  
* 信号

  * 发送信号：`kill`
  * 处理信号：`signal`，`sigaction`
  * 信号集：`sigemptyset`，`sigfillset`，`sigaddset`，`sigdelset`，`sigismember`，`sigprocmask`，`sigpending`
  * 信号的文件描述符：`signalfd`

* 编程规范
  * 日志：`syslog`，`openlog`，`setlogmask`，`closelog`
  * 用户和组：真实用户`uid`，有效用户`euid`，真实组`gid`，有效组`egid`，设置用户/组ID
  * 进程关系：进程组`pgid`，会话`sid`
  * 资源限制：`getrlimit`
  * 进程目录：`getcwd`，`chdir`，`chroot`
  * 守护进程：`daemon`

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

  * 地址查询函数`getaddrinfo`、`freeaddrinfo`，`getnameinfo`，`getnameinfo`
  
* 服务器编程框架

  * I/O处理单元
    * 五种I/O模型：阻塞I/O、非阻塞I/O、信号驱动I/O、异步I/O、I/O多路复用(select、poll、epoll)
    * 两种事件处理模式：Reactor、Proactor
  * 逻辑单元
    * 两种并发模式：半同步/半异步模式、领导者/追随者模式
    * 高效逻辑处理方式：有限状态机
  * 存储单元

> 琐碎知识：
>
> * 带外数据
>   * 发送/接收带外数据，用MSG_OOB标志的send/recv
>   * 检测带外数据的两种方式：SIGURG信号，IO复用的异常事件
>   * 带外数据的定位：`sockatmark`
> * 四种定时方法
>   * socket选项SO_RCVTIMEO和SO_SNDTIMEO
>   * SIGALRM信号
>   * I/O复用系统调用的超时参数
>   * 时间文件化timerfd