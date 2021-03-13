## priority_queue特点
队列和排序的完美结合，内部是由堆实现的，删除和插入的时间复杂度均为O(logN)

## 定义
> priority_queue<Type, Container, Functional>

举例：
- 升序队列：priority_queue <int,vector<int>,greater<int> > q;这里需要注意与sort()传入的参数是相反的。
- 自定义数据类型：重载operator < 或者重写仿函数
1. 重载operator < 运算符

```cpp
struct Node{
    int x, y;
    Node(int a=0, int b=0):
        x(a),y(b){}
};
bool operator<(Node a, Node b){//返回true时，说明a的优先级低于b
    //x值较大的Node优先级低（x小的Node排在队前）
    //x相等时，y大的优先级低（y小的Node排在队前）
    if( a.x== b.x ) return a.y> b.y;
    return a.x> b.x;
}
int main(){
    priority_queue<Node> q;
    return 0;
}
```

2. 大顶堆，使用greater<>仿函数，并重载operator > 

3. 重写仿函数的例子

```cpp
struct Node{
    int x, y;
    Node( int a= 0, int b= 0 ):
        x(a), y(b) {}
};
struct cmp{
    bool operator() ( Node a, Node b ){//默认是less函数
        //返回true时，a的优先级低于b的优先级（a排在b的后面）
        if( a.x== b.x ) return a.y> b.y;
        return a.x> b.x; }
};
int main(){
    priority_queue<Node, vector<Node>, cmp> q;
    return 0;
}
```


## 基本操作
> top 访问队头元素

> empty 队列是否为空

> size 返回队列内元素个数

> push 插入元素到队尾 (并排序)

> emplace 原地构造一个元素并插入队列

> pop 弹出队头元素

> swap 交换内容
