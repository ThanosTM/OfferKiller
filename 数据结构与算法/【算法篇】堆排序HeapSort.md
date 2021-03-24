## 原理
- 堆是这样一种数据结构，完全二叉树且非叶子结点的值均不大于或不小于其左或其右的值

- 过程主要有两步：
1. 建堆，将每个非叶子结点调为最大堆
2. 调整堆，将根结点（大顶堆的顶）与最后一个结点交换

- 反复利用完全二叉树这样一个结论：对于序号从根节点0开始记的方式，第i个结点的左结点编号为 2 * i+1，右结点编号为 2 * i+2（如果有的话）

- 时间复杂度为O(NlogN)，空间复杂度O(1)（原地），非稳定排序。

## 实现代码
```cpp
#include <iostream>
#include <vector>
using namespace std;

void heap_sort(vector<int> &array);
void build_heap(vector<int> &array, int n);
void adjust_heap(vector<int> &array, int n, int i);
void print_vector(const vector<int> &array);

int main(){
    vector<int> array{4, 3, 8, 6, 4, 3, 9, 2, 1, 1, 0, 8};
    print_vector(array);
    heap_sort(array);
    print_vector(array);
    return 0;
}

void heap_sort(vector<int> &array){
    int n = array.size();
    build_heap(array, n);

    for(int i = n-1; i > 0; i--){
        swap(array[0], array[i]);
        adjust_heap(array, i, 0);
    }
}   

void build_heap(vector<int> &array, int n){
    for(int i = n/2 - 1; i >= 0; i--){
        adjust_heap(array, n, i);
    }
}

void adjust_heap(vector<int> &array, int n, int i){
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    int largest = i;
    if(left < n && array[left] > array[largest]){
        largest = left;
    }
    if(right < n && array[right] > array[largest]){
        largest = right;
    }
    if(largest != i){
        swap(array[largest], array[i]);
        adjust_heap(array, n, largest);
    }
}

void print_vector(const vector<int> &array){
    for(auto iter : array){
        cout << iter << ' ';
    }
    cout << endl;
}
```
