## 原题回顾
给定 n 个非负整数表示每个宽度为 1 的柱子的高度图，计算按此排列的柱子，下雨之后能接多少雨水。



上面是由数组 [0,1,0,2,1,0,1,3,2,1,2,1] 表示的高度图，在这种情况下，可以接 6 个单位的雨水（蓝色部分表示雨水）。 感谢 Marcos 贡献此图。

示例:

![image](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2018/10/22/rainwatertrap.png)

输入: [0,1,0,2,1,0,1,3,2,1,2,1]
输出: 6

## 题目分析
#### 最容易想到的方法
对每个i左右扫描，找到高度不小于它的两根柱子，求出i处接的雨水，该种方法的时间复杂度为O(n^2)，因此，当输入数组元素很多时，将超时；

#### 怎么想到“栈”的呢？
- 这时需要一个复杂度为O(n)的算法，这也就意味着，只能扫描常数多次，而每次扫描时，有可能要用到此前扫描过的柱子的高度，这就需要一个数据结构加以记录；
- 另一方面，这是不是有点类似于括号的匹配？只有出现了不低于之前柱子的高度，才能计算两者间的接水量，而此后就和第一根柱子没有关系了...

#### 单调递减栈
由上面的分析可以得知，使用到“栈”是一个很自然的想法。单调递减栈形容这样一种栈，从栈底到栈顶的元素是递减的，当需要压入一个元素时，需要先从栈顶弹出所有小于它的元素。

#### 单调递增/减栈的其他应用
- [每日气温](https://blog.csdn.net/fyydhsw/article/details/102830770)
- [最大子矩阵的大小](https://zhuanlan.zhihu.com/p/101785785)

## 本题实现

```cpp
class Solution {
public:
    int trap(vector<int>& height) {
        stack<int> st;
        int sum = 0;
        int last;
        for(int i = 0; i < height.size(); i++){
            while(!st.empty() && height[st.top()] <= height[i]){
                last = height[st.top()];
                st.pop();
                if(st.empty())break;
                sum += (i - st.top()-1)*(min(height[st.top()], height[i])-last);
            }
            st.push(i);
        }
        return sum;
    }
};
```

## 扩展1：每日气温

```cpp
#include <iostream>
#include <vector>
#include <stack>
using namespace std;

//单调递减栈：时间复杂度O(n)

void print_vector(const vector<int> &vt){
    for(int i = 0; i < vt.size(); i++){
        printf("%d ", vt[i]);
    }
    printf("\n");
}

vector<int> convert(const vector<int> &vt){
    vector<int> ans(vt.size(), 0);
    stack<int> st;
    for(int i = 0; i < vt.size(); i++){
        while(!st.empty() && vt[i] > vt[st.top()]){
            ans[st.top()] = i-st.top();
            st.pop();
        }
        st.push(i);
    }
    return ans;
}

int main(){
    freopen("daily_input.txt", "r", stdin);
    freopen("daily_output.txt", "w", stdout);
    vector<int> temp;
    int input;
    while(scanf("%d", &input) == 1){
        temp.emplace_back(input);
    }

    print_vector(convert(temp));

    return 0;

}
```

## 扩展2：最大子矩形的大小

思路：有些类似于接雨水，对于每列矩形，需要左右延伸至尽可能远的地方，而这就可以使用一个单调递增栈实现


```cpp
#include <iostream>
#include <vector>
#include <stack>
using namespace std;

void print_vector(const vector<int> &vt){
    for(int i = 0; i < vt.size(); i++){
        printf("%d ", vt[i]);
    }
    printf("\n");
}

int main(){
    freopen("maxsubrect_input.txt", "r", stdin);
    vector<int> rect;
    int sub;
    while(scanf("%d", &sub) == 1)rect.emplace_back(sub);

    print_vector(rect);

    stack<int> st;
    int maxans = 0;
    for(int i = 0; i < rect.size(); i++){
        while(!st.empty() && rect[st.top()] >= rect[i]){
            int top = st.top();
            st.pop();
            int left = st.empty() ? -1 : st.top();
            printf("%d ", (i - left -1) * rect[top]);
            maxans = max(maxans, (i - left -1) * rect[top]);
        }
        st.push(i);
    }

    while(!st.empty()){
        int top = st.top();
        st.pop();
        int left = st.empty() ? -1 : st.top();
        printf("%d ", (rect.size() - left - 1)*rect[top]);
        maxans = max(maxans, (int)(rect.size()-left-1)*rect[top]);
    }

    printf("\nmaxans = %d\n", maxans);
    return 0;
}
```

## 扩展2的扩展
#### 问题描述：
给定一个整型矩阵map，其中的值只有0和1两种，求其中全是1的所有矩形区域中，最大的矩形区域为1的数量。

例如：
1 1 1 0

其中，最大的矩形区域有3个1，所以返回3。

再如：

1 0 1 1

1 1 1 1

1 1 1 0

其中，最大的矩形区域有6个1，所以返回6.

#### 解法
对于每一行，分别计算当前列的高度（遇到0则立即清0），并利用扩展2解法求出当前行的最大子矩形大小，最后求总体最大的大小
