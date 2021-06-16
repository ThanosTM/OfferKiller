## 原理
希尔排序是插入排序的一种，又称“缩小增量排序”，是插入排序的一种更加高效的改进版本。
- 在排序前将一个序列分成好几组序列，步长每次排序逐渐缩短直至1
- 每次排序时对每组序列进行插入排序，那么最后一次排序就是普通的插入排序了
- 改进之处在于，随着步长的逐步减小，数组趋于宏观有序，减小了移位和交换的次数
- 如何选择步长的序列是很有讲究的，最简单的可以是每次折半，这深刻影响着排序的时间复杂度，但最优的序列尚未被发现并证明

![image](https://pic4.zhimg.com/80/v2-b6aad859c643c1977c6e7cf4a8d8c727_720w.jpg)


## 实现代码
```cpp
void shell_sort(vector<int> &array){
    int n = array.size();
    for(int step = n/2; step >= 1; step /= 2){
        for(int i = step; i < n; i++){
            int temp = array[i];
            int j = i;
            while(j - step >= 0 && temp < array[j - step]){
                array[j] = array[j - step];
                j -= step;
            }
            array[j] = temp;
        }
    }
}
```

## 应用场景
有些程序员有时候会选择希尔排序，因为对于中等大小的数组它的运行时间可以接受，同时代码量很小，而且不需要使用额外的内存空间。
