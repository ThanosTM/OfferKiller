## 多线程与pthread
- 每个线程所独有的：线程ID，一组寄存器值，栈，调度优先级及策略，信号屏蔽字，errno变量，线程私有数据
- 线程间共享的：可执行程序的代码，程序的全局内存和堆内存，栈，文件描述符；
- pthread，也成POSIX线程，来自标准POSIX.1-2001.

## 线程创建`pthread_create()`
```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg); 
```

函数的说明如下：
- 线程句柄`thread`：新的线程创建成功后，通过该参数将线程句柄返回给调用者，以便后续的管理；
- 入口函数`start_routine`；
- 入口参数`arg`：传入入口函数`start_routine`中，可以为创建的线程准备好一些专有数据；
- 线程属性`attr`：参数可选，可以传入NULL使用默认属性；
- 若线程创建成功，该函数返回0；

#### 一个简单的例子

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *thread1_func(void *arg){
    unsigned int count = *(unsigned int *)arg;
    while(count--){
        printf("thread1: %d\n", count);
        usleep(100000);
    }
}

void *thread2_func(void *arg){
    unsigned int count = *(unsigned int *)arg;
    while(count--){
        printf("thread2: %d\n", count);
        usleep(100000);
    }
}

int main(){
    pthread_t thread1, thread2;
    int ret;
    int arg = 10;

    ret = pthread_create(&thread1, NULL, thread1_func, &arg);
    if(ret != 0){
        printf("create thread1 error!\n");
    }

    ret = pthread_create(&thread2, NULL, thread2_func, &arg);
    if(ret != 0){
        printf("create thread2 error!\n");
    }

    printf("This is main process!\n");
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}
```

该程序编译时需要使用-lpthread动态链接库
```
gcc thread_create.c -o thread_create -lpthread
```

代码分析：
- `pthread_create()`函数创建了两个线程，并将10作为参数传入入口函数；
- `pthread_join()`会阻塞主进程的执行，直到合并的线程执行结束，第二个参数可以用于接收线程函数的返回值。
```c
int *thread_ret = NULL;
pthread_join(thread1, (void **)&thread_ret);
```

## 线程的合并与分离
线程也是系统的一种资源，有创建必然有回收，否则将会导致资源泄露。

#### 线程合并`pthread_join()`
```c
int pthread_join(pthread_t thread, void **retval);
```
- 线程合并是一种==主动回收线程资源==的方案，`pthread_join()`阻塞调用的进程或线程，直到合并的线程结束为止。
- 第一个参数`thread`为线程句柄，第二个参数若不为NULL，将接收线程函数的返回值，使用方式见上；
- 若该函数调用成功，将返回0；

#### 线程分离`pthread_detach()`
```c
int pthread_detach(pthread_t thread);
```
- 线程分离将线程资源的回收工作交给系统来完成，不会阻塞当前线程，由于采用了系统的自动回收机制，因此无法获得被分离线程的返回值。


## 线程的属性
线程可以由线程属性对象来描述，Linux线程的属性一般有以下这些：绑定属性、分离属性、调度属性、堆栈大小、满占警戒区大小等。该线程属性可以由以下两个接口分别进行初始化和销毁：
```c
int pthread_attr_init(pthread_attr_t *attr);  
int pthread_attr_destory(pthread_attr_t *attr); 
```

#### 绑定属性
- 也叫线程作用域属性：PTHREAD_SCOPE_SYSTEM系统级竞争资源，PTHREAD_SCOPE_PROCESS进程内竞争资源。
- 涉及到轻量级进程（LWP）的概念，轻量级进程是内核的调度实体，与普通进程相比，LWP与其他进程共享逻辑地址空间和系统资源，与线程相比，拥有自己的进程标识符，并且与其他进程有着父子关系。默认情况下，线程的绑定属性是非绑定的，即由哪些轻量级进程来控制哪些线程由操作系统来控制；当设置成绑定时，线程具有较高的响应速度，因为绑定线程可以保证在需要的时候总是有一个轻量级进程可以使用。设置绑定属性可以调用：
```c
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
pthread_create(&th, &attr, func, NULL);
```

但是对于Linux系统而言，==Linux的线程永远是绑定的==，或者说Linux的线程就等于轻量级进程；其实在业界有一种m:n的线程方案，但是Linux采用了1:1的方案，也就是说该属性在Linux系统上不起作用，这个接口纯属是为了兼容POSIX标准。

#### 分离属性
这与线程的合并与分离相关，默认的线程是PTHREAD_CREATE_JOINABLE可合并的，若设置了PTHREAD_CREATE_DETACHED分离的，就不需要再调用join或detach来回收线程资源了。
```c
pthread_attr_setdetachstat(&attr, PTHREAD_CREATE_DETACHED);
```

#### 调度属性
###### 调度算法
Linux提供多种调度算法：
- SCHED_OTHER：CFS调度
- SCHED_FIFO：实时调度策略，先到先服务，一直占用CPU直到更高优先级任务到达或自己放弃；
- SCHED_RR：实时调度策略，时间片轮转，时间片用完后重新分配时间片并放置与就绪队列尾部;
- SCHED_BATCH：非标准要求，用于批量执行；
- SCHED_IDLE：非标准要求，优先级比最大的nice还要低

可以使用以下接口指定线程调度算法：
```c
pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
```

###### 优先级
线程优先级是1~99，数值越大代表优先级越高，只有实时调度算法优先级才有效，对于SCHED_OTHER算法的优先级恒为0；同时实时调度算法的优先级也不是能够随便设置的，首先是要root用户设置，并且还需要放弃线程的继承权。其定义为：
```c
struct sched_param{
    int sched_priority;
};

int pthread_attr_setschedparam(pthread_attr_t *attr, struct sched_param *param);
```

###### 继承权
新线程在默认情况下是拥有继承权的，新线程要继承父线程的调度属性，其接口为：
```c
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
```

#### 堆栈大小属性
线程间拥有共享的逻辑地址空间，但是确拥有各自的执行栈，局部变量与函数调用都通过独自的线程栈来存储。Linux系统为每个线程默认分配了==8MB==的线程栈空间，若不够用可以通过以下接口修改大小：
```c
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
```
线程栈尽量以4KB，即页面大小的整数倍大小分配。

#### 满栈警戒区
堆栈用满是一件非常魏霞你的事情，可能会导致对内核空间的破坏，可能会被恶意利用从而造成严重的后果。因此可以为线程堆栈设置一个满栈警戒区，当有代码访问到这个区域时，则会抛出SIGSEGV信号。
```c
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);
```

## 线程本地存储TLS
- 在多线程模式下，同时出现多个执行单元，同一个函数会在多个执行单元中运行，但加锁实现数据保护又是不必要的，因为开销过大；而局部变量是不受多线程影响的，每个线程栈相互独立，多个线程同时执行函数访问到得到局部变量都是线程独有的，不会产生数据竞争。
- 在某些场景下，多线程希望自己看到的全局变量是自己的，既不想看到别人的，也不想别人对其修改，最终全局变量只是一个形式罢了，最终目的是每个线程拥有独立的一份，即==线程本地存储，Thread Local Storage，TLS==.
- Linux支持两种定义和使用TLS变量的方法：
1. ==__thread关键字==：这是最简单的方式，只需要用__thread对全局变量进行修饰即可实现像访问全局变量一样访问TLS；__thread关键字是gcc对C语言的扩展，而并非C语言标准所定义。
```c
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

__thread int var = 0;
// int var = 0;

void *worker(void *arg){
    int idx = *(int *)arg;
    int i;

    for(i = 0; i < 10; i++){
        printf("thread: %d ++var = %d\n", idx, ++var);
        usleep(200*1000);
    }
}

int main(){
    pthread_t pid1, pid2;

    int arg1 = 1, arg2 = 2;

    pthread_create(&pid1, NULL, worker, &arg1);
    pthread_create(&pid2, NULL, worker, &arg2);

    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);

    return 0;
}
```

2. ==`pthread_key_create`函数==：来自库函数的支持，需要先创建一个线程本地存储区，再通过API读取或设置线程变量的值。
```c
nt pthread_key_create(pthread_key_t *key, void (*destructor)(void*));  
int pthread_key_delete(pthread_key_t key);  
void* pthread_getspecific(pthread_key_t key);  
int pthread_setspecific(pthread_key_t key, const void *value);
```

一个简单的例子：
```c
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

pthread_key_t g_key;
int var = 0;

void *worker(void *arg){
    int idx = *(int *)arg;
    int i;

    for(i = 0; i < 10; i++){
        int tmp = *(int *)pthread_getspecific(g_key) + 1;
        pthread_setspecific(g_key, &tmp);
        printf("thread: %d ++var = %d\n", idx, tmp);
        usleep(200*1000);
    }
}

int main(){
    pthread_t pid1, pid2;

    int arg1 = 1, arg2 = 2;

    pthread_key_create(&g_key, NULL);
    pthread_create(&pid1, NULL, worker, &arg1);
    pthread_create(&pid2, NULL, worker, &arg2);

    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    pthread_key_delete(g_key);
    return 0;
}
```


## 线程的同步

#### 互斥量
互斥量mutex本质上是一把锁，在访问共享资源前对互斥量进行加锁，访问完成后进行解锁；对互斥量进行加锁后，其他任何试图再次对互斥量加锁的线程都将阻塞，当互斥量被解锁后，所有阻塞在该互斥量上的线程都会变成可运行状态，竞争这把锁，其他线程继续阻塞。
###### 互斥量的静态创建
```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```

###### 互斥量的动态创建
由`pthread_mutex_init`动态创建的mutex需要使用`pthread_mutex_destroy`释放。
```c
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *restrict attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

###### 互斥量的操作
- `int pthread_mutex_lock(pthread_mutex_t *mutex);`
- `int pthread_mutex_unlock(pthread_mutex_t *mutex);`
- `int pthread_mutex_trylock(pthread_mutex_t *mutex);`：该函数尝试对互斥量进行加锁，若本身处于未锁定状态则将其锁定并返回0，若已上锁则返回EBUSY，而不会阻塞线程。
- `int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex, const struct timespec *restrict tsptr);`：绑定线程阻塞时间，达到超时时间值得到时候还未获得锁的话，则直接返回ETIMEDOUT；

###### 避免死锁
发生死锁的情形：
- 试图对同一个互斥量加锁两次；
- 多个线程循环等待互斥量；

避免死锁：==仔细控制互斥量的加锁顺序==，例如假设需要对两个互斥量A和B同时加锁，则在所有线程中保证先加锁A再加锁B，则这两个互斥量就不会产生死锁。

#### 读写锁
读写锁具有更高的并行性，特别是对于对数据结构的读次数远大于写的情况；读写锁具有3种状态：读模式加锁、写模式加锁、不加锁，一次只有一个线程可以占有写模式的读写锁，但是多个线程可以同时占有读模式的读写锁。

###### 细节
- 写加锁状态下，所有对该读写锁进行加锁操作的线程都将阻塞，知道写线程解锁；
- 读加锁状态下，所有试图以读模式加锁的线程都可以获得访问权，但是任何以写模式加锁的线程都会阻塞，知道所有线程释放读锁为止。此外：==当一个线程试图以写模式获取读加锁状态下的读写锁时，操作系统会阻塞随后的读模式锁请求，避免读锁长期占用而导致写线程饥饿。==

###### 初始化与销毁
```c
int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *restrict attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
```

###### 操作
```c
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
```

###### 应用
作业请求队列由一个读写锁保护，多个工作线程获取单个主线程分配给他们的作业：当从队列中添加或取出作业时，采用写模式加锁；当从队列中搜索作业时（例如通过线程id匹配），使用读模式加锁，可以实现所有工作线程并发地搜索队列。


#### 条件变量
条件变量给多线程提供了一个会合的场所，需要与互斥量一起使用（本身是由互斥量所保护的），允许线程以无竞争的方式等待指定的条件发生。

###### 初始化
```c
int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
int pthread_cond_destroy(pthread_cond_t *cond);
```

###### 等待条件变量
```c
int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
int pthread_cond_timedwait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, const struct timespec *restrict tsptr);
```

执行流程：调用者需要将锁住的互斥量传给函数，函数将调用线程放置在等待条件的线程列表上，对互斥量解锁。这时线程就不会错过条件的任何变化，但函数返回时，互斥量将再次被锁住。

###### 通知条件变量
```c
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
```
可以理解为在给线程或者条件发送信号，这里注意一定要改变了条件状态之后再给线程发送信号。

###### 应用：==生产者消费者模型==
```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
const int MAX_COUNTER = 20;
const int DELAY = 200 * 1000;
const int PRODUCER = 3;
const int CONSUMER = 4;
int counter = 0;

void *producer(void *arg){
    int idx = (int)arg;
    while(1){
        pthread_mutex_lock(&mutex);
        while(counter == MAX_COUNTER){
            pthread_cond_wait(&not_full, &mutex);
            printf("producer%d wake!\n", idx);
        }
        counter++;
        printf("producer%d: counter = %d\n", idx, counter);
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&not_empty);
        usleep(DELAY);
    }
}

void *consumer(void *arg){
    int idx = (int)arg;
    while(1){
        pthread_mutex_lock(&mutex);
        while(counter == 0){
            pthread_cond_wait(&not_empty, &mutex);
            printf("consumer%d wake!\n", idx);
        }
        counter--;
        printf("consumer%d: counter = %d\n", idx, counter);
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&not_full);
        usleep(2 * DELAY);
    }
}


int main(){
    pthread_t prod[PRODUCER];
    pthread_t cons[CONSUMER];

    for(int i = 0; i < PRODUCER; i++){
        pthread_create(&prod[i], NULL, producer, (void *)i);
    }

    for(int i = 0; i < CONSUMER; i++){
        pthread_create(&cons[i], NULL, consumer, (void *)i);
    }

    for(int i = 0; i < PRODUCER; i++){
        pthread_join(prod[i], NULL);
    }

    for(int i = 0; i < CONSUMER; i++){
        pthread_join(cons[i], NULL);
    }
    return 0;
}
```

这里要注意==惊群效应==，当使用`pthread_cond_broadcast`时将会唤醒所有等待改条件的线程，此时需要使用while(){}进行判断是否真的可以轮到它来执行了。


#### 自旋锁
- 自旋锁与互斥锁的区别在于，不是通过休眠使线程阻塞，而是==在获取锁之前一直处于忙等的状态==，可适用于以下情况：锁被持有的时间短，线程不希望在重新调度上花费太多的成本。
- 自旋锁用于非抢占式内核中是十分有用的：除了提供互斥机制，还会阻塞中断，这样中断处理程序就不会使系统陷入死锁状态，因为它需要获取已被加锁的自旋锁。在这种类型的内核中，中断处理程序不能休眠，因此能用的同步原语只能是自旋锁。
- 但在用户层，自旋锁并不是非常有用，很多互斥量的实现非常高效（甚至会先自旋一小段时间，超过阈值才会休眠）

###### 初始化
与mutex几乎一致

###### 操作
与mutex几乎一致

**需要注意的是**：不要调用在持有自旋锁的情况下可能会进入休眠状态的函数，这样会浪费CPU资源，会使得其他线程获取自旋锁需要等待的时间更长。


#### 屏障barrier
是用户协调多个线程并行工作的同步机制，屏障允许每个线程等待直到所有的合作线程都达到了某一点，然后继续开始执行。`pthread_join()`就是一种屏障。但是屏障对象的概念更广，允许任意数量的线程等待。

###### 初始化与销毁
```c
int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned int count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
```
其中，count指定了在允许所有线程继续运行之前，必须达到屏障的线程数目。

###### 等待
表面本线程已经完成了工作，等待其他线程追赶上来。
```c
int pthread_barrier_wait(pthread_barrier_t *barrier);
```
