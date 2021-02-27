## 快速上手
#### gets()函数与其代替

- 使用示例
```c
#define MAX_INPUT 1000

char input[MAX_INPUT];
while(gets(input) != NULL){
    //TODO:
}
```

- 功能：从标准输入stdin读取一行文本（直到'\n'之前），返回非NULL值；若到达输入末尾/文件尾，返回NULL

- 存在问题：不检查边界，易导致缓冲区溢出，造成覆盖其他数据或程序漏洞的问题

- 代替：fgets()，采用第二个参数限制其输入数量，使用示例：

```c
#include <stdio.h>   
#include <string.h>
#define MAX_INPUT   10

int main(void)
{
    char str1[MAX_INPUT];  
    fgets(str1, MAX_INPUT, stdin);
    printf("%s\n", str1);
    printf("%d\n", strlen(str1));
    return 0;
}
```

- fgets()使用注意
1. 输入参数为N，则将读入N-1个字符，加上一个'\0'
2. 或者遇到第一个换行符为止，==并将换行符存储在字符串中==（注意与gets()的区别）


#### 一些scanf()的细节
- 注意：输入值前的空白（空格，制表符，换行符等）将被跳过，遇到下一个空白符表示停止
- 如何输入一个包含空格的字符串？
1. 用fgets()
2. getchar()函数循环读取

```c
char input2[MAX_INPUT];
int ch, i = 0;
ch = getchar();
while((ch = getchar()) != '\n' && ch != EOF){
    input2[i++] = ch;
} 
```
这里有个小问题：为何ch要被声明成int型？EOF是int的

3. scanf("%[^\n]",str);

```c
char input3[MAX_INPUT];
scanf("%[^\n]", input3);
```
其中，%[^\n]表示接受到除'\n'为止，而[a-z]表示接受只接受小写字母

#### 一些字符串操作函数<string.h>
- size_t strlen(const char *str)
计算字符串的长度，==注意与sizeof()的区别！==，strlen()不包含'\0'

- char *strcpy(char *dest, const char *src)
- char *strncpy(char *dest, const char *src, size_t n)
字符串的复制，返回值也是指向dest

- char *strcat(char *dest, const char *src)
- char *strncat(char *dest, const char *src, size_t n)
字符串拼接，返回值也是指向dest

- char *strstr(const char *haystack, const char *needle)
- char *strchr(const char *str, int c)
返回第一次出现某字符串/字符的位置，==类型为指针==

- char *strtok(char *str, const char *delim)
字符串分割，注意重复调用获得每一段字符串

```c
#include <string.h>
#include <stdio.h>
 
int main () {
   char str[80] = "This is - www.runoob.com - website";
   const char s[2] = "-";
   char *token;
   
   /* 获取第一个子字符串 */
   token = strtok(str, s);
   
   /* 继续获取其他的子字符串 */
   while( token != NULL ) {
      printf( "%s\n", token );
    
      token = strtok(NULL, s);
   }
   
   return(0);
}
```


## 基本概念


## 数据
#### short、int、long的长度
- 标准规定：长整型至少要和整型一样长，而整型至少应该和短整型一样长，编译器可以根据自身硬件来选择合适的大小

- 主流的编译器中，32位机器与64位机器各类型所占字节数如下表所示

  C类型 | 32 | 64
  --- | --- | ---
  char | 1 | 1
  short int | 2 | 2
  int	| 4	| 4
  long int | 4 | 8
  long long int | 8 | 8
  char* | 4 | 8
  float | 4 | 4
  double | 8 | 8

- 对于特定平台的整数类型取值，头文件<limits.h>包含了最小值与最大值的数值，如SHRT_MIN/LONG_MAX/UINT_MAX

#### 区分：常量指针和指针常量
- 指针常量——指针类型的常量(int *const p;)

指针自身是一个常量，不可修改其值，始终指向同一个地址，但指向地址的值可以修改

- 常量指针——指向常量的指针(int const *p/const int *p)

指针指向的内容不可改变，但可以修改指向的地址

- 区分方法：
1. (int *const p)中const修饰p，因此p是一个常量，而p的类型是指向int的指针，故p是指针常量

2. (int const *p/const int *p)中const修饰 *p，因此 *p是一个常量，p是指向该常量的指针，故p是常量指针

#### 区分：数组指针和指针数组
- 数组指针——int (*p)[];

本质是一个指针，指针指向一个数组

- 指针数组——int *p[];

本质是一个数组，数组中的元素是指向int的指针

- 区分方法：明确优先级() > [] > *
1. int (*p)[]，先看()内， *p表示p是一个指针，指向int []数组
2. int *p[]，根据结合性 p[]为整体表示p是一个数组，数组中的每个元素为指向int的指针

#### extern和static关键字用法整理
- static的使用
1. 修饰局部变量

普通局部变量存储在栈中，随着语句块的生存周期结束而释放；而static修饰的局部变量存储在静态数据区，生存周期为整个程序，只在初次运行时进行初始化。

2. 修饰全局变量

标识该全局变量仅在本源文件内有效，位于不同源文件中的static变量对应与不同的实体；特别地，当static写在.h文件中并被多个.cpp文件包含时，每个变量对应不同实体，因此为了避免错误，一般将static声明在.cpp文件内

3. 修饰函数

标识该函数仅在本源文件内有效，与修饰全局变量一样

4. C++中的static

    1. 对类中的成员函数进行修饰，表明该函数属于一个类而不属于此类的任何实例化对象；只能访问静态成员函数和静态成员变量，而不能访问非静态函数或变量（这也很好理解）
    2. 对类中的成员变量进行修饰，表明该变量为类及其所有对象所拥有；在存储空间中只有一个副本，可以供类或对象去调用
    
- extern的使用
1. 标识变量或者函数的定义在别的文件中，提示编译器遇到此变量或者函数在其他模块中寻找

这里需要注意的是：extern的变量要严格对应声明形式！举例：

```
示例：
A.cpp定义：char a[20] = "Hello, world!";
B.cpp声明：extern char *a;
编译通过，运行时出错：非法访问
因为B中的a值为0x6C6C6548("lleH"的ASCII码值，为什么呢？)，显然*a会造成非法访问

修改：
修改A.cpp：char *a = "Hello, world!";
或 修改B.cpp：extern char a[];
```

同样地，当extern修饰函数时，若提供方单方面修改了函数原型，而使用方仍然沿用原来的extern声明，编译也将通过，但运行时往往会造成系统错误；解决办法：通常提供方将对外接口的声明放在.h中，供使用方include，省去了extern

2. C++中的extern

```
用法：
extern "C"{
    ...
}
作用：
指示编译器将这部分代码按C语言进行编译
```

C++为了解决多态的问题，会将函数名和参数联合起来命名，因此当C++环境中调用C函数时常常会出现找不到定义的错误，这时需要将C语言函数用extern "C"进行指定


## 语句

## 操作符和表达式
#### 取模%的符号讨论
结论：运算结果由被除数符号决定
```c
#include <stdio.h>
int main(void)
{
    printf("%d\n", 8%5);
    printf("%d\n", 8%-5);
    printf("%d\n", -8%5);
    printf(" %d\n", -8%-5);
    return 0;
}

输出：
3
3
-3
-3
```

#### 移位运算符的细节
- 逻辑右移（填0）与算数右移（填符号）
C语言中没有对两者进行区分，若对一个负数进行右移操作，其结果由编译器所决定

- 移位一个负值或一个比操作数位数还要多的位数
未定义的，不可预测的

#### sizeof分析
- sizeof(类型)返回该类型所占的字节数
- sizeof(变量)返回该变量所占的字节数
1. 当输入数组名时，返回该数组的长度（字符串）
2. 当输入一个表达式时，该表达式不会进行求值，举例：sizeof(a = b+1)
- 结构体与联合的sizeof

#### + / += / ++ 运算符分析
- +=的特点：左操作数只求值一次

应用
```
a[2 * (y - 6*f(x))] = a[2 * (y - 6*f(x))] + 1;
a[2 * (y - 6*f(x))] += 1;
```
由于编译器无从知道f(x)函数是否会产生副作用，因此第一句话会进行两次求值；而第二句话的效率更高

- 前缀与后缀++：均会复制一份变量值的副本

类似于下面这种写法是未定义的：
```
c + --c
```
不同编译器的值可能不同，因此不应该这么写，也不需要纠结这类表达式的结果（大一C++老师的典型误区）


## 指针
#### 指针常量
- 除了NULL指针，C语言没有其他内建指针常量
- 极少使用指针常量：因为很少情况会知道编译器把变量放在什么地方；例外，访问硬件，访问内存中某个特定的位置，操作系统与输入输出设备控制器通信
- 操作100地址处的内存：强制类型转换
```
*(int *)100 = 25;
```

#### 指针运算
- 指针 ± 整数 = 指针
- 指针 - 指针 = 整数（指向同一个数组中的元素）
- 关系运算 >= / <= / < / >（同一数组）

注意：
1. 指针指向数组最后一个元素的后一个元素是合法的，也允许与指向数组中的指针进行比较，但间接访问可能会失败
2. 指针指向数组第一个元素的前一个元素是非法的，不能进行比较，也不能间接访问

所以下面两种写法：
```c
for(vp = &values[N_VALUES]; vp > &values[0]; ){
    *--vp = 0;
}

for(vp = &values[N_VALUES-1]; vp >= &values[0]; vp--){
    *vp = 0;    //有可能有问题，原因在于最后一个循环后
}
```


## 函数
#### ADT与黑盒
- 通过限制函数和数据定义的作用域，设计和实现抽象数据类型ADT，使实现细节和外界隔绝
- 限制对模块的访问，通过static关键字，限制对非接口的函数和数据的访问

#### 可变参数列表
- 可变参数列表通过宏来实现，宏定义于<stdarg.h>中，通过类型va_list，和宏定义va_start, va_arg, va_end来实现
- 示例
```c
#include <stdarg.h>

float average(int n_values, ...){
    va_list var_arg;
    float sum = 0;

    va_start(var_arg, n_values);

    for(int i = 0; i < n_values; i++){
        sum += va_arg(var_arg, int);
    }

    va_end(var_arg);
    return sum / n_values;
}
```

- 一些限制
1. 可变参数必须从头到尾按照顺序逐个访问，中途开始访问不可
2. 无法判断实际存在的参数的数量
3. 无法判断每个参数的类型

解决：使用命名参数
1. 上例中使用n_values指定了参数数量，但他们被假定成int型
2. printf()函数中，格式化字符串不仅指定了参数数量，还指定了每个参数的类型

## 数组
#### 一些零碎
- 数组名是指针常量，例如以下操作是不允许的：
```c
int a[10];
int b[10];
a = b;  //x
a++; //x
```

- 奇怪的下标引用法：始终记住a[b] = *(a + b)即可，因此以下写法也是合法的虽然不应该这么写
```c
ap[-1]; // *(ap - 1)
2[array]; // *(2 + array)，即array[2]
```

- 据书上描述：使用指针访问数组比下标访问更具效率，因为下标访问的过程中涉及到乘法，而指针访问编译时已经完成了乘法运算，运行时只进行固定数字相加；而经过实验，在x86平台下，通过查看指针访问和下标访问的汇编代码，下标访问也没有涉及到乘法。

另：一般来讲，不能为了效率上的细微差别而牺牲可读性

- 数组的自动初始化：自动变量的数组声明时给出初始值，执行时会压入运行时栈，然后隐式地包含很多条赋值语句，效率堪忧；当数组的初始化位于局部代码块中，同时初始化又不是必需时，可以使用static修饰，只需在程序开始前执行一次。

#### 字符数组与字符串常量
- 考虑两种写法：
```c
char message1[] = "Hello";
char *message2 = "Hello";
```
- 第一条是声明一个字符数组，包含 'H','e','l','l','o','\0'共6个元素，值可以修改
- 第二条是声明一个字符串常量，并用一个指针指向，值不可修改，不能使用例如strcpy()等函数

#### 多维数组
- 二维数组的类型分析
```c
int matrix[6][10];

matrix类型：int (*)[10]，数组指针
*matrix类型：int *
**matrix类型：int

声明一个指向matrix第二行的指针：
int (*p)[10] = matrix + 1;
```

- 作为函数参数

```c
void func2(int (*mat)[10]);
void func2(int mat[][10]);

以下是不正确的（常犯的错误）
void func2(int **mat); //mat的类型是指向int型指针的指针
void func2(int mat[][]); //没有指定指向数组的长度
```

- 两种声明一组字符串：注意在内存中的存储方式区别

```c
char const *keyword[] = {
    "do",
    "for",
    "if",
    "register",
    "return",
    "switch",
    "while",
    NULL
};

char const keyword[][9] = {
    "do",
    "for",
    "if",
    "register",
    "return",
    "switch",
    "while",
}
```
各有优点，一般采用第一种方式

## 字符串、字符和字节
- strlen()函数原型
```c
size_t strlen(char const *string);
```
其中size_t在<stddef.h>中定义，为无符号整型，因此以下两句话实际不同：
```c
if(strlen(x) >= strlen(y))

if(strlen(x) - strlen(y) >= 0) //无符号减无符号不会减出负数，永远为真
```

- 复制字符串时，若dst和src有重叠，结果是未定义的，非要使用的话，用memmove()，中间涉及拷贝到临时位置

- 直接测试和操控字符将会降低程序的可移植性，例如测试字符ch是否为大写：
```c
if(ch >= 'A' && ch <= 'Z') //仅针对ASCII字符机器

if(isupper(ch)) //定义与<ctype.h>，更为通用
```

- 查找一个字符串的前缀：strspn(), strcspn()

返回str起始部分匹配group中任意字符的字符数

- 查找任何几个字符：strpbrk()



## 结构和联合
#### 结构的存储分配
- 结构体的默认存储方式采用以==最大字节元素字节数==进行对齐，例如含有double或者long long的按8字节对齐等

```
示例1：
struct ALIGN{   //4字节对齐
    char a;     //offset:0
    short b;    //offset:2
    int c;      //offset:4
};              //总字节数:8

示例2：
struct ALIGN{   //8字节对齐
    int a;      //offset:0
    char b;     //offset:4
    double c;   //offset:8
    char d[5];  //offset:16
    int e;      //offset:24
};              //总字节数：32

示例3：
struct ALIGN{   //2字节对齐
    char a;     //offset:0
    short b;    //offset:2
    char c;     //offset:4
}               //总字节数：6
```

- 确定结构中某个成员的具体位置，使用offsetof宏，位于<stddef.h>

```
offsetof(struct ALIGN, b);
```

#### 结构的自引用和相互依赖
- 自引用：必须包含自身的指针（应用：链表、树等），而不能包含自身，否则将造成无穷递归

- 相互依赖：基于同样的原因，至少要有一个结构必须在另一个结构内部以指针的方式存在

```c
struct B;

struct A{
    struct B *partner;
};

struct B{
    struct A *partner;
};
```
此处声明时用到了“不完整声明” struct B;

#### 结构体作为函数参数传入
- 当结构体较大时，传递结构体本身将产生巨大开销，涉及到拷贝、入栈、出栈，因此考虑采用指针传入；若实在不想修改其中的值，可以采用常量指针

```c
void print_receipt(Transactions const *trans);
```

#### 位段
- 不同长度的字段实际上存储在一个或者多个整型变量中，应用场景：操作设备的硬件寄存器
```c
struct CHAR{
    unsigned ch     : 7;
    unsigned font   : 6;
    unsigned size   : 19;
};
```

- 一些注意：
1. 优点：简化了代码，压缩了内存
2. 缺点：移植性较弱，例如在16位机上就不能运行上述代码，编译器不允许分配19个比特
3. 任何可以用位段实现的任务均可以使用移位和屏蔽的==位操作==来实现

#### 联合
- 联合所有成员引用内存中的相同位置，在特定场合可以大幅压缩内存大小
- 分配个联合的内存取决于最长成员的长度，若出现比较大的成员，考虑在联合中存储==指针==
- 联合的初始化：一定得是第一个成员的类型，否则将尝试类型转换，举例：
```c
union {
    int a;
    float b;
    char c[4];
}x = {5};
```


## 动态内存分配
#### realloc
- 原型
```c
void *realloc(void *ptr, size_t new_size);
```

- 可以应用于在原地扩展、缩小内存，当原先内存块无法改变大小时，将分配另一块大小正确的内存并把内容复制到新块上。因此使用realloc后不能再使用旧指针，应将返回值作为新指针。
- 若第一个参数为NULL，则行为与malloc一致


## 使用结构和指针


## 高级指针话题
#### 比较高级的指针声明

```c
int *f, g;  //声明了一个指向int型的指针f和int型g
int *f();   //声明了一个返回值为int*的函数f
int (*f)(); //声明了一个指向返回值为int的函数的指针
int *f[];   //指针数组
int (*f)[]; //数组指针
int (*f[])();  //指针数组，每个元素指向返回值为int的函数
```

考虑这样一个：int (*(*x)[10])()

逐层分析：首先x是指针，指向的是一个包含10个元素的一维数组，数组里面的每个元素也是指针，指向的是返回值为int的函数

#### 函数指针
- 创建形式：访问前一定要初始化！

```c
//声明
int f(int);
int (*pf)(int) = &f;

//调用
f(25);
(*pf)(25);
pf(25);
```

- 声明中的&是可选的，调用时的*也是可选的

- 应用1：回调函数

用户把函数指针作为参数传递给其他函数void *，后者回调用户的函数

- 应用2：转移表

初始化一个函数指针数组，特别要注意下标的越界问题

#### 示例：将strcmp函数作为参数传入qsort()中实现字符串快速排序
- 不能直接传入，因为qsort()中的类型时(const void *, const void *)，而strcmp()中是(const char *, const char *)，类型不匹配
- 外面封装一层函数
```c
int compare(const void *a, const void *b){
    return strcmp(*(char **)a, *(char **)b);
}
```

- 为什么要写成*(char **)而不是(char *)?

因为字符串的类型是(const char *)，因此要先强制转换成指向(const char *)的指针；

## 预处理器
#### 预定义符号

符号 | 类型 | 含义
---|--- | ---
__FILE__ | %s | 进行编译的源文件名车
__LINE__ | %d | 当前行号
__DATE__ | %s | 编译日期
__TIME__ | %s | 编译时间

#### 宏定义
- 易出错的地方，例如：

```c
#define SQUARE(x)   x * x
SQUARE(a + 1)

#define DOUBLE(x)   x + x
10 * DOUBLE(a)
```

- #argument，把一个宏转换为一个字符串
```c
#define PRINT(FORMAT,VALUE) \
        printf("The value of " #VALUE \
        " is " FORMAT "\n", VALUE)
        
PRINT("%d", x + 3);
//The value of x + 3 is 25
```

- ##结构连接两边的符号

```c
#define ADD_TO_SUM(sum_number,value) \
        sum ## sum_number += value
        
ADD_TO_SUM(5, 25);
//sum5 += 25;
```

- 宏与函数
1. 宏比使用函数在程序规模与速度方面都更胜一筹
2. 宏与类型无关
3. 宏可能会大幅增加程序的长度
4. 宏实现函数无法实现的任务，例如参数名称作为参数传入


## 输入/输出函数
- 错误报告：perror(const char *message)

打印字符串message + ": " + 打印用于解释errno当前错误代码的信息

- 终止执行：void exit(int status)

预定义的符号有 EXIT_SUCCESS 和 EXIT_FAILUER

- 打开流：FILE *fopen(const char *name, const char *mode)

模式 | 描述
---|---
"r" | 打开一个用于读取的文件。该文件必须存在
"w" | 创建一个用于写入的空文件。如果文件名称与已存在的文件相同，则会删除已有文件的内容，文件被视为一个新的空文件
"a" | 追加到一个文件。写操作向文件末尾追加数据。如果文件不存在，则创建文件
"r+" | 打开一个用于更新的文件，可读取也可写入。该文件必须存在
"w+" | 创建一个用于读写的空文件
"a+" | 打开一个用于读取和追加的文件

打开需要进行检查，若打开失败会返回NULL，并在errno中指示错误

- 字符I/O
1. int fgetc(FILE *stream);
2. int getc(FILE *stream);
3. int getchar(void); 时钟从标准输入读取
4. int fputc(int ch, FILE *stream);
5. int putc(int ch, FILE *stream);
6. int putchar(int ch);

- 未格式化的行I/O
```c
char *fgets(char *buffer, int size, FILE *stream);

int fputs(const char *buffer, FILE *stream);
```

- 格式化的行I/O
1. scanf家族: scanf(), fscanf(), sscanf()
2. printf家族：printf(), fprintf(), sprintf()

其中sprintf()是一个潜在的错误根源，因为无法确定缓冲区需要多大。

- 二进制I/O
```c
size_t fread(void *buffer, size_t size, size_t count, FILE *stream);

size_t fwrite(void *buffer, size_t size, size_t count, FILE *stream);
```

- 刷新和定位函数
1. 迫使输出流的缓冲区内的数据进行物理写入：int fflush(FILE *stream)
2. 定位函数
```c
long ftell(FILE *stream);
int fseek(FILE *stream, long offset, int from);
```

from可取：SEEK_SET起始，SEEK_CUR当前，SEEK_END尾部


```c
void rewind(FILE *stream);  //回滚到起始位置
int fgetpos(FILE *stream, fpos_t *position);
int fsetpos(FILE *stream, fpos_t const *position);
```

- 流错误函数
```c
int feof(FILE *stream); //判断是否位于文件尾
int ferror(FILE *stream);
void clearerr(FILE *stream);
```


## 标准函数库
- 整数除法
```c
div_t div(int number, int deominator);
```
返回值是一个div_t型的结构体，包含quot——商，和rem——余数。但经过测试，效率反而没有先使用/再使用%高，推测可能原因在与函数调用、压栈弹栈

- 字符串转整型变量
```c
int atoi(char const *string);
long int atol(char const *string);
long int strtol(char const *string, char **unused, int base);
unsigned long int strtoul(...);
```
效果：跳过前导空白字符，转换合法字符，忽略任何非法缀尾字符

- 字符串转浮点型
```c
double atof(char const *string);
double strtod(char const *string, char **unused);
```

使用示例：
```c
#include <stdio.h>
#include <stdlib.h>

int main(){
    const char *string = "1234A5.52";
    char *unused1, *unused2;
    printf("%d, %f, %d, %f\n",  
            atoi(string), 
            atof(string), 
            strtol(string, &unused1, 12), 
            strtod(string, &unused2)
    );
    printf("%c, %c\n", *unused1, *unused2);
    return 0;
}
```

- 非本地跳转
```c
#include <setjmp.h>
int setjmp(jmp_buf state);
void longjmp(jmp_buf state, int value);
```
1. setjmp()函数的初始化返回0，并在jmp_buf中保存当前的状态值，longjmp()的效果是使得执行再次从setjmp返回，此时的返回值是longjmp()中的参数
2. 使用场景：如果存在一长串的调用链，即使只有最深层的那个函数发现了错误，调用链中的所有函数都必须返回并检查错误代码，可以使用非本地跳转


- 信号

信号 | 含义
---|---
SIGABRT | 程序请求异常终止
SIGFPE | 发生一个算术错误
SIGILL | 检测到非法指令
SIGSEGV | 检测到对内存的非法访问
SIGINT | 收到一个交互性注意信号
SIGTERM | 收到一个终止程序的请求

```c
显式引发一个信号：
int raise(int sig); 

注册信号处理函数：
void (*signal(int sig, void (*handler)(int)))(int);
```
如何理解这个signal()的函数原型？

首先这是一个函数，返回一个返回值为void，参数为int的函数指针，事实上这个返回值是先前的处理函数指针；第一个参数是注册的信号值，第二个参数是信号处理函数的函数指针

示例：
```c
#include <stdio.h>
#include <signal.h>

void sig_handler(int sig){
    printf("signal %d catched!\n", sig);
    signal(sig, fpe_handler);
}

int main(){
    signal(SIGINT, sig_handler);
    while(1){;}
    printf("exit!\n");
    return 0;
}

```

同时有两个宏定义可以直接作为第二个参数传入：
SIG_DFL——恢复默认，SIG_IGN——忽略

- 排序和查找
```c
void qsort(void *base, size_t n_element, 
        size_t el_size, 
        int (*compare)(void const *, void const *));
        
void *bsearch(void const *key, void const *base,
        size_t n_element, size_t el_size, 
        int (*compare)(void const *, void const *));
```
若查找的值不存在，则返回NULL


## 经典抽象数据类型
####一个改进的栈实现
- 目标：可以拥有多个堆栈、拥有多种类型、命名不冲突
- 泛型数组栈

```c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define GENERIC_STACK(STACK_TYPE,SUFFIX,STACK_SIZE)         \
                                                            \
        static STACK_TYPE stack##SUFFIX[STACK_SIZE];        \
        static int top_element##SUFFIX = -1;                \
                                                            \
        int is_empty##SUFFIX(void){                         \
            return top_element##SUFFIX == -1;               \
        }                                                   \
                                                            \
        int is_full##SUFFIX(void){                          \
            return top_element##SUFFIX == STACK_SIZE - 1;   \
        }                                                   \
                                                            \
        void push##SUFFIX(STACK_TYPE value){                \
            assert(!is_full##SUFFIX());                     \
            top_element##SUFFIX += 1;                       \
            stack##SUFFIX[top_element##SUFFIX] = value;     \
        }                                                   \
                                                            \
        void pop##SUFFIX(void){                             \
            assert(!is_empty##SUFFIX());                    \
            top_element##SUFFIX -= 1;                       \
        }                                                   \
                                                            \
        STACK_TYPE top##SUFFIX(void){                       \
            assert(!is_empty##SUFFIX());                    \
            return stack##SUFFIX[top_element##SUFFIX];      \
        }

GENERIC_STACK(int, _int, 10);
GENERIC_STACK(float, _float, 3);

int main(){
    while(!is_full_float()){
        float input;
        scanf("%f", &input);
        push_float(input);
    }

    while(!is_empty_float()){
        printf("Popping %f\n", top_float());
        pop_float();
    }

    while(!is_full_int()){
        int input;
        scanf("%d", &input);
        push_int(input);
    }
        
    while(!is_empty_int()){
        printf("Popping %d\n", top_int());
        pop_int();
    }

    return 0;
}
```


## 运行时环境






