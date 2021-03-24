## 原理
- 经典的分治思想
- 时间复杂度O(NlogN)，空间复杂度O(N)，是稳定的排序算法
![image](https://img-blog.csdnimg.cn/20190313201623880.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2FhYmJjYzEzMg==,size_16,color_FFFFFF,t_70)
![image](https://img-blog.csdnimg.cn/20190313201838372.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2FhYmJjYzEzMg==,size_16,color_FFFFFF,t_70)

## 实现代码

递归方式
```
#include <iostream>
#include <vector>
using namespace std;

void print_vector(const vector<int> &array);
void merge(vector<int> &array, int low, int mid, int high);
void merge_sort(vector<int> &array, int low, int high);

int main(){
    vector<int> array{4, 3, 8, 6, 4, 3, 9, 2, 1, 1, 0, 8};
    print_vector(array);
    merge_sort(array, 0, array.size()-1);
    print_vector(array);
    return 0;
}

void print_vector(const vector<int> &array){
    for(auto iter : array){
        cout << iter << ' ';
    }
    cout << endl;
}

void merge(vector<int> &array, int low, int mid, int high){
    int i = low, j = mid + 1, k = 0;
    int *temp = new int[high - low + 1];
    while(i <= mid && j <= high){
        if(array[i] <= array[j]){
            temp[k++] = array[i++];
        }
        else{
            temp[k++] = array[j++];
        }
    }

    while(i <= mid){
        temp[k++] = array[i++];
    }

    while(j <= high){
        temp[k++] = array[j++];
    }

    for(i = low, k = 0; i <= high; i++, k++){
        array[i] = temp[k];
    }
    delete[] temp;
}

void merge_sort(vector<int> &array, int low, int high){
    if(low < high){
        int mid = low + (high - low)/2;
        merge_sort(array, low, mid);
        merge_sort(array, mid + 1, high);
        merge(array, low, mid, high);
    }
}
```

非递归方式
```cpp
void merge_sort2(vector<int> &array){
    int n = array.size();
    int size = 1, low, high, mid;
    while(size < n){
        low = 0;
        while(low + size < n){
            mid = low + size - 1;
            high = mid + size;
            if(high > n-1)
                high = n-1;

            merge(array, low, mid, high);
            low = high + 1;
        }
        size *= 2;
    }
}
```
