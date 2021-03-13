## priority_queue特点
队列和排序的完美结合，内部是由堆实现的，删除和插入的时间复杂度均为O(logN)

## 定义
> priority_queue<Type, Container, Functional>

举例：
- 升序队列：priority_queue <int,vector<int>,greater<int> > q;这里需要注意与sort()传入的参数是相反的。
- 自定义数据类型：

```cpp
struct tmp1 //运算符重载<
{
    int x;
    tmp1(int a) {x = a;}
    bool operator<(const tmp1& a) const
    {
        return x < a.x; //大顶堆
    }
};

priority_queue<tmp1> d;
```

## 基本操作
> top 访问队头元素

> empty 队列是否为空

> size 返回队列内元素个数

> push 插入元素到队尾 (并排序)

> emplace 原地构造一个元素并插入队列

> pop 弹出队头元素

> swap 交换内容
