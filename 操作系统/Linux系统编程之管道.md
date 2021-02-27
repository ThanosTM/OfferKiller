## 学习背景
在进行呼吸机项目调试时，在PC端需要模拟下位机串口收发，因此使用两个个FIFO管道，由呼吸机上位机程序对其进行读写，并且另开两个进程进行模拟收发调试；在完成这次调试后，想要系统补充学习一下Linux的管道机制的知识。

## 文章出处
参考：
- [匿名管道](https://www.cnblogs.com/mickole/p/3192210.html)
- [管道读写规则](https://www.cnblogs.com/mickole/archive/2004/01/13/3192461.html)
- [命名管道（FIFO）](https://www.cnblogs.com/mickole/archive/2004/01/13/3192909.html)

## 匿名管道
#### 管道简介
管道是Linux支持的最初的Unix IPC形式之一

 - 半双工，互相的通信需要两个管道
 - 只能用于父子进程或者兄弟进程
 - 写入与读出：写入内容添加在管道缓冲区的末尾，读出从头部读出数据
 
 #### 管道创建
 - 需要包含头文件<unistd.h>
 - 原型：`int pipe(int fd[2])`
 - `fd[0]`读管道，`fd[1]`写管道
 - 成功返回0，错误返回错误代码
 - 创建的管道两端处于同一进程，没有什么实际作用；因此一般用法是，再fork一个子进程实现父子进程通信，示意图：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200916125942171.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM0ODE2NjQ3,size_16,color_FFFFFF,t_70#pic_center)
#### 一个例子
```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h> 

int main(void)
{
    int fds[2];
    if(pipe(fds) == -1){
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    if(!fork()){
		if(pid == 0){
        close(fds[0]);//子进程关闭读端
        write(fds[1],"hello",6);
        exit(EXIT_SUCCESS);
    }
    
    close(fds[1]);//父进程关闭写端
    char buf[10] = {0};
    read(fds[0],buf,10);
    printf("receive datas = %s\n",buf);
    return 0;
}
```

## 管道读写细节

 - 当管道没有数据可以读时：
	 - 阻塞情况下，read()调用阻塞，直到有数据到来为止
	 - 非阻塞情况下，read()调用返回-1，errno值为EAGAIN

 - 当管道满时：
 	- 阻塞情况下，write()调用阻塞，直到有进程读出数据
 	- 非阻塞情况下，write()调用返回-1，errno值为EAGAIN

 - 所有通道写端文件描述符被关闭，则read()返回0，并不报错
 - 所有通道读端文件描述符别关闭，则write()产生信号SIGPIPE
 - 当写入数据量不大于PIPE_BUF时，linux将保证写入的原子性
 - 当写入数据量大于PIPE_BUF时，将不再保证写入的原子性
 

>  In Linux versions before 2.6.11, the capacity of a pipe was the same as the system page size (e.g., 4096 bytes on i386).  Since Linux 2.6.11, the pipe capacity is 65536 bytes.

对于原子性的理解：一个进程中，要么全部写入，要么一个都不写入，而当不具有原子性时，写入时可能出现其他进程穿插写入。

## 命名管道FIFO
#### 匿名管道的一些局限性

 - 只支持单向数据流
 - 只用于亲缘关系进程之间
 - 没有名字
 - 缓冲区有限，因为只存在在内存中，创建时分配一个页面的大小
 - 无格式字节流

#### 命名管道FIFO简介

 - FIFO不同于匿名管道，它提供一个路径名与之关联，并以FIFO的文件形式存在于文件系统中，即使进程间不存在亲缘关系，也能通过FIFO互相通信。注意，FIFO严格遵循先进先出，不支持例如lseek()等文件定位操作。
 - 命名通道的创建
 	- 命令行创建：$mkfifo  filename
 	- 程序中创建：

```c
#include <sys/types.h>
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode)
```
当该路径名以及存在时，会返回EEXIST，典型的调用代码中，会首先检查是否返回该错误，若是则直接调用open()函数即可。

 - 命名管道比匿名管道多了一个open()操作，FIFO的打开规则为：
	- 若打开操作为阻塞（默认为阻塞），为读而打开FIFO将阻塞到有进程为写而打开FIFO，为写而打开亦然；
	- 若打开操作设置了非阻塞位，则在另一端没有打开的情况下，为读而打开FIFO将成功返回，为写而打开将返回ENXIO错误

## 自己写的示例代码
写进程代码：

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME   "MY_FIFO"
int main(){
    int fd;
    if((fd = open(FIFO_NAME, O_WRONLY)) < 0){
        perror("open fifo error!");
        return -1;
    }
    printf("Open fifo successfully.\n");
    while(1){
        char buf[100] = {0};
        for(int i = 0; i < 99; i++){
            buf[i] = 'A' + i%52;
        }
        write(fd, buf, 100);
        sleep(1);
    }
    return 0;
}
```

两个进程读FIFO代码：

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME   "MY_FIFO"
int main(){
    int fd;
    if(!fork()){
        if((fd = open(FIFO_NAME, O_RDONLY)) < 0){
            perror("open fifo error!");
            return -1;
        }
        printf("Open fifo successfully.\n");
        while(1){
            char buf[10];
            int ret = read(fd, buf, 10);
            if(ret <= 0)exit(EXIT_FAILURE);
            printf("child read %d byte from fifo: %s\n", ret, buf);
        }
        exit(EXIT_SUCCESS);
    }
    if((fd = open(FIFO_NAME, O_RDONLY)) < 0){
        perror("open fifo error!");
        return -1;
    }
    printf("Open fifo successfully.\n");
    while(1){
        char buf[10];
        int ret = read(fd, buf, 10);
        if(ret <= 0)exit(EXIT_FAILURE);
        printf("father read %d byte from fifo: %s\n", ret, buf);
    }
    return 0;
}

```
