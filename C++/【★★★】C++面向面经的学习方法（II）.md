## 6、const关键字分析
#### 兼容C语言的部分
###### 1. const修饰局部变量
位于全局变量区，不可以被直接修改，编译器提供限制，但能通过指针间接修改

###### 2. const修饰全局变量
- 位于只读数据段，不可以被直接修改，编译器提供限制，也不能被指针间接修改，操作系统提供限制（触发段错误）
- const全局变量的作用域是该文件内（而普通全局变量默认是extern型的），若要多文件共享该const变量，需要每一个都声明为extern const型

###### 3. 指针常量与常量指针
const修饰变量的一种，详见《C和指针》学习笔记

###### 4. 修饰函数的参数
防止参数在函数体内被修改

###### 5. 修饰函数的返回值

```c
const char * GetString(void);
const char *str = GetString(); //√
```

编译器提供检查，但只会发出警告，不会报错（C++中将提供更严格的限制）

#### C++中的扩展
###### const修饰类对象
只能访问const成员变量和const成员函数

```cpp
class A{
private:
    const int a;
public:
    A():a(1){}
    int getInt() const{ return a; }
}

const A a;
a.getInt(); //√
```

###### const成员变量
初始化const成员变量只能通过构造函数的初始化列表实现

###### const成员函数
不能修改该类中的成员变量，一般不改变数据成员的成员函数最好都要在后面加 const

###### const修饰引用类型
自己写的示例程序：
```cpp
#include <iostream>    
using namespace std;    

class A{
private:
    int i;
public:
    A():i(0){ cout << "construct A" << endl; }
    A(const A &){ cout << "copy construct A" << endl; }
    ~A(){ cout << "destruct A" << endl; }
    void setInt(int x){ i = x; }
    int getInt(){ return i; }
};

class B{
    int i;
public:
    B(){ i = 0; }
    //传入参数A调用复制构造函数，析构函数
    //返回值调用复制构造函数，析构函数
    A getA(A a){
        return a;
    }

    int getInt(){
        return i;
    }
};

class C{
private:
    int i;
public:
    C(){ i = 0; }
    //传入参数A不调用复制构造函数，析构函数
    //返回值不调用复制构造函数，析构函数
    A & getA(A &a){
        return a;
    }

    int & getInt(){
        return i;
    }
};
    
int main(){
    A a;    //construct A * 1
    B b;
    C c;
    b.getA(a);  //copy construct A * 2, destruct A * 2
    c.getA(a);  //copy construct A * 0, destruct A * 0

    //b.getInt() = 2;  //不合法，不是可修改的左值
    c.getInt() = 2; //合法，且C中的i真的被修改了，因此实际使用时最好加const确保不被修改，编译器提供检查
    cout << c.getInt() << endl;
    return 0;
    //destruct A * 1
}
```

分析：
- 引用传递参数和返回值可以防止生成临时变量，调用复制构造函数和析构函数（自定义数据类型而言，内置数据类型例如int则没有区别）
- const修饰函数参数的引用，指示函数体内无法修改该参数值（否则传入的参数将会发生修改）
- const修饰返回值的引用，指示返回值无法作为左值进行修改（否则打破数据封装性原则）

## 7、static关键字分析
#### 兼容C的部分
修饰局部变量，修饰全局变量，修饰函数（详见《C和指针》学习笔记）

#### static静态成员变量
- 类内声明，类外定义和初始化
- 相当于类域中的全局变量，被所有对象和派生类共享
- 可以作为成员函数的可选参数
```cpp
class base{ 
public : 
    static int _staticVar; 
    int _var; 
    void foo1(int i=_staticVar);//正确,_staticVar为静态数据成员 
    void foo2(int i=_var);//错误,_var为普通数据成员 
};
```
- 可以是所属类的类型
```cpp
class base{ 
public : 
    static base _object1;//正确，静态数据成员 
    base _object2;//错误 
    base *pObject;//正确，指针 
    base &mObject;//正确，引用 
};
```

#### static静态成员函数
- 不能调用非静态成员变量和函数（没有this指针）
- 不能声明为virtual, const, volatile


## 8、C++多态的实现
#### 多态的定义：“一种接口，多种调用”
- 编译时多态：函数重载、运算符重载
- 运行时多态：类的继承与虚函数
- 此处注意重写与重载的区别（都是实现多态的方式）：
1.  重载：类内、类外，需要有相同的函数名，不同的参数列表
2.  重写：发生在类间（子类和基类），函数名、参数列表、返回值类型都要与原函数相同，父类中需要有virtual修饰

#### 虚函数的注意事项
- 构造函数不能为虚函数，而析构函数一般声明为虚函数，释放基类指针时可以实际释放子类空间
- C++11的override与final关键字：
1. override强制重写虚函数（也可以防止写错虚函数的函数名）
2. final表示最终虚函数，之后不能再被重写

#### 虚表机制（回顾类的大小计算以及C语言实现面向对象）
- 存放该类中所有虚函数的地址，放置在该对象地址空间的最前面，这是为了使派生类快速获得虚函数表
- 派生类虚表的构造过程：
1. 将基类虚表中的虚函数拷贝一份到派生类虚表中
2. 对于每个派生类重写了的虚函数对原相同偏移量的函数进行覆盖
3. 新增加的虚函数按照类中的声明次序加到派生类第一张虚表的最后（多继承即拥有多个基类时，会有多张虚表），这是为了让不同基类的指针都能调用到子类的虚函数实现

#### 图解
![image](https://img-blog.csdn.net/20180820143644168?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM2MzU5MDIy/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

![image](https://img-blog.csdn.net/20180821095203702?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM2MzU5MDIy/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

![image](https://img-blog.csdn.net/20180820162125485?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM2MzU5MDIy/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

![image](https://img-blog.csdn.net/20180821103628852?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM2MzU5MDIy/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

![image](https://img-blog.csdn.net/20181023164040667?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzM2MzU5MDIy/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

#### 参考文章
- [C++多态的实现](https://blog.csdn.net/timecur/article/details/90045486)
- [C++虚表实验](https://blog.csdn.net/timecur/article/details/95981159)
- [C++虚函数表图解](https://blog.csdn.net/qq_36359022/article/details/81870219)

## 9、inline关键字分析
#### C/C++引入inline的原因
对于短小的函数，调用需要经过开辟栈空间、参数压栈、跳转、结果返回等，使用inline编译器对其直接在原地展开，提升了程序运行的速度

#### 给编译器的一种建议
内联是以代码膨胀作为代价的，对于长函数、包含循环、switch、递归等复杂函数，编译器可以决定不理睬

#### 类成员函数与inline
- 定义在类中的函数默认都是内联的
```cpp
class A
{
    public:void Foo(int x, int y) {  } // 自动地成为内联函数
}
```

- 定义在类外的函数可以被声明为内联的（推荐做法）
```cpp
// 头文件
class A
{
    public:
    void Foo(int x, int y);
}
// 定义文件
inline void A::Foo(int x, int y){}
```

#### virtual虚函数可以被声明成inline吗？
看情况，当虚函数表现为多态性时不行。究其原因，最本质是因为inline是编译时展开，编译器需要在编译时确定其具体代码，而虚函数（运行时多态性）必须要到运行时才能确定执行哪个函数。

举例：
```cpp
#include <iostream>  
using namespace std;
class Base
{
public:
   inline virtual void who()
   {
   	cout << "I am Base\n";
   }
   virtual ~Base() {}
};
class Derived : public Base
{
public:
   inline void who()  // 不写inline时隐式内联
   {
   	cout << "I am Derived\n";
   }
};

int main()
{
   // 此处的虚函数 who()，是通过类（Base）的具体对象（b）来调用的，
   // 编译期间就能确定了，所以它可以是内联的，但最终是否内联取决于编译器。 
   Base b;
   b.who();

   // 此处的虚函数是通过指针调用的，呈现多态性，需要在运行时期间才能确定，
   // 所以不能为内联。  
   Base *ptr = new Derived();
   ptr->who();

   // 因为Base有虚析构函数（virtual ~Base() {}），
   //所以 delete 时，会先调用派生类（Derived）析构函数，
   //再调用基类（Base）析构函数，防止内存泄漏。
   delete ptr;
   ptr = nullptr;
   
   return 0;
} 
```

#### 其他
###### 建议inline放在头文件中
声明应和定义一致，编译器必须随处可见内联函数的定义，因此放在头文件中比较合适

###### 用于实现的关键字
仅与声明放在一起是无效的
```cpp
inline void Foo(int x, int y); // inline 仅与函数声明放在一起
void Foo(int x, int y){}
```

## 10、C++ STL（速成版，后面要看侯捷的STL源码剖析）
#### 介绍
标准模板库（STL），它的基本概念就是把==数据和操作分离==，含有容器、算法、迭代器组件等。迭代器是容器和算法之间的粘合剂，使任何算法都可以和任何容器进行交互操作。

在STL中体现了==泛型程序设计==的思想，是以类型参数化的方式实现的（模板）。

## 容器
#### STL中的容器
- 序列容器：vector   string   deque   list
- 关联容器：set   map   multiset   multimap
- 适配容器：stack   queue   priority_queue（优先队列）

#### 使用场景
1. 如果你需要高效的随机存取，而不在乎插入和删除的效率，使用vector
2. 如果你需要大量的插入和删除，而不关心随机存取，则应使用list
3. 如果你需要随机存取，而且关心两端数据的插入和删除，则应使用deque。
4. 如果你要存储一个数据字典，并要求方便地根据key找value，那么map是较好的选择
5. 如果你要查找一个元素是否在某集合内存中，则使用set存储这个集合比较好

#### vector 和 list 的区别
###### vector
- 底层实现是动态数组（连续存储容器）
- 如果没有剩余空间了，则会重新配置原有元素的两倍空间，然后将原空间元素以复制的方式初始化新空间，再增加新的元素
- ==使用场景==：经常随机访问，且不经常对非尾部结点进行插入操作

###### list
- 底层实现是双向链表（不连续），每插入一个元素都会分配空间，每删除一个元素都会释放空间
- 随机访问性能很差，只能快速访问头尾结点
- ==使用场景==：经常插入删除大量数据

#### map和set有什么区别
###### map
- 关联容器，底层为红黑树
- 元素是key-value(关键字-值对)，关键字起到索引作用，值则表示与索引相关联的数据
- 允许修改值，支持下标操作
###### set
- 关联容器，底层为红黑树
- set是关键字的集合，每个元素只包含一个关键字
- const型不能修改元素值，不支持下标操作

#### unordered_map和map的区别
###### 底层实现
map为红黑树，自动排序功能，插入、删除、查找的时间复杂度均为O(logN)；而unordered_map底层为哈希表，查找的效率为O(1)，但哈希表的建立比较耗时

###### 优缺点总结
- 内存占有率，哈希表更高
- unordered_map执行效率更高
- 对于有顺序要求的问题，map更为合适

#### STL迭代器的作用，为何有了指针还需要迭代器？
迭代器不是指针，而是类模板，通过重载一些指针运算符使其表现地像指针；这是一种指针概念的提升，提供了比指针更高级的行为，把不同集合类的访问逻辑抽象出来，使得不用暴露集合内部的结构而达到循环遍历集合的目的

#### 红黑树知识补充
- 平衡二叉树又称为AVL树，是一种特殊的二叉排序树。其左右子树都是平衡二叉树，且左右子树高度之差的绝对值不超过1。

- 红黑树是一种二叉查找树，但在每个节点增加一个存储位表示节点的颜色，可以是红或黑（非红即黑），红黑树是一种弱平衡二叉树，相对于要求严格的AVL树来说，它的旋转次数少，所以对于搜索，插入，删除操作较多的情况下，通常使用红黑树。

- 所以红黑树在查找，插入删除的性能都是O(logn)，且性能稳定，所以STL里面很多结构包括map底层实现都是使用的红黑树。

#### 哈希表知识补充
- 哈希表的实现主要包括构造哈希和处理哈希冲突：构造哈希，主要包括直接地址法，除留余数法。

- 处理哈希冲突：当哈希表关键字集合很大时，关键字值不同的元素可能会映射到哈希表的同一地址上，这样的现象称为哈希冲突。常用的解决方法有：

1. 开放定址法，冲突时，用某种方法继续探测哈希表中的其他存储单元，直到找到空位置为止。（如，线性探测，平方探测）

2. 再哈希法：当发生冲突时，用另一个哈希函数计算地址值，直到冲突不再发生。

3. 链地址法：将所有哈希值相同的key通过链表存储，key按顺序插入链表中。