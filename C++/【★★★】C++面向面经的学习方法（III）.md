## 11、C++强制类型转换
- C++引入新的强制类型转换机制，克服了C语言强制类型转换的缺点
- 四种不同功能的强制类型转换

#### static_cast<>
- 用于进行比较自然和低风险的转换
- 类层次结构中，基类和派生类之间指针的转换：上行转换是安全的，下行转换是不安全的
- 基本数据类型转换，安全性由开发者保证

#### reinterpret_cast<>（C++的设计思想：你可以做你想做的任何事，但身为成年人要对自己的行为负责）
- 逐比特复制的转换操作，不同类型指针、不同类型引用、指针和整数之间等等，是一个高危操作，可移植性也差

```cpp
#include <iostream>
using namespace std;

class A
{
public:
    int i;
    int j;
    A(int n):i(n),j(n) { }
};

int main()
{
    A a(100);
    int &r = reinterpret_cast<int&>(a); // 强行让 r 引用 a

    r = 200;                            // 把 a.i 变成了 200
    cout << a.i << "," << a.j << endl;  // 输出 200,100

    int n = 300;
    A *pa = reinterpret_cast<A*> (&n);  // 强行让 pa 指向 n
    pa->i = 400;                        // n 变成 400
    pa->j = 500;                        // 此条语句不安全，很可能导致程序崩溃
    cout << n << endl;                  // 输出 400

    long long la = 0x12345678abcdLL;
    pa = reinterpret_cast<A*>(la);     // la太长，只取低32位0x5678abcd拷贝给pa
    unsigned int u = reinterpret_cast<unsigned int>(pa);   // pa逐个比特拷贝到u
    cout << hex << u << endl;          // 输出 5678abcd

    typedef void (* PF1) (int);
    typedef int (* PF2) (int,char *);
    PF1 pf1;  
    PF2 pf2;
    pf2 = reinterpret_cast<PF2>(pf1);  // 两个不同类型的函数指针之间可以互相转换
}
```

#### const_cast<>
- const_cast仅用于去除const属性的转换，可以将常量指针转为非常量指针，常量引用转为非常量引用，但去常量属性是一个危险的动作，要慎用

#### dynamic_cast<>
- dynamic_cast通过运行时类型检查来保证安全性，对于不安全的类型转换会返回NULL指针，使用场景是继承链之间的类型转换
- dynamic_cast也可用作交叉转换，

## 12、重写、重载、覆盖
#### 函数重载overload
- 定义：==同一作用域内==，==同名函数==的形式==参数（个数、类型、顺序）不同==时，构成函数重载。
- 返回值类型与重载无关
- 类的静态成员函数和普通成员函数可以形成重载
- 同一作用域，例如全局函数之间、同一类成员函数之间

举例：
```cpp
int max(int a,int b){
    return a>b?a:b;
};

double max(double a,double b){
    return a>b?a:b;
}
```

#### 函数隐藏Hiding
- 定义：==不同作用域中==定义的同名函数构成函数隐藏（==不要求函数返回值和参数类型相同==），例如派生类成员函数屏蔽同名基类成员函数，类成员函数屏蔽全局函数（派生类重写基类虚函数的情形除外）

#### 函数重写（==覆盖）Override
###### 定义
派生类和基类==同返回值类型==、==同名==、==同参数==的==虚函数==重定义，构成虚函数重写

###### 一种特殊情形：返回类型协变Covariant
基类虚函数返回类型为基类的指针或者引用，新的返回类型为派生类的指针或者引用，进行覆盖的方法叫返回类型协变。一般而言，覆盖要求返回值类型严格相等，因此返回类型协变是一种特殊情况。

## 13、虚拟继承
#### 为什么要引入虚拟继承？
虚拟继承是应对多重继承而出现的，当B1和B2同时继承A时，D类中会出现两份A的变量和函数，为了节省内存空间和菱形继承的二义性问题，可以将B1和B2声明为虚拟继承。
```
class A;
class B1:public virtual A;
class B2:public virtual A;
class D:public B1,public B2;
```

#### 空间大小计算
虚拟继承的子类要多出一个指向基类对象的指针

```cpp
#include <stdio.h>  
class A {  
public:  
    int a;  
};//sizeof(A) = 4  
 
class B : virtual public A {  
};// sizeof(B) =4+4=8  
 
class C : virtual public A {          
};//sizeof(C) =4+4=8  
 
class D : public B, public C{         
};  
//sizeof(D)=8+8-4=12这里需要注意要减去4，因为B和C同时继承A，只需要保存一个A的副本就好了，sizeof(D)=4(A的副本)+4(B的虚表)+4(C的虚表)=12，也可以是8（B的副本）+8（c的副本）-4（A的副本）=12
```


## 14、成员变量的初始化顺序
#### 初始化顺序：
1. 基类的静态或全局变量
2. 派生类的静态或全局变量
3. 基类的成员变量
4. 派生类的成员变量

#### 其他
1. 初始化列表方式，只有定义变量的顺序有关
2. 构造函数中初始化，与构造函数中的顺序有关
3. const必须在初始化列表中初始化
4. static必须在类外初始化

## 15、虚析构函数
当基类指针指向派生类对象时，若delete基类指针，当没有定义virtual时，只会调用基类的析构函数，而造成派生类对象析构不完全；而定义虚析构函数后，将先析构派生类对象，再析构基类对象。

```cpp
#include <iostream>
using namespace std;

class A{
public:
    A(){ cout << "construct A" << endl; }
    virtual ~A(){ cout << "destruct A" << endl; }
};

class B : public A{
public:
    B(){ cout << "construct B" << endl; }
    ~B(){ cout << "destruct B" << endl; } 
};

int main(){
    A *pa = new B();
    delete pa;
    return 0;
}
```

上述程序的运行结果可以说明父类与子类的构造函数和析构函数的调用过程。
```cpp
construct A
construct B
destruct B
construct B
```