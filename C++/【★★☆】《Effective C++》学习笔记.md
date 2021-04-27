## 条款01：视C++是一个语言联邦
- C：块、语句、预处理器、内置数据类型、数组、指针
- Object-Oriented C++：类、封装、继承、多态
- Template C++：泛型编程，TMP模板元编程
- STL：对容器、迭代器、算法以及函数对象的规约有极佳的紧密配合与协调

## 条款02：尽量以const,enum,inline替换#define
#### 使用const

```cpp
#define ASPECT_RATIO    1.653
const double AspectRatio = 1.653;
```

- 宏定义的记号名称可能不会出现在编译错误信息中，不利于修改
- 宏定义的记号不会出现在记号表中，不利于调试
- 预处理器盲目地将#define替换，可能导致更大的代码量
- 类型安全（这本书上没说）


class专属常量的声明方式：

```cpp
class GamePlayer{
private:
    static const int NumTurns = 5;
    int scores[NumTurns];
};

const int GamePlayer::NumTurns;
```

```cpp
class GamePlayer{
private:
    static const int NumTurns;
    int scores[NumTurns];
};

const int GamePlayer::NumTurns = 5;
```

#### 使用enum
如果编译器恰好只允许上面第2中声明定义方式，则可以改用enum，有一个好处是enum和#define一样不会导致非必要的内存分配
```cpp
class GamePlayer{
private:
    enum { NumTurns = 5 };
    int scores[NumTurns];
};
```

#### 使用inline
#define的一种用法是用宏定义实现类似函数，但不会招致函数调用带来的额外开销，但这种用法是危险的！

```cpp
#define CALL_WITH_MAX(a,b)  f((a) > (b) ? (a) : (b))

CALL_WITH_MAX(++a, b);
CALL_WITH_MAX(++a, b+10);
```

在上述代码中，即便已经很小心地对每个a和b均加上了()，但仍然会导致后面两种调用时a累加次数的问题。而==改用inline和template的结合便可以带来如同宏定义一般的效率和泛型==
```cpp
template<typename T>
inline void callWithMax(const T &a, const T &b){
    f(a > b ? a : b);
}
```

## 条款03：尽可能使用const
#### 指针常量和常量指针
- 见《C和指针》学习
- 与STL迭代器结合：将迭代器声明为const类型则是指针常量，而const_iterator才是常量指针

```cpp
vector<int> vec;

const vector<int>::iterator iter = vec.begin();
*iter = 10;     //正确
++iter;         //错误

vector<int>::const_iterator cIter = vec.begin();
*cIter = 10;    //错误
++cIter;        //正确
```

#### 令函数返回值为一个常量值
往往能够降低因客户错误而造成的以外，例如重载operator*运算符时：

```cpp
const Rational operator* (const Rational *lhs, const Rational *rhs);
```
可以避免发生形如if(a * b = c)这样的错误。

#### const成员函数
const成员函数能够重载同名同参数的成员函数，这是C++的一个重要特性。

```cpp
class TextBlock{
public:
    const char& operator[] (size_t position) const{
        return text[position];
    }
    char& operator[] (size_t position){
        ;return text[position];
    }
private:
    string text;
};
```
这里注意返回的都是引用，若返回的是值则可能发生编译错误，或者修改的只是一个副本。

#### 允许const成员函数修改对象内的某些bits（释放bitwise约束）
可以通过对需要修改的成员变量添加`mutable`

```cpp
class CTextBlock{
public:
    size_t length() const;
private:
    char *pText;
    mutable size_t textLength;
    mutable bool lengthIsValid;
};

size_t CTextBlock::length() const {
    if(!lengthIsValid){
        textLength = strlen(pText);
        lengthIsValid = true;
    }
    return textLength;
}
```

#### 书写不当的函数能通过bitwise测试，却不符合const性质
其中由于const成员函数返回了一个指向对象内部的值，使得在外部能够错误地修改const对象内部的成员变量。（这点在后面的条款中还能提到）
```
#include <iostream>
#include <cstring>
using namespace std;

class CTextBlock{
private:
    char *pText;

public:
    CTextBlock(char *text){
        pText = new char[strlen(text)+1];
        strcpy(pText, text);
        pText[strlen(text)] = '\0';
    }
    void print_text() const{ cout << string(pText) << endl; }
    char & operator[] (size_t position) const{
        return pText[position];
    }
};

int main(){
    char pText[] = "Hello";
    const CTextBlock tb(pText);
    tb.print_text();
    tb[1] = 'a';
    tb.print_text();
    return 0;
}
```

#### const和非const成员函数中避免重复
假设在opertor[]中，除了返回下标处的值之外，还有执行一系列诸如边界检验、记录数据访问、检验数据完整性等一系列操作，则const和非const都要定义一份，显得代码臃肿（但不是不行！）解决办法是，使用const_cast去除const属性用于定义非const版本

```cpp
class TextBlock{
public:
    ...
    const char & operator[] (size_t position) const{
        ...
        ...
        ...
        return text[position];
    }
    char & operator[] (size_t position){
        return const_cast<char &>(
            static_cast<const TextBlock&>(*this)[position]
        );
    }
};
```

注意几点：
- 非const中单纯调用[]（没有static_cast一步），会导致无穷递归
- const_cast去const属性是危险的，且很少是正确的选择！
- 为什么是先定义const版本再转non-const版本而不是反过来？——因为const成员函数调用非const是错误的行为。


## 条款04：确定对象被使用前已先被初始化
#### 对于内置数据类型最好手工对其进行初始化
C++不保证初始化它们，可能得到的是一个未明确的初值

#### 使用初始化列表代替构造函数中的赋值
这里需要区分：初始化与赋值的区别，==初始化具有比赋值更高的效率==，因为赋值时被赋值的对象会调用default构造函数但立马又被copy赋予新值，而初始化只会调用一次copy，这对于内置数据类型没有区别，但对于自定义数据类型往往有着本质的效率差异；==若成员变量为const或者reference，则只能使用初始化==，原因是const和reference不能被赋值，最简单的方法就是不管三七二十一都使用成员初始列；==成员初始化顺序十分固定取决于声明次序==

#### 十分棘手的跨编译单元之间的初始化次序
即无法保证某一编译单元A在另一单元B之前初始化，若B初始化时用到了尚未初始化的A对象，则可能出现未知的问题。解决方法是设计模式中的单例模式：

```cpp
class FileSystem{...};
FileSystem & tfs(){
    static FileSystem fs;
    return fs;
}
```
C++保证局部的static变量会在首次调用时被初始化，但程序终止时被析构。


## 条款05：了解C++默默编写并调用了哪些函数
当你写下class Empty {};这个空类时，编译器实际会按需（如果被调用的话）生成以下代码：

```cpp
class Empty{
public:
    Empty(){...}
    Empty(const Empty &rhs){...}
    ~Empty(){...}
    Empty & operator= (const Empty &rhs){...}
};
```

考虑下面两个问题：
- 若被拷贝的对象中含有const或者reference会如何？默认的拷贝构造函数不会有问题，因为这是逐bit的==初始化==，而赋值运算符中的==赋值==操作是不被允许的。
- 若基类的operator=被声明为private，则编译器拒绝为派生类生成operator=操作符，导致派生类的operator=无法被调用。

## 条款06：若不想使用编译器自动生成的函数，就该明确拒绝
如果你不希望编译器为你自动生成拷贝构造函数和赋值运算符，则可以使用以下的方式拒绝：

```cpp
class HomeForSale{
public:
    ...
private:
    HomeForSale(const HomeForSale &);
    HomeForSale & operator= (const HomeForSale &);
};
```

实现了以下几点：
- 将复制构造函数和赋值运算符==设为private类型==，阻止了编译器自动生成的相应函数，若被调用会==发生编译错误==。
- 做到以上一条仍然能够被成员函数和友元friend所调用，因此==只给出声明而不给出实现==，若不幸被调用会==给出链接错误==。
- 以上两点在iostream库中被广泛使用


此外，将链接期错误移至编译期也是可能的，可==定义一个基类专门用于不可拷贝不可赋值的操作类型==，此技巧应用于boost库中

```
class Uncopyable{
protected:
    Uncopyable() {}
    ~Uncopyable() {}
private:
    Uncopyable(const Uncopyable &);
    Uncopyable & operator= (const Uncopyable &);
};

class HomeForSale: private Uncopyable{
    ...
};
```

## 条款07：为多态基类声明virtual析构函数
#### 带有多态性质的基类需要声明一个virtual析构函数
判断标准是该class中有无带有virtual的函数，若没有声明则会导致如下问题：基类指针指向派生类对象，delete调用的是基类析构函数，而派生类的部分参数将造成资源泄露；析构函数正确的运作方式为：==最深层的派生类先被析构，然后逐层调用基类析构函数==。

#### 如果类的设计不是为了基类或不具备多态属性，析构函数不应该声明成virtual
这是因为virtual析构函数需要在对象中携带虚表指针，导致类占用的空间变大，却无卵用。

#### 带有pure virtual的析构函数
在抽象类定义中（含有纯虚函数的），除了要声明析构函数为纯虚函数之外，还需要提供一份纯虚析构函数的定义，这是因为在逐级调用基类的析构函数时，会在派生类析构函数中创建一个基类析构函数的调用，若找不到相关实现，链接器将报错

```cpp
class AWOV{
public:
    virtual ~AWOV() = 0;
};

AWOV::~AWOV(){ }
```

## 条款08：别让异常逃离析构函数
#### 不应该在析构函数中抛出异常
当有多个元素需要析构抛出异常时，当多个异常同时存在的情况下，程序不是==结束执行==就是==导致不明确的行为==。

#### 重新设计设计接口使得异常在普通函数中抛出
在下例中，提供了close()方法给客户处理错误的机会，否则将没有机会响应。若客户能够坚信没有问题则可以自信地依赖析构函数去close.
```cpp
class DBConn{
public:
    void close(){
        db.close();
        closed = true;
    }
    
    ~DBConn(){
        if(!closed){
            try{
                db.close();
            }
            catch(...){
                其他
            }
        }
    }
private:
    DBConnection db;
    bool closed;
};
```


## 条款09：绝不在构造和析构过程中调用virtual函数
原因在于在Base的构造函数期间，virtual函数不是virtual函数，同理，当析构函数进入派生类的Base部分时，它就应该被看作是一个Base对象。

```cpp
#include <iostream>
using namespace std;

class Transaction{
public:
    Transaction();
    virtual void logTransaction(){
        cout << "Base" << endl;
    };
};

Transaction::Transaction(){
    logTransaction();
}

class BuyTransaction: public Transaction{
public:
    virtual void logTransaction() const;
};

void BuyTransaction::logTransaction() const{
    cout << "Buy" << endl;
}


int main(){
    BuyTransaction buy;
    return 0;
}
```

以上代码中将输出`Base`，该问题可以通过让派生类将必要的构造信息向上传递给基类构造函数（通过virtual是自上向下）：

```cpp
TransactionL::Transaction(const string &info){
    logTransaction(info);   //该函数现在被声明为non-virtual
}

BuyTransaction(parameters)
: Transaction(createLogString(parameters)){...}
```

## 条款10：令operator=返回一个reference to *this
原因是赋值由此可以写成连锁形式：`x = y = z = 15;`，则根据右结合律，将15依次赋给z,y,x；需要注意的是这是一种约定俗称，不这么做也能通过编译，但这份协议被所有内置类型和标准库所遵守。

```cpp
class Widget{
public:
    Widget & operator= (const Widget &rhs){
        ...
        return *this;
    }
    Widget & operator+= (const Widget &rhs){
        ...
        return *this;
    }
};
```


## 条款11：在operator=中处理“自我赋值”
#### 为什么要考虑自我赋值？

```cpp
//比较蠢的做法
Widget w;
w = w;

//潜在的自我赋值
a[i] = a[j];
*px = *py;
```

#### 一种实现：没有考虑new抛出异常
```cpp
Widget & Widget::operator=(const Widget &rhs){
    if(this == &rhs) return *this;
    
    delete pb;
    pb = new Bitmap(*rhs.pb);
    return *this;
}
```
此时当new抛出异常后，会退出该函数导致赋值其实没有完成，pb指向的是已经被delete掉的内存

#### 改进：精心布置语句顺序
```cpp
Widget & Widget::operator=(const Widget &rhs){
    Bitmap *pOrig = pb;
    pb = new Bitmap(*rhs.pb);
    delete pOrig;
    return *this;
}
```

#### copy-and-swap技术
这是一个常见而够好的operator=撰写方式，有两种写法：（第二种伶俐巧妙，且可能有助于编译器生成更高效地代码，但牺牲了清晰性）

```cpp
Widget & Widget::operator=(const Widget *rhs){
    Widget temp(rhs);       //copy
    swap(temp);             //条款29
    return *this;
}
```

```cpp
Widget & Widget::operator=(Widget rhs){ //这里使用by value方式传入
    swap(rhs);
    return *this;
}
```


## 条款12：复制对象时勿忘其每一个成分
当选择自己实现copy构造函数和copy赋值函数时，编译器几乎不会提供可能出错的提示，主要有下面两种情况
#### 当你为class里新增一个成员变量后
你需要为每一个构造函数和赋值运算符中添加这个成员变量的初始化/赋值，否则会出现局部拷贝，且编译器不会给出任何提示

#### 派生类的copy时需要考虑基类部分
派生类的copy应该调用基类的构造函数完成对该对象基类部分的赋值：

```cpp
PriorityCustomer::PriorityCustomer(const PriorityCustom &rhs)
    :   Customer(rhs),
        priority(rhs.priority)
{
    ...
}

PriorityCustomer & PriorityCustomer::operator=(const ProrityCustomer &rhs){
    Customer::operator=(rhs);
    priority = rhs.priority;
    return *this;
}
```

- 无论是在copy构造函数调用copy赋值运算符还是反之，都是没有意义的
- 如果在copy构造函数和copy赋值运算符有大量相同代码，可以考虑使用第三个函数init()完成，并把它声明为private.


## 条款13：以对象管理资源
此条引入“==智能指针==”的概念（见C++11新特性学习），体现了两个关键想法：1. 获得资源后立即放进管理对象RAII、2. 管理对象运用析构函数确保资源被释放。

## 条款14：在资源管理类中小心copying行为
还是上面这个话题，你要充分考虑copy该资源的时候会发生什么，怎么做，4种方式：
#### 禁止复制：见条款06
#### 对底层资源引用计数：shared_ptr就是这么做的
#### 复制底部资源：所谓深拷贝
#### 转移底部资源的拥有权：auto_ptr


## 条款15：在资源管理类中提供对原始资源的访问
资源管理类是一个对抗资源泄露的堡垒，但很多API直指资源本身，需要资源管理类能够提供对原始资源的访问。

#### 智能指针中的用法
shared_ptr()提供get()方法返回原始资源的指针；同时auto_ptr和shared_ptr都重载了*和->运算符，允许他们隐式转换为底层指针对资源进行访问。

#### 显式与隐式
显式比较安全，而隐式对客户相对友好但可能发生歧义

```cpp
//显式
class Font{
public:
    FontHandle get() const { return f; }
};

//隐式
class Font{
    operator FontHandler() const { return f; }
};

//但是考虑这种用法
Font f1(getFont());
FontHandle f2 = f1;
```
以上例子中，原意是拷贝一个Font对象，但由于定义了隐式转化函数使得发生了歧义，实际的效果是f2和f1均指向这块Font资源，当f1被销毁时资源随之释放，f2由此变成虚吊的。


## 条款16：成对使用new和delete时要采用相同形式
这里主要将的是new[]和delete[]需要相对应，考虑下面两个问题：
- 当class中提供了多种构造函数，请确保每一个都是以相同的new形式创建的，这样才能在析构函数中使用对应的delete
- 对于typedef：
```cpp
typedef string AddressLines[4];
string *pal = new AddressLines;
delete[] pal;   //这才是对的
```
为了避免此类错误，最好不要对数组形式使用typedef动作，事实上，应该多使用vector等容器，vector<string>


## 条款17：以独立语句将new出的对象置入智能指针
考虑这样一个例子：

```cpp
processWidget(shared_ptr<Widget>(new Widget), priority());
```

乍一看没有什么问题，但是C++对于调用priority()、new Widget、shared_ptr构造函数，这三个步骤的次序是没有严格规定的，若先new Widget，再在priority()中抛出异常，则new出的对象无法传入智能指针，由此导致了内存泄露。==因此务必将new出对象置于智能指针的操作以独立的语句呈现==。

```cpp
shared_ptr<Widget> pw(new Widget);
processWidget(pw, priority());
```


## 条款18：让接口容易被正确使用，不易被误用
考虑如下的Date类：

```cpp
class Date{
public:
    Date(int month, int day, int year);
};
```

- 为了避免用户将月份和日期等输反，可以考虑以下操作，将Day、Month声明成结构体，并规定explicit显式转换：

```cpp
struct Month{
    explicit Month(int m)
        : val(m){ }
    int val;
};

Date d(Month(3), Day(30), Year(1995));
```

- 这里又应该考虑到，月份的取值是有限制的，可以考虑以下方法：

```
class Month{
public:
    static Month Jan() { return Month(1); }
    static Month Feb() { return Month(2); }
    ...
    static Month Dec() { return Month(12); }
private:
    explicit Month(int m);
    ...
};

Date d(Month::Mar(), ...);
```

## 条款19：设计class犹如设计type


## 条款20：宁以pass-by-reference-to-const替换pass-by-value
- 对于内置类型和STL迭代器和函数对象二元，pass-by-value的代价并不昂贵，但对于其他自定义数据类型，传入const引用能够更加高效，有效解决切割问题
#### 更高效地参数传递

```cpp
bool validateStudent(const Student &s);
```

#### 避免切割问题
考虑下面两个类：

```cpp
class Window{
public:
    virtual void display() const;
};

class WindowWithScrollBars: public Window{
public:
    virtual void display() const;
};

void Display(Window w);

WindowWithScrollBar wwsb;
Display(wwsb);
```
参数w会被构造成一个Window对象，是以值传递的，多态性就会被切除，而使用const引用传递则将保持多态性，调用的实际是派生类的display()函数，因为引用往往以指针来实现的。


## 条款21：必须返回对象时，别妄想返回其reference
考虑两个有理数相乘的情况，你可能想通过传递引用或者指针的形式减少复制构造函数和析构函数的调用，于是写出了以下3种可能犯错的代码：

```cpp
const Rational & operator* (const Rational &lhs, const Rational &rhs){
    Rational result(...);
    return result;
}
```
错误原因：试图返回一个局部变量的引用

```cpp
const Rational & operator* (const Rational &lhs, const Rational &rhs){
    Rational *result = new Rational(...);
    return *result;
}
```
错误原因：当调用诸如x * y * z的代码时，返回将调用两次new，导致内存泄露

```cpp
const Rational & operator* (const Rational &lhs, const Rational &rhs){
    static Raional result;
    result = ...
    return result;
}
```
错误原因：每次看到的都只能是static的现值，执行如同if(x * y == w * z)时永远将是true

此时我们不能贪图通过返回引用带来有限的性能提升，而应为了保证正确的行为而承担复制构造和析构的成本。

```cpp
const Rational operator* (const Rational &lhs, const Rational &rhs){
    return Rational(...);
}
```

## 条款22：将成员变量声明为private
切记：你要将成员变量声明为private，这可以赋予客户访问数据的一致性、可细微划分访问控制、允诺约束条件获得保证，并提供class作者以充分的实现弹性。protected成员变量不会比private成员变量更具有封装性，因为一旦有某一成员变量涉及到更改，都会引发不可预知的大量代码收到破坏。


## 条款23：宁以non-member、non-friend替换member函数
考虑以下clearBrowser()函数的实现：
- member

```
class WebBrowser{
public:
    void clearCache();
    void clearHistory();
    void removeCookies();
    void clearBrowser(){
        clearCache();
        clearHistory();
        removeCookies();
    }
};
```
- non-member

```cpp
void clearBrowser(WebBrowser &wb){
    wb.clearCache();
    wb.clearHistory();
    wb.removeCookies();
}
```

non-member可以比member提供更好的封装性、包裹弹性、机能扩充性。考虑对象内的数据，==越少代码可以看到数据/访问数据，越多的数据可被封装==。


## 条款24：若所有参数皆需类型转换，请为此采用non-member函数
考虑为有理数类Rational提供乘法运算符的支持，允许整数隐式转换为有理数这个要求看上去颇为合理。但若为Rational提供operator*的成员函数，写法是这样的：

```cpp
class Rational{
public:
    const Rational operator* (const Rational &rhs) const;
};
```

这样的写法会引发一个问题：考虑以下写法：

```cpp
result = oneHalf * 2;
result = 2 * oneHalf;
```

其中第一种写法会调用operator*成员函数，并将2调用构造函数隐式转换为Rational，而第二种写法将会报错。为了解决此问题，应将operator *函数定义为非成员函数。这里还有一个点需要注意的是，==这是不涉及模板C++的编写方式，设计模板的见条款46.==

```cpp
const Rational operator* (const Rational &lhs, const Rational &rhs){
    return Rational(...);
}
```


## 条款25：考虑写出一个不抛异常的swap()函数
- 缺省情况下的swap()函数：涉及到三个对象的复制，对于某些pimpl手法而言不够高效

```cpp
namespace std{
    template<typename T>
    void swap(T &a, T &b){
        T temp(a);
        a = b;
        b = temp;
    }
};
```
pimpl手法，见条款31，置换两个Widget只需要交换两个指针。
```
class WidgetImpl{
public:
    ...
private:
    int a, b, c;
    std::vector<double> v;
    ...
};

class Widget{
public:
    ...
private:
    WidgetImpl *pImpl;
};
```

- 解决：提供一个member的swap，也提供non-member swap版本调用前者，==特化==std::swap

```cpp
class Widget{
public:
    void swap(Widget &other){
        using std::swap;
        swap(pImpl, other.pImpl);
    }
};

namespace std{
    template<>
    void swap<Widget>(Widget &a, Widget &b){
        a.swap(b);
    }
}
```

- 考虑更复杂的情形，Widget定义为类模板

```cpp
template<typename T>
class Widget{...};
```

此时我们不能在std中添加全新的重载函数进去，比较合适的方法是，将Widget类和swap函数定义到全新的命名空间中。

```cpp
namespace WidgetStuff{
    template<typename T>
    class Widget{...};
    
    template<typename T>
    void swap(Widget<T> &a, Widget<T> &b){
        a.swap(b);
    }
}
```

此时调用函数中应该这么编写：

```cpp
template<typename T>
void doSomething(T &obj1, T &obj2){
    using std::swap;
    swap(obj1, obj2);
}
```

此时根据C++的==名称查找法则==，swap会优先搜寻Widget所在命名空间的swap函数，当不幸没有搜到时，会调用std空间的缺省swap函数，这得益于第一句将std::swap接口暴露出来。


## 条款26：尽可能延后变量定义式的出现时间
- 考虑以下情形，在计算密码的加密版本的函数中，需要首先对密码长度进行检查，过短则抛出异常，随后对密码进行加密：

```cpp
std::string encryptPassword(const std::string &password){
    using namespace std;
    string encryped;
    if(password.length() < MinimumPasswordLength){
        throw ...;
    }
    
    encrypted = password;
    encrypt(encryped);
    return encrypted;
}
```

在该代码中，过早地定义了string encryped，导致若丢出异常仍要承受构造与析构函数，此外，在条款4中提到直接在构造时提供初值比先构造出一个对象再进行赋值效率高，因此进行如下修改：

```cpp
std::string encryptPassword(const std::string &password){
    ...
    string encrypted(password);
    encrypt(encrypted);
    return encrypted;
}
```

- 如果一个变量只在循环中使用，那么应定义于循环内or循环外？——定义在循环内使得代码更易可读和维护，因此除非明确赋值操作比构造+析构的开销低，并且这段代码是高度效率敏感的，应该采用定义在循环内的方法（尽可能延后）
```cpp
Widget w;
for(int i = 0; i < n; i++){
    w = ...
}

for(int i = 0; i < n; i++){
    Widget w(...);
}
```


## 条款27：尽量少做转型动作
#### C++的转型系统
- 旧时：(T)exp——C语言风格，T(exp)函数风格
- 新式：4种xxx_cast<T>(exp)

#### const_cast
唯一有能力将常量去除的转型操作符

#### dynamic_cast
- 执行“安全向下转型”，用于决定某对象是否归属继承体系的某个类型，可能耗费重大运行成本，底层会执行多次strcmp用于比较class名，特别应该避免的：深度继承、多重继承、连串dynamic_cast
- 如何避免使用dynamic_cast

```cpp
typedef std::vector<std::tr1::shared_ptr<Window>> VPW;
VPW winPtrs;

for(VPW::iterator iter = winPtrs.begin(); iter != winPtrs.end(); ++iter){
    if(SpecialWindow *psw = dynamic_cast<SpecialWindow *>(iter->get()))
        psw->blink();
}
```

1. 提供类型安全容器，当需要处理多种窗口类型时，可能需要多种容器

```cpp
typedef std::vector<std::tr1::shared_ptr<SpecialWindow>> VPSW;
VPSW winPtrs;

for(VPSW::iterator iter = winPtrs.begin(); iter != winPtrs.end(); ++iter){
    (*iter)->blink();
}
```

2. 可以考虑在基类Window中提供virtual函数（如果不具备该功能可以什么都不做），在各个派生类提供具体实现

```cpp
for(VPW::iterator iter = winPtrs.begin(); iter != winPtrs.end(); ++iter){
    (*iter)->blink();
}
```

#### reinterpret_cast
意图执行低级转型，具体实现可能取决于编译器，是不可移植的。在本书中只有在条款50中：针对原始内存写一个调试用的分配器时用到了该转型。

#### static_cast
强迫隐式转型，例如non_const向const转（反之则不可以），int转double，基类指针转派生类指针。

- 尽量不使用旧式转型，唯一推荐使用的场景：explicit构造函数，Widget(15)
- 注意以下似是而非的代码是错误的：

```cpp
class Window{
public:
    virtual void OnResize(){...}
};

class SpecialWindow : public Window{
public:
    virtual void OnResize(){
        static_cast<Window>(*this).OnReszie();
        ...
    }
};
```

该代码的本意是，在SpecialWindow中除了执行基类动作之外，还执行自身额外的动作；但实际的执行效果是，这个OnResize()针对的是一个转型后的暂时副本身上，实际应该这么实现：

```cpp
class SpecialWindow : public Window{
public:
    virtual void OnResize(){
        Window::OnResize();
        ...
    }
};
```


## 条款28：避免返回handlers指向对象内部成分
基于重要原因：==确保封装性、真正的const、避免虚吊号码牌==

- 成员变量的封装性最多只等于返回其reference函数的访问级别，因为传出了其引用，且外部能够调用const成员函数修改内部的值。解决方法是返回const refrence

```cpp
const Point& upperLeft() const { return pData->ulhc; }
```

- 但上述代码依然有风险：虚吊号码牌，如下例，根据条款3，boundingBox()被设计成值返回的，于是在语句结束后该返回的临时副本被销毁，导致pUpperLeft实际指向一个已经被销毁了的对象内部

```cpp
const Point * pUpperLeft = &(boundingBox(*pgo).upperLeft());
```

## 条款29：为“异常安全”而努力是值得的
异常安全函数需要提供以下3个保证之一：
- 基本承诺：程序内任何事物均保持在有效状态下，但可能无法预知在哪一状态
- 强烈保证：调用函数需要有这样的确认，如果函数成功，就是完全成功，若失败，则恢复到“调用函数之前”。
- 不抛掷nothrow

不提供异常安全的函数：其中一旦new抛出bad_alloc异常，unlock将永远无法得到执行
```cpp
void PrettyMenu::changeBackGround(std::istream &imgSrc){
    lock(&mutex);
    delete bgImage;
    ++imageChanges;
    bgImage = new Image(imgSrc);
    unlock(&mutex);
}
```

修改为其提供强烈保证：
```cpp
class PrettyMenu{
    std::tr1::shared_ptr<Image> bgImage;
    void changeBackGround(std::istream &imgSrc){
        Lock ml(&mutex);
        bgImage.reset(new Image(imgSrc));
        ++imggeChanges;
    }
};
```

有一个一般化的设计策略可以实现强烈保证，即copy-and-swap，思想是在副本上做一切必要的修改，确认无异常后才赋给真正的值。


## 条款30：透彻了解inline的里里外外
- 优点：免除函数调用成本，编译器或许将有能力对函数本体的执行语境相关最优化
- 缺点：增加目标代码的大小，程序体积过大，导致额外的换页行为和缓存命中率的丢失，重新编译的负担


## 条款31：将文件间的编译依存关系降至最低
==一般的构想是：相依于声明式，而非定义式，两个手段是Handle classes和Interface== classes，优点是解除了接口和实现之间的耦合关系，缺点是付出一定的性能和内存开销

#### Handle classes
将对象实现细节隐藏在一个指针背后
```cpp
class PersonImpl;
class Date;
class Address;

class Person{
private:
    std::tr1::shared_ptr<PersonImpl> pImpl;
    
public:
    Person(const std::string &name, const Date &birthday, 
            const Address &addr);
    std::string name() const;
};

Person::Person(const std::string &name, const Date &birthday, 
                const Address &addr)
    : pImpl(new PersonImpl(name, birthday, addr)){}
    
std::string Person::name() const{
    return pImpl->name();
}
```

#### Interface classes
令Person为一种特殊的抽象基类，意义在于一一描述继承类的接口，不带成员变量也没有构造函数

```cpp
class Person{
public:
    virtual ~Person();
    virtual std::string name() const = 0;
    ...
    static std::tr1::shared_ptr<Person>create(
        const std::string &name, 
        const Date &birthday,
        const Address &addr
    )
};

class RealPerson: public Person{
public:
    RealPerson(const ...):...{}
    virtual ~RealPerson(){}
    std::string name() const;
    
private:
    std::string name;
    ...
};

std::tr1::shared_ptr<Person> Person::create(const ...){
    return std::tr1::shared_ptr<Person>(new RealPerson(...));
}
```


## 条款32：确定你的public继承关系塑模处is-a关系
以C++进行面向对象编程，最重要的一个原则是，==public继承意味着is-a关系==。假设class D以public继承class B，则说明B比D表现出更一般化的概念，即D比B表现出更特殊化的概念。因此，B对象可派上用场的任何地方，每一个D对象一样可以派上用场——里式替换原则。

- 比较常规的例子：Persoon和Student
- 比较不常规的例子：Bird和Penguin，这里就存在一个问题，企鹅是一种鸟，鸟是会飞的，于是写出以下代码：
```cpp
class Bird{
public:
    virtual void fly();
    ...
};

class Penguin: public Bird{
    ...  
};
```

但事实上，企鹅是不会飞的，这里就引出了上述关系的逻辑错误，严谨地说，大多数鸟是会飞的，而企鹅是属于不会飞的鸟。一种解决方法是，定义一个会飞的鸟类：
```cpp
class Bird{
    ...
};

class FlyBird: public Bird{
public:
    virtual void fly();
    ...
};

class Penguin: public Bird{
    ...
};
```

另一种解决方法是，为企鹅的fly产生一个运行期错误。
```cpp
class Penguin: public Bird{
public:
    virtual void fly() { error(); }
};
```

好的接口应该要防止无效的代码通过编译，把错误移到编译期，因此第一种方法更好。

- 比较不常规的例子：Rectangle和Squre，一般认为正方形是一种特殊的长方形，但实际上以public继承塑模它们的关系并不准确。


## 条款33：避免遮掩继承而来的名称
- 这其实是一个作用域的问题：==派生类作用域被嵌套在基类的作用域之内==。编译器查找一个变量或函数的流程是这样的：先在派生类作用域寻找，若没找到进而到基类中寻找，若没找到进而在包含Base的namespace作用域中寻找，若没找到进而到global作用域中寻找。
- 遮盖规则：如同local作用域的double x会遮盖global作用域中int x一样，派生类中的函数会遮盖基类中的同名函数（==不论是否具有相同参数类型，不论是否是virtual==）。

```cpp
class Base{
public:
    virtual void mf1() = 0;
    virtual void mf1(int);
    virtual void mf2();
    void mf3();
    void mf3(double);
};

class Derived: public Base{
public:
    virtual void mf1();
    void mf3();
    void mf4();
};
```

事实上，现在在Derived类中，只能看到mf1(),mf3(),mf4()，而mf1(int),mf3(double)被覆盖了。==这对于public继承是不希望如此的==，解决办法是使用using将基类的接口暴露出来。

```cpp
class Base{
public:
    virtual void mf1() = 0;
    virtual void mf1(int);
    virtual void mf2();
    void mf3();
    void mf3(double);
};

class Derived: public Base{
public:
    using Base::mf1;
    using Base::mf3;
    virtual void mf1();
    void mf3();
    void mf4();
};
```

- 同时这个遮盖规则也可以被加以利用，例如private继承中不希望继承base class的所有函数，则可以使用转交函数。
```cpp
class Base{
public:
    virtual void mf1() = 0;
    virtual void mf1(int);
};

class Derived: private Base{
public:
    virtual void mf1(){
        Base::mf1();
    }
};
```

这里mf1()函数被转交，而mf1(int)由于私有又被覆盖而变得不可见了。


## 条款34：区分接口继承和实现继承
#### 声明一个pure virtual函数让派生类之继承函数接口
“你必须提供一个draw()函数，但我不干涉你怎么实现它”，同时pure也是可以提供实现的（见后）

#### 声明一个impure virtual函数让派生类继承该接口和缺省实现
考虑若干有若干不同型号的飞机，每一种飞机都有fly()方法，且原则上需要不同fly()的实现，提供了一份默认的实现，但同时又要防止忘记定义自己的fly()函数，可以考虑以下丝滑的实现方式

```cpp
class AirPlane {
public:
    virtual void fly(const Airport &destination) = 0;
};

void Airplane::fly(const Airport &destination){
    //定义缺省行为
}

class ModelA: public Airplane {
public:
    virtual void fly(const Airport &destination){
        Airplane::fly(destination); //A型号采用缺省行为
    }
};

class ModelC: public Airplane {
public:
    virtual void fly(const Airport &destination){
        ... //自定义实现
    }
}
```

#### 声明一个non-virtual函数让派生类继承类接口及其一份强制实现
不变性与凌驾特异性，不应该在派生类中被重新定义


## 条款35：考虑virtual函数以外的其他选择
#### NVI手法：Non-Virtual Interface
这是Template Method设计模式的一个独特表现形式，优点在于下面“做一些事前工作”和“做一些事后工作”之中，==重新定义virtual函数表示某些事“如何”被完成，而base保留“何时”被调用的权利==。

```cpp
class GameCharacter{
public:
    int healthValue() const{
        ... //事前工作
        int ret = doHealthValue();
        ... //事后工作
        return ret;
    }

private:
    virtual int doHealthValue() const{
        ...
    }
};
```

#### 函数指针实现Strategy模式
优点是提供了很多有趣的弹性设计，缺点是需要适当削弱class的封装性，因为外部函数无从直接访问class内部非public的成员，唯有将其通过public开放出来，实际设计中应做适当权衡。

```cpp
class GameCharacter;    //前置声明
int defaultHealthCacl(const GameCharacter &gc);

class GameCharacter{
public:
    typedef int (*HealthCalcFunc)(const GameCharacter &);
    explicit GameCharacter(HealthCalcFunc hcf = defaultHealthCalc)
        : healthFunc(hcf){}
    
    int healthValue() const{
        return healthFunc(*this);
    }

private:
    HealthCalcFunc healthFunc;
};
```

- 可以为不同的实例赋予不同的健康值计算函数
- 可以为同一实例在运行期设置不同的健康值计算函数

#### tr1::function实现Strategy模式
`std::tr1::function<int (const GameCharacter &)>`

#### 古典的Strategy设计模式
将健康值计算做成一个==分离的继承体系==，不同的健康值计算策略继承此基类提供不同的健康值计算函数。


## 条款36：绝不重新定义继承而来的non-virtual函数
此条应该是针对public继承，因为public继承是一种is-a的关系，non-virtual具有不变性和凌驾其特异性，即适用于B对象的每一件事都应该适用于D对象，B对象的每一个继承类都一定继承其non-virtual成员函数。若发生遮盖隐藏，则违背了上述原则。

## 条款37：绝不重新定义继承而来的缺省参数值
- 首先基于条款36，只应该继承virtual函数；又因为==virtual函数是动态绑定的，而缺省参数值是静态绑定的==。即使在派生类中赋予不同的缺省参数值，通过指针和引用方式调用的仍然是基类的默认缺省参数值。
- 为什么缺省参数值是静态绑定而非动态呢？因为C++兼顾了运行时效率。

## 条款38：通过复合塑模出has-a或“根据某物实现出”
复合(composition)意味着has-a（有一个）或者is-implemented-in-terms-of（根据某物实现出）的关系。

#### has-a
一般很容易区分has-a和is-a的区别，例如在Person中包含Address addr和PhoneNumber faxNumber等，均是has-a关系

#### is-implemented-in-terms-of
例如如果我想要自己实现一个Set（不使用std::set是以为其为了保证性能而占用了更多的内存），考虑采用std::list作为其底层实现。此时不能说Set是一个std::list，而是“Set由std::list实现”。

```cpp
template<class T>
class Set{
public:
    bool member(const T &item) const;
    void insert(const T &item);
    void remove(const T &item);
    std::size_t size() const;
private:
    std::list<T> rep;
};
```

## 条款39：明智而审慎地使用private继承
- private继承体现的是is-implemented-in-terms-of关系，而非is-a.选用策略是，==尽可能使用复合，必要时才选用is-implemented-in-terms-of== （以下两种）
- 需要访问base class的protected成分的或者想要重新定义virtual函数的
- 空间最优的——EBO空白基类最优化

```cpp
class Empty{};

class HoldAnInt{
private:
    int x;
    Empty e;
};
```

```cpp
class HoldAnInt: private Empty{
private:
    int x;
};
```

考虑这两种设计，首先空白类的大小为1的字节，又由于内存对齐导致第一种的HoldAnInt实际上占用了8个字节。采用private继承后（不是内含那一种对象），则只占用4个字节。实际应用中，Empty并非完全空白，可能包含很多typedef, enum, static, non-virtual函数等等，这被广泛应用于STL中.


## 条款40：明智而审慎地使用多重继承
#### 导致某些歧义性
若从一个以上的base继承相同的名称，可能会导致很多歧义，编译器将对此执行最佳匹配，当多个匹配度相同时则产生歧义，正确的做法是显式指定调用哪个：`mp.BorrorableItem::checkout()`

#### 钻石型多重继承带来的问题
基类的成员变量将通过每条路径被复制，到低层次的类中将包含多份来自高层次的成员变量（==默认的做法==），解决：virtual base class虚基类

#### 虚基类
- 需要令所有直接继承自虚基类的class采用虚拟继承，于是IOFile类中只包含一份File类中的成员变量
```cpp
class File{...};
class InputFile: virtual public File{...};
class OutputFile: virtual public File{...};
class IOFile: public InputFile, public OutputFile{...};
```

- 付出的代价：体积大，速度慢，因为编译器将为了避免成员变量的重复做很多额外的手脚。此外，成本还包括初始化成本，virtual base的初始化责任是交给最低层类负责的。

#### 多重继承也有其正当用途
例如：“public继承某个interface class”与“private继承某个协助实现的class”相结合


## 条款41：了解隐式接口和编译器多态
- “运行期多态”与“编译器多态”：“运行期多态”指运行时才决定哪一个virtual函数被执行，“编译器多态”包括重载函数与templates模板
- 模板对于templates参数而言，接口是隐式的，奠基与有效表达式。例如以下就隐式规定了w必须包含size()方法与!=运算符，否则将在编译期报错。
```cpp
template<typename T>
void doProcessing(T &w){
    if(w.size() > 10 && w != someNastyWidget){
        ...
    }
}
```

## 条款42：了解typename的双重意义
#### 声明template参数，前缀关键字class和typename可以互换
`template<typename T>class Widget;`

#### 使用typename关键字来标识嵌套从属类型名称
- 嵌套从属类型名称：几个概念，从属名称（template出现名称相依于某个template参数），嵌套从属名称（从属名称在class内呈嵌套状），非从属名称；
- 解析困难的来源：C::const_iterator，一定是一个类型名称吗？也许是一个静态成员变量的名字，因此要显式地标注为`typename C::const_iterator iter;`
- 例外：基类列中不能加、成员初始列不能加；

## 条款43：学习处理模板化基类内的名称
- 假设在`template<typename Company>class MsgSender`中有一个方法为sendClear()，可在派生类模板内通过"this->"指涉基类模板内的成员名称，因为当基类从模板类中被具体化时，对于那些基类的内容是毫无所悉的。

```cpp
template<typename Company>
class LoggingMsgSender:: public MsgSender<Company> {
public:
    void sendClearMsg(const MsgInfo &info){
        this->sendClear(info);
    }
};
```

或者：`using MsgSender<Company>::sendClear;`或者`MsgSender<Company>::sendClear(info);`每种方法的目的是相同的，为了承诺编译器基类模板的任何特化版本都将支持其一般版本所提供的接口

- 反例：特化版本可以不提供一般版本接口：这种写法指的是模板全特化，即一旦类型参数被指定成CompanyZ，该对象就被特化成下面这个形式
```cpp
template<>
class MsgSender<CompanyZ> {
public:
    .. //不含sendClear()方法
};
```

## 条款44：将与参数无关的代码抽离templates
- 代码膨胀：编写templates时，代码的重复是隐晦的，只有被具现化时才会体现出重复性，膨胀示例：
```cpp
template<typename T, std::size_t n>
class SquareMatrix{
public:
    void invert();      //矩阵求逆
};
```

- 优化：接口和上面一致，但是所有矩阵共享同一份SquareMatrixBase
```cpp
template<typename T>
class SquareMatrixBase{
protected:
    void invert(std::size_t matrixSize);
};

template<typename T, std::size_t n>
class SquareMatrix: private SquareMatrixBase{
private:
    using SquareMatrixBase::invert;
    
public:
    void invert(){ this->invert(n); }
};
```

## 条款45：运用成员函数模板接收所有兼容类型
#### 使用成员函数模板生成“可以接受所有兼容类型”的函数
- 例如想要完成在自定义的智能指针完成类似普通指针的隐式转换，称之为==泛化的copy构造函数==
```cpp
template<typename T>
class SmartPtr{
public:
    template<typename U>
    SmartPtr(const SmartPtr<U> &other);     //为了生成copy构造函数
};
```

- 另一种角色：支持赋值操作，例如想要将shared_ptr兼容内置指针、auto_ptr等
```cpp
template<typename T>
class shared_ptr{
public:
    template<class Y>
    explicit shared_ptr(Y *p);  //规定explicit是为了使内置指针隐式向shared_ptr转换不被允许
    
    template<class Y>
    shared_ptr & operator=(shared_ptr<Y> const &r);
    template<class Y>
    shared_ptr & operator=(auto_ptr<Y> const &r);
    //兼容来自shared_ptr和auto_ptr的赋值操作
};
```

- 有一个问题：一旦T和Y的类型相同，泛化的copy构造函数会被具现化为“正常的”copy构造函数，如果想要控制copy构造函数的方方面面，需要同时声明泛化copy构造函数模板和普通copy构造函数。


## 条款46：需要类型转换时请为模板定义非成员函数
考虑将条款24中的Rational乘法扩展为模板类：
```cpp
template<typename T>
const Rational<T operator* (const Rational<T &lhs, const Rational<T> &rhs);

Rational<int> oneHalf(1,2);
Rational<int> result = oneHalf * 2;
```
但是以上代码不能通过编译，原因在于非template时会将int隐式转换为Rational，但template实参推导过程中不会将隐式转换函数考虑在内。==于是出现了以下逻辑==：为了让类型转换发生在所有实参身上，需要一个non-member函数；为了让这个函数被自动具现化，需要将它声明在类内；于是用什么方法能够在类内声明一个non-member函数呢？==答案是friend.==
```cpp
template<typename T>
class Rational{
public:
    friend const Rational operator* (const Rational &lhs, const Rational &rhs)
    {
        return Rational(lhs.numberator() * rhs.numerator(),
                        lhs.denominator() * ths.denominator());    
    }
};
```

由于这种方式在类内的定义式会被默认为inline，好在此处函数体较为简短，遇到较长的函数体可以考虑定义一个helper，只在friend中调用这个helper


## 条款47：请使用traits classes表现类型信息
#### 什么是traits
Traits并不是C ++关键字或者一个预先定义好的组件，它是一种技术，也是一个C ++程序员共同遵守的协议。要求之一是：==对内置类型和用户自定义类型的表现必须一样好==。

#### 考虑实现一个STL中的advance
```cpp
template<typename IterT, typename DistT>
void advance(IterT &iter, DistT d);
```
- 需要实现的功能：将迭代器iter移动d个距离，这看似简单的功能却暗藏玄机，首先只有随机访问才能支持+=操作（vector/deque/string），其余都只能进行++/--操作；此外forward迭代器是不支持向后移动，但Bidirectional迭代器支持双向移动，d可正可负；最后需要提供对普通内置指针的支持。
- 先给每个数据结构的迭代器打上卷标，表明所使用的迭代器类型
```cpp
template<...>
class deque{
public:
    class iterator {
    public:
        typedef random_access_iterator_tag iterator_category;
    };
};

template<...>
class list{
public:
    class iterator{
    public:
        typedef bidirectional_iterator_tag iterator_category;
    };
};
```

iterator_traits实现
```cpp
template<typename IterT>
struct iterator_traits{
    typedef typename IterT::iterator_category iterator_category;
};
```

针对内置指针的偏特化
```cpp
template<typename IterT>
struct iterator_traits<IterT *>{
    typedef random_access_iterator_tag iterator_category;
};
```

函数重载实现不同类型的迭代器版本的advance
```cpp
template<typename IterT, typename DistT>
void doAdvance(IterT &iter, DistT d, std::random_access_iterator_tag)
{
    ider += d;
};

template<typename IterT, typename DistT>
void doAdvance(IterT &iter, DistT d, std::bidirectional_iterator_tag)
{
    if(d >= 0){ while(d--) ++iter; }
    else { while(d++) --iter; }
};

//用抛出异常的方式禁止input_iterator_tag型迭代器向后移动
template<typename IterT, typename DistT>
void doAdvance(IterT &iter, DistT d, std::input_iterator_tag)
{
    if(d < 0){
        throw std::out_of_range("Negative distance");
    }
    while(d--) ++iter;
};
```

最终的advance实现
```cpp
template<typename IterT, typename DistT>
void advance(IterT &iter, DistT d){
    doAdvance(iter, d
        typename std::iterator_traits<IterT>::iterator_category());
}
```

## 条款48：认识template元编程
- template元编程可将工作从运行期移往编译器，从而实现早期错误侦测和更高的执行效率
- 条款47中的traits解法就是TMP的代表。（对应使用typeid()进行运行期的类型检查效率很低）


## 条款49：了解new-handler的行为


## 条款50：了解new和delete的合适替换时机
- **检测运用错误**：delete失败导致内存泄露；多次delete导致不确定行为
- **为了手机动态分配内存的统计信息**
- **增加分配和归还的速度**：Class专属分配器是区块尺寸固定的分配器实例，例如Boost的Pool程序库。
- **降低缺省内存管理器带来的额外空间开销**
- **弥补某些体系结构中的非最佳齐位**：例如x86中访问doubles是最快速的
- **为了将相关对象集中成簇**：placement见条款52


## 条款51：编写new和delete时需固守常规

## 条款52：写了placement new也要写placement delete

## 条款53：不要轻忽编译器的警告
不能轻易忽视也不能过度依赖编译器的警告信息。
```cpp
class B{
public:
    virtual void f() const;
};

class D: public B{
public:
    virtual void f();
};
```

## 条款54：让自己熟悉包括TR1在内的标准程序库
TR1只是一份规范，而Boost是一个好的实物来源，==其实很多组件已经加入了C++新标准==
#### C++98的标准程序库有哪些
- STL
- iostream
- 国际化支持wchar、wstring
- 数值处理complex、valarray
- 异常阶层体系
- C89

#### TR1新组建
- 智能指针：tr1::shared_ptr和tr1::weak_ptr，已加入C++11
- tr1::function，更富弹性地接受任何可调用物，已加入C++11，用法见条款35
- tr1::bind，已加入C++11
- 哈希表：tr1::unordered_map已加入C++11
- 正则表达式
- tr1::tuple：pair的扩展，可以持有任意个数的对象
- tr1::array：固定大小的数组
- 元模板编程，Type Traits支持
- tr1::result_of，推导函数调用的返回类型


## 条款55：让自己熟悉Boost
Boost是一个C++开发者社群，[http://boost.org](http://boost.org)
- 字符串与文本处理、容器、函数对象和高级编程、泛型编程、模板元编程、数学与数值、正确性与测试、数据结构、语言间的支持、内存、杂项

