## 12、select、poll、epoll
#### I/O多路复用
- 内核一旦发现进程指定的一个或者多个I/O条件准备读取，它就通知该进程。
- 适用：
1. 客户处理多个描述字时（交互式输入和网络套接字）
2. TCP服务器既要处理监听套接字，又要处理已连接套接字口
3. 服务器既要处理TCP又要处理UDP
4. 处理多个服务或者多个协议

#### select函数
###### 描述
该函数准许进程指示内核等待多个事件中的任何一个发送，并只在有一个或者多个事件发生或经历一段时间后唤醒。

###### 参数设置
```cpp
#include <sys/select.h>
#include <sys/time.h>

int select(int maxfdp1,fd_set *readset,fd_set *writeset,fd_set *exceptset,const struct timeval *timeout)
返回值：就绪描述符的数目，超时返回0，出错返回-1
```

- maxfdp1：待测试的描述字个数，实际将测试0~maxfdp1-1,
- readset/writeset/exceptset：指示内核测试读，写，异常条件的描述字（不感兴趣则置空），对文件集合的操作如下：
```cpp
void FD_ZERO(fd_set *fdset);           //清空集合

void FD_SET(int fd, fd_set *fdset);   //将一个给定的文件描述符加入集合之中

void FD_CLR(int fd, fd_set *fdset);   //将一个给定的文件描述符从集合中删除

int FD_ISSET(int fd, fd_set *fdset);   // 检查集合中指定的文件描述符是否可以读写 
```

- timeout：指示等待描述字中任何一个就绪可花多少时间，3中情况
1. 永远等待：NULL，仅有一个描述字准备好I/O才返回
2. 等待一段固定时间
3. 根本不等待：设置为0，检查描述字后立即返回——轮询

###### 原理
![image](https://images0.cnblogs.com/blog/305504/201308/17201205-8ac47f1f1fcd4773bd4edd947c0bb1f4.png)

1. 使用copy_from_user从用户空间拷贝fd_set到内核空间
2. 注册回调函数__pollwait
3. 遍历所有fd，调用其对应的poll方法（对于socket，这个poll方法是sock_poll，sock_poll根据情况会调用到tcp_poll,udp_poll或者datagram_poll）
4. 以tcp_poll为例，其核心实现就是__pollwait，也就是上面注册的回调函数。
5. __pollwait的主要工作就是把current（当前进程）挂到设备的等待队列中，不同的设备有不同的等待队列，对于tcp_poll来说，其等待队列是sk->sk_sleep（注意把进程挂到等待队列中并不代表进程已经睡眠了）。在设备收到一条消息（网络设备）或填写完文件数据（磁盘设备）后，会唤醒设备等待队列上睡眠的进程，这时current便被唤醒了。
6. poll方法返回时会返回一个描述读写操作是否就绪的mask掩码，根据这个mask掩码给fd_set赋值。
7. 如果遍历完所有的fd，还没有返回一个可读写的mask掩码，则会调用schedule_timeout是调用select的进程（也就是current）进入睡眠。当设备驱动发生自身资源可读写后，会唤醒其等待队列上睡眠的进程。如果超过一定的超时时间（schedule_timeout指定），还是没人唤醒，则调用select的进程会重新被唤醒获得CPU，进而重新遍历fd，判断有没有就绪的fd。
8. 把fd_set从内核空间拷贝到用户空间。


###### 缺点
- fd在用户态和内核态直接的来回拷贝开销很大
- 每次调用要将进程的引用加入所有被监视的队列（一次遍历），进程被唤醒并不知道哪个文件收到数据（两次遍历）
- fd有上限1024（32位机）

#### poll函数
###### 描述
基本与select函数的机制是一样的，也是通过轮询管理多个描述符，不论文件描述符是否就绪都需要进行用户态和内核态之间的大量拷贝，==但poll没有最大文件描述符数量的限制==，原因是链式存储。

###### 参数设置

```cpp
#include <poll.h>
int poll ( struct pollfd * fds, unsigned int nfds, int timeout);
```

- pollfd结构体指针fds：指定多个被监视的文件描述符，结构体中的events是事件掩码，用户设置等待的事件，内核返回时将请求的发生的事件在revents中返回。

#### ==epoll函数==
###### 描述
epoll是select和poll的增强版，更灵活，无数量限制，使用一个文件描述符管理多个文件描述符，将用户关心的文件描述符事件放在内核的事件表中，因此用户态和内核态之间的拷贝开销很小。

###### 参数设置

```cpp
#include <sys/epoll.h>
int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);
```

- ==epoll_create==：创建一个epoll的句柄，size用来告诉内核这个监听的数目一共有多大。这个参数不同于select()中的第一个参数，给出最大监听的fd+1的值。需要注意的是，当创建好epoll句柄后，它就是会占用一个fd值，在linux下如果查看/proc/进程id/fd/，是能够看到这个fd的，所以在使用完epoll后，必须调用close()关闭，否则可能导致fd被耗尽。返回值是epfd;
- ==epoll_ctl==：epoll的事件注册函数， 不同于select()，epoll先向内核注册要监听的事件类型。
1. epfd：epoll文件描述符
2. op：3种动作：
```
EPOLL_CTL_ADD：注册新的fd到epfd中；
EPOLL_CTL_MOD：修改已经注册的fd的监听事件；
EPOLL_CTL_DEL：从epfd中删除一个fd；
```
3. fd：需要监听的fd
4. epoll_event结构体：告诉内核要监听哪些事件

```
struct epoll_event {
    __uint32_t events;  /* Epoll events */
    epoll_data_t data;  /* User data variable */
};

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
};
```

- ==epoll_wait==：等待事件的产生，相当于select()函数，events参数用来接收内核传递的事件集合

###### 工作模式
- LT(Level Trigger，默认)：epoll_wait检测的描述符事件发生时，可以不必立即处理，下次调用时会再次响应应用程序并通知此事件
- ET(Edge Trigger)：必须立即处理，如果不处理，下次调用时就不会再次通知了
- ET减少了事件重复触发的此处，但必须要使用非阻塞套接口，以免由于一个文件句柄的阻塞操作把处理多个文件描述符的任务饿死。

###### 网上复制下来的代码，值得一读
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>

#define IPADDRESS   "127.0.0.1"
#define PORT        8787
#define MAXSIZE     1024
#define LISTENQ     5
#define FDSIZE      1000
#define EPOLLEVENTS 100

//函数声明
//创建套接字并进行绑定
static int socket_bind(const char* ip,int port);
//IO多路复用epoll
static void do_epoll(int listenfd);
//事件处理函数
static void
handle_events(int epollfd,struct epoll_event *events,int num,int listenfd,char *buf);
//处理接收到的连接
static void handle_accpet(int epollfd,int listenfd);
//读处理
static void do_read(int epollfd,int fd,char *buf);
//写处理
static void do_write(int epollfd,int fd,char *buf);
//添加事件
static void add_event(int epollfd,int fd,int state);
//修改事件
static void modify_event(int epollfd,int fd,int state);
//删除事件
static void delete_event(int epollfd,int fd,int state);

int main(int argc,char *argv[])
{
    int  listenfd;
    listenfd = socket_bind(IPADDRESS,PORT);
    listen(listenfd,LISTENQ);
    do_epoll(listenfd);
    return 0;
}

static int socket_bind(const char* ip,int port)
{
    int  listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
        perror("socket error:");
        exit(1);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1)
    {
        perror("bind error: ");
        exit(1);
    }
    return listenfd;
}

static void do_epoll(int listenfd)
{
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int ret;
    char buf[MAXSIZE];
    memset(buf,0,MAXSIZE);
    //创建一个描述符
    epollfd = epoll_create(FDSIZE);
    //添加监听描述符事件
    add_event(epollfd,listenfd,EPOLLIN);
    for ( ; ; )
    {
        //获取已经准备好的描述符事件
        ret = epoll_wait(epollfd,events,EPOLLEVENTS,-1);
        handle_events(epollfd,events,ret,listenfd,buf);
    }
    close(epollfd);
}

static void
handle_events(int epollfd,struct epoll_event *events,int num,int listenfd,char *buf)
{
    int i;
    int fd;
    //进行选好遍历
    for (i = 0;i < num;i++)
    {
        fd = events[i].data.fd;
        //根据描述符的类型和事件类型进行处理
        if ((fd == listenfd) &&(events[i].events & EPOLLIN))
            handle_accpet(epollfd,listenfd);
        else if (events[i].events & EPOLLIN)
            do_read(epollfd,fd,buf);
        else if (events[i].events & EPOLLOUT)
            do_write(epollfd,fd,buf);
    }
}
static void handle_accpet(int epollfd,int listenfd)
{
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t  cliaddrlen;
    clifd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen);
    if (clifd == -1)
        perror("accpet error:");
    else
    {
        printf("accept a new client: %s:%d\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
        //添加一个客户描述符和事件
        add_event(epollfd,clifd,EPOLLIN);
    }
}

static void do_read(int epollfd,int fd,char *buf)
{
    int nread;
    nread = read(fd,buf,MAXSIZE);
    if (nread == -1)
    {
        perror("read error:");
        close(fd);
        delete_event(epollfd,fd,EPOLLIN);
    }
    else if (nread == 0)
    {
        fprintf(stderr,"client close.\n");
        close(fd);
        delete_event(epollfd,fd,EPOLLIN);
    }
    else
    {
        printf("read message is : %s",buf);
        //修改描述符对应的事件，由读改为写
        modify_event(epollfd,fd,EPOLLOUT);
    }
}

static void do_write(int epollfd,int fd,char *buf)
{
    int nwrite;
    nwrite = write(fd,buf,strlen(buf));
    if (nwrite == -1)
    {
        perror("write error:");
        close(fd);
        delete_event(epollfd,fd,EPOLLOUT);
    }
    else
        modify_event(epollfd,fd,EPOLLIN);
    memset(buf,0,MAXSIZE);
}

static void add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

static void delete_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

static void modify_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}
```

###### 如何区分不同事件？
epoll_wait()返回事件数目和events结构体数组，可以遍历每一个events结构体，里面有事件的mask，通过与宏定义相与，可获得事件类型。

###### 如何区分不同文件？
同样是events结构体中还有epoll_data_t项，其中的fd则是发生事件对应的fd.

#### ==epoll底层原理==
![image](https://images2015.cnblogs.com/blog/899685/201701/899685-20170102144714222-318866749.png)
###### epoll_create
epoll被内核初始化后，会开辟出epoll自己的内核高速cache区用于安置每一个想要监视的socket，每个socket节点将以==红黑树==的方式存储（快速查找、删除、插入）。==内核高速缓存==：建立连续的物理内存页，在上建立slab层，每次使用的都是空闲的已经分配好的对象；此外又建立了一个==就绪list==链表，用于存放已经就绪的节点

###### epoll_ctl
将socket放到epoll文件系统的红黑树之上，给内核中断处理程序注册一个==回调函数==（如果这个句柄的中断到了，就放到就绪列表里），于是：epoll的基础就是回调

###### epoll_wait
直接返回就绪列表中的元素即可

###### LT和ET在底层上的区别
对于ET，调用epoll_wait后清空就绪链表；而对于LT模式，对于存在未处理的事件的socket又放回到就绪链表里了，于是下次调用epoll_wait又将直接返回该事件的fd

###### 高效的本质在于
1. 减少了用户态和内核态之间的文件描述符的拷贝
2. 减少了对可读可写文件描述符的遍历
3. 用户和内核mmap同一块内存，避免了无谓的内存拷贝
4. I/O性能不会随着监听文件数量增加而增加
5. 红黑树存储fd，插入，查找，删除性能不错

###### 并发量低的情况
若并发量低，又大多比较活跃，则select反正都要遍历所有，实现有简单，将比epoll更高效


## 13、GET和POST的区别
#### 一般的八股说法
- GET语义是请求数据，而POST是发送数据
- 传参：GET将通过URL传递参数，'?'分割，'&'相连；而POST是放在请求数据体中。
- 长度限制：GET传递的参数长度有限制，一般为2k之内；而POST是没有限制的
- 安全性：GET比POST更不安全，因为GET将参数暴露在URL中。
- 浏览器后退或刷新：GET无害；POST会重新提交（一般浏览器会弹窗告知）
- 浏览器书签：GET含有参数的URL地址是可以书签收藏的
- 缓存：GET的URL可以被缓存
- 浏览器历史：参数保留在浏览器历史中
- 数据类型限制：GET只允许ASCII字符，而POST可以添加二进制数据

#### 实际上
- GET和POST是不存在任何区别的，HTTP只是行为准则，TCP才是实现的根本，协议是人定的并且约定俗成的，这关乎到服务器端的设计。
- 关于URL长度：大多数浏览器会限制的2K之内，而大多数服务器是支持64K的，超过的部分不予处理
- 关于数据体：GET也可以在数据体中塞数据，但还是取决于服务器的设计，有的服务器会选择接收，而其他则可能直接忽略。
- 关于安全性：实际上两者都不安全，从传输角度看因为HTTP是明文传输，安全性依赖其加密方式。
- 对于大多数浏览器，GET只发一个数据报，而POST分两次发（header+data），可以有限地优化网络性能（然而这不是HTTP协议的强制规定，取决于浏览器设计框架，FireFox就只发一个）。

