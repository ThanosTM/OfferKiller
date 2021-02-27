## 原题回顾
给出 R 行 C 列的矩阵，其中的单元格的整数坐标为 (r, c)，满足 0 <= r < R 且 0 <= c < C。

另外，我们在该矩阵中给出了一个坐标为 (r0, c0) 的单元格。

返回矩阵中的所有单元格的坐标，并按到 (r0, c0) 的距离从最小到最大的顺序排，其中，两单元格(r1, c1) 和 (r2, c2) 之间的距离是曼哈顿距离，|r1 - r2| + |c1 - c2|。（你可以按任何满足此条件的顺序返回答案。）

 

示例 1：

输入：R = 1, C = 2, r0 = 0, c0 = 0
输出：[[0,0],[0,1]]
解释：从 (r0, c0) 到其他单元格的距离为：[0,1]
示例 2：

输入：R = 2, C = 2, r0 = 0, c0 = 1
输出：[[0,1],[0,0],[1,1],[1,0]]
解释：从 (r0, c0) 到其他单元格的距离为：[0,1,1,2]
[[0,1],[1,1],[0,0],[1,0]] 也会被视作正确答案。
示例 3：

输入：R = 2, C = 3, r0 = 1, c0 = 2
输出：[[1,2],[0,2],[1,1],[0,1],[1,0],[0,0]]
解释：从 (r0, c0) 到其他单元格的距离为：[0,1,1,2,2,3]
其他满足题目要求的答案也会被视为正确，例如 [[1,2],[1,1],[0,2],[1,0],[0,1],[0,0]]

## 比较容易能够想到的方法
#### 自定义排序

```cpp
vector<vector<int>> allCellsDistOrder(int R, int C, int r0, int c0) {
     vector<vector<int>> ans(R*C, vector<int>(2));
        for(int i = 0; i < R; i++){
            for(int j = 0; j < C; j++){
                ans[j+i*C] = vector<int>{i, j};
            }
        }
        sort(ans.begin(), ans.end(), [&](vector<int> a, vector<int> b){
            return abs(a[0]-r0)+abs(a[1]-c0) < abs(b[0]-r0) + abs(b[1]-c0);
        });
    
        return ans;
}
```

#### BFS

```cpp
int dist(int r1, int c1, int r2, int c2){
    return abs(r1-r2) + abs(c1-c2);
}

vector<vector<int>> allCellsDistOrder(int R, int C, int r0, int c0) {
    vector<vector<vector<int>>>bucket(maxSize+1,vector<vector<int>>());
    
    for(int i = 0; i < R; i++){
        for(int j = 0; j < C; j++){
            bucket[dist(i, j, r0, c0)].emplace_back(vector<int>{i, j});
        }
    }

    vector<vector<int>> ans;
    for(int i = 0; i <= maxSize; i++){
        ans.insert(ans.end(), bucket[i].begin(), bucket[i].end());
    }
    return ans;
}

```

## 桶排序
按距离大小分组，每组相当于一个桶，其中的距离值相等


```cpp
class Solution {
public:
    int dist(int r1, int c1, int r2, int c2){
        return abs(r1-r2) + abs(c1-c2);
    }

    vector<vector<int>> allCellsDistOrder(int R, int C, int r0, int c0) {
        int dir[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

        vector<vector<bool>> isVisited(R, vector<bool>(C, false));
        queue<vector<int>> Q;
        Q.push(vector<int>{r0, c0});
        isVisited[r0][c0] = true;

        vector<vector<int>> ans;
        while(!Q.empty()){
            vector<int> front = Q.front();
            Q.pop();
            ans.emplace_back(front);
            int r1 = front[0], c1 = front[1];

            for(int i = 0; i < 4; i++){
                int r2 = r1+dir[i][0], c2 = c1+dir[i][1];
                if(r2 < R && r2 >= 0 && c2 < C && c2 >= 0 && !isVisited[r2][c2]){
                    Q.push(vector<int>{r2, c2});
                    isVisited [r2] [c2] = true;
                }
            }
        }
        return ans;
    }
};
```


