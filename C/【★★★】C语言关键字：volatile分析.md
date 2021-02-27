## volatile的含义
- volatile，不稳定的、易变的。用于声明一个变量可能被某些编译器未知的因素更改，例如：操作系统、硬件或者其他线程等等，==指示编译器对访问该变量的代码不再优化==，可以提供对特殊地址的稳定访问。

- I think one very important property of volatile is that it makes the variable be written to memory when modified ==（立即写）== and reread from memory each time it accessed ==（重新读）==. 


## 何谓编译器优化？
1. 示例：（重新读）

```c
int i = 5;
int a = i;
//……
int b = i;
```
编译器发现两次从i读取数据的代码之间，并没有对i进行过操作，则会自动把上次读的数据放在b中，而不是重新从i里面读

2. 示例：（立即写）
```c
int *ip = 设备地址
*ip = 1;
*ip = 2;
...
```
程序的本意是向某个地址（通常与硬件相关）发送两个指令，但编译器会认为第一个赋值是没有意义的，而将其优化掉；使用volatile后编译器就会保持程序的原意

## 适用于哪些情况？
1. 多线程应用中被几个任务共享的变量（有篇博文中包含了这一项，但是经过进一步深入研究，==不应该在多线程编程中使用volatile==，应用场景很有限，多数场景下是"blind luck"，见下文分析）
2. 一个中断服务子程序中会访问到的非自动变量
3. 并行设备的硬件寄存器（如状态寄存器）

举例：
1. 中断服务程序中修改供其他程序检测的变量

```c
volatile static sig_atomic_t i = 0;
int main(void){
    while(1){
        if(i)dosomething();
    }
}

void isr_handler(void){
    i = 1;
}
```
其中sig_atomic_t定义于<signal.c>中，实际为某种整型，C标准规定这种类型变量的存取是单指令的，不会被中断（both hw/sw）。

2. 存储器映射的硬件寄存器通常也要加volatile

```c
int  *output = (unsigned  int *)0xff800000;//定义一个IO端口；

int   init(void){
    int i;
    for(i=0;i< 10;i++)
        *output = i;
}
```

## 3个经典的问题
#### 一个参数可以即是const，又是volatile吗？
- 可以是。例如对于只读的状态寄存器而言，如果仅为volatile，它仍有可能被意想不到地修改，若加const，程序就不应该去修改它。

- 举个例子：在程序A中，我们要访问一个只读寄存器c，这时候修饰它为const，但是在程序B中，我们又会改变c的值，为了在A中避免编译优化造成程序逻辑错误，我们将其修饰为volatile，这样c就具有了双重属性。

- 这就带来了一个令人困惑的问题：既然被声明成const，为什么值会发生修改？这是一个复杂的问题：见笔记《C与C++ const类型变量的修改》

#### 一个指针可以是volatile吗？
可以是。例如当一个中断服务子程序企图修改一个指向buffer的指针时。

#### 下面的函数有什么错误？
```c
int square(volatile int *ptr){
    return *ptr * *ptr;
}
```

该函数的本意是返回ptr指向的int值的平方，但由于*ptr被volatile修饰，因此编译器将产生类似于下面这种代码:
```c
int square(volatile int *ptr){
    int a, b;
    a = *ptr;
    b = *ptr;
    return a * b;
}
```

而在此过程中，*ptr的值可能会发生意想不到的变化，a != b，违背了该函数的本意，因此应该修改为：
```c
int square(volatile int *ptr){
    int a;
    a = *ptr;
    return a * a;
}
```

## 不要在C/C++多线程中使用volatile！
==C++11标准明确指出解决多线程的数据竞争问题应该使用原子操作或者互斥锁。==
#### 既不充分也不必要
- 既不充分：volatile没有原子性和顺序性的语义，绝大多数volatile多线程同步都是错误的
- 也不必要：对共享数据区域的代码块加锁，已经足够保证数据访问的同步性，加不加volatile无所谓

#### 追求极限效率？
使用volatile无非是因为锁的开销太大，想使用轻量高效的同步方式，从极限效率考虑来实现很底层的接口。这要求编写者对程序逻辑走向很清楚才行，不然就会出错。而在此之前不妨问一问：你真的到达非volatile不可的性能瓶颈了吗？


## 参考文章：
1. https://blog.csdn.net/littletigerat/article/details/6439413
2. https://blog.csdn.net/Deep_l_zh/article/details/48652571
3. https://blog.csdn.net/qq_40860986/article/details/87544046
4. https://stackoverflow.com/questions/78172/using-c-pthreads-do-shared-variables-need-to-be-volatile
5. https://blog.csdn.net/Vay0721/article/details/79035854

