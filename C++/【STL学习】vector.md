## vector特点
- 可以看作C++数组的升级版，不同之处在于array实现的静态数组，而vector是动态数组，可以进行元素的插入和删除，同时动态调整所占用的内存空间；
- vector擅长在尾部插入或删除元素，其时间复杂度为O(1)；而在中部或者头部进行插入或者删除时，其时间复杂度将达到O(n)；
- 需要包含<vector>头文件，并在std命名空间下；

## vector的创建
#### 创建一个空的vector

```cpp
std::vector<double> values; //创建一个空的vector容器
values.reserve(20);         //为该容器预留20个元素的空间
```

#### 使用{}初始化vector

```cpp
std::vector<int> primes {2, 3, 5, 7, 11, 13, 17, 19};
```

##### 指定元素个数和默认值

```cpp
std::vector<double> values(20); //创建20个元素的vector，默认值为0.0
std::vector<double> values(20, 1.0); //默认值为1.0
```
这里值得一提的是，元素数量和默认值的指定不仅可以是常量，还可以是变量；

#### 从另一个vector复制构造得来

```cpp
std::vector<char>value1(5, 'c');
std::vector<char>value2(value1);
```

#### 可以使用一对指针或迭代器指定初始值范围

```cpp
int array[]={1,2,3};
std::vector<int>values(array, array+2);//values 将保存{1,2}
std::vector<int>value1{1,2,3,4,5};
std::vector<int>value2(std::begin(value1),std::begin(value1)+3);//value2保存{1,2,3}
```

## 成员函数

函数成员 | 函数功能
---|---
begin() | 返回指向容器中第一个元素的迭代器。
end() | 返回指向容器最后一个元素所在位置后一个位置的迭代器，通常和 begin() 结合使用。
rbegin() | 返回指向最后一个元素的迭代器。
rend() | 返回指向第一个元素所在位置前一个位置的迭代器。
cbegin() | 和 begin() 功能相同，只不过在其基础上，增加了 const 属性，不能用于修改元素。
cend() | 和 end() 功能相同，只不过在其基础上，增加了 const 属性，不能用于修改元素。
crbegin() | 和 rbegin() 功能相同，只不过在其基础上，增加了 const 属性，不能用于修改元素。
crend() | 和 rend() 功能相同，只不过在其基础上，增加了 const 属性，不能用于修改元素。
size() | 返回实际元素个数。
max_size() | 返回元素个数的最大值。这通常是一个很大的值，一般是 232-1，所以我们很少会用到这个函数。
resize() | 改变实际元素的个数。
capacity() | 返回当前容量。
empty() | 判断容器中是否有元素，若无元素，则返回 true；反之，返回 false。
reserve() | 增加容器的容量。
shrink _to_fit() | 将内存减少到等于当前元素实际所使用的大小。
operator[ ] | 重载了 [ ] 运算符，可以向访问数组中元素那样，通过下标即可访问甚至修改 vector 容器中的元素。
at() | 使用经过边界检查的索引访问元素。
front() | 返回第一个元素的引用。
back() | 返回最后一个元素的引用。
data() | 返回指向容器中第一个元素的指针。
assign() | 用新元素替换原有内容。
push_back() | 在序列的尾部添加一个元素。
pop_back() | 移出序列尾部的元素。
insert() | 在指定的位置插入一个或多个元素。
erase() | 移出一个元素或一段元素。
clear() | 移出所有的元素，容器大小变为 0。
swap() | 交换两个容器的所有元素。
emplace() | 在指定的位置直接生成一个元素。
emplace_back() | 在序列尾部生成一个元素。

## 注意几点：
- capacity()和size()的区别？

size()返回vector中实际包含的元素个数，而capacity()返回vector的容量，size()是不超过capacity()的

- 当需要增加一个元素时，往往==不仅仅分配一个元素的内存，而是额外分配更多的内存==；因此每次增加新的元素时，有时是可以瞬间完成的，有时则涉及到申请新的内存空间；
- 一旦vector的内存被重新分配，==原先的应用、指针、迭代器都可能失效，需要重新生成==。
- push_back()和emplace_back()有何区别？

这两个函数均是在vector后插入一个新元素，且调用语法一致；区别在于，push_back()是先构造好新节点，再移动或拷贝到vector尾部（会调用复制/移动构造函数）；而emplace_back()则是直接在尾部创建这个元素，更为高效；

总结：==emplace_back()执行效率更高，推荐优先使用；而emplace_back()是C++11标准新增的，需要兼顾之前版本时，还是应该使用push_back().==

## 插入元素操作
除了上述在尾部插入（时间复杂度为O(1)）的操作，还有在任意位置插入单个或者多个元素的操作（时间复杂度为O(n)）
#### insert()函数
四种调用方式：

```cpp
iterator insert(pos,elem)       //在pos之前插入元素elem
iterator insert(pos,n,elem)     //在pos之前插入n个元素elem
iterator insert(pos,first,last) //在pos之前插入迭代器first和last之间的元素[first, last)
iterator insert(pos,initlist)   //在pos之前插入初始化列表{}中的所有元素
```
均返回第一个新插入元素位置的迭代器

#### emplace()
与emplace_back()类似，也是C++11标准新增函数，执行更为高效，但每次只插入一个元素；其中参数arg可以理解为被插入元素的构造函数需要的参数；因此==更推荐使用emplace()函数==

## 删除元素操作
vector有着多种删除元素的操作方式，需要根据不同的具体情况灵活选择
#### pop_back()
删除vector的最后一个元素，size减小而capacity不变

#### erase(pos)
删除迭代器pos指向的元素，并返回下一个元素的迭代器，size减小而capacity不变

#### erase(first, last)
删除[first, last)中的元素，其余同上

erase()函数的实现原理：删除指定后会将后面所有元素向前移，并减小size，因此这其实可能是一个很耗时的操作；当不需要考虑元素顺序时，可以考虑使用swap()与pop_back()相结合的手段。

####  remove(begin, end, elem)
删除容器中所有指定的元素，并返回下一个元素的迭代器，==不会改变size和capacity==

remove()的实现原理：遍历vector所有元素，将目标元素处用非目标元素向前补充，例如[1,3,3,4,3,5]在remove 3操作后，得到的将是[1,4,5,4,3,5]，这就是size不会改变的原因。因此==remove的后续操作（例如遍历）很依赖与其返回值==；另外多余的元素可以使用erase()一次性删除。

举个例子：

```cpp
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;
int main()
{
    vector<int>demo{ 1,3,3,4,3,5 };
    //交换要删除元素和最后一个元素的位置
    auto iter = std::remove(demo.begin(), demo.end(), 3);
    demo.erase(iter, demo.end());
    cout << "size is :" << demo.size() << endl;
    cout << "capacity is :" << demo.capacity() << endl;
    //输出剩余的元素
    for (int i = 0; i < demo.size();i++) {
        cout << demo[i] << " ";
    }
    return 0;
}
```

#### clear()
删除所有元素

## 另外的一些常用操作
#### vector内元素的排序
采用sort()函数，需要#include <algorithm>

默认的排序方式是从小到大，需要定义<运算符

```cpp
sort(vt.begin(), vt.end());
```

重写比较函数实现自定义排序规则：

```cpp

#include<stdio.h>
#include<vector>
#include<algorithm>
using namespace std;
bool cmp(int x,int y) ///cmp函数传参的类型不是vector<int>型，是vector中元素类型,即int型
{
    return x>y;
}
int main()
{
    int c1[]={1,3,11,2,66,22,-10};
    vector<int> c(c1,c1+7);
    sort(c.begin(),c.end(),cmp);
    for(int i=0;i<c.size();i++){
        printf("%d ",c[i]);
    }
    printf("\n");
    return 0;
}
```

#### 翻转元素

```cpp
#include <algorithm>
reverse(vt.begin(), vt.end());
```

#### 查找某个元素

```cpp
#include <algorithm>
vector<int>::iterator it = find(vt.begin(), vt.end(), 3); //返回元素迭代器

if(it != vt.end())  // 判断是否查到

int index=&*it-&temp[0];///放入迭代器中得到容器中的位置
```


