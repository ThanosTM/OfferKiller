## 原题回顾
现在你总共有 n 门课需要选，记为 0 到 n-1。

在选修某些课程之前需要一些先修课程。 例如，想要学习课程 0 ，你需要先完成课程 1 ，我们用一个匹配来表示他们: [0,1]

给定课程总量以及它们的先决条件，返回你为了学完所有课程所安排的学习顺序。

可能会有多个正确的顺序，你只要返回一种就可以了。如果不可能完成所有课程，返回一个空数组。

示例 1:

```
输入: 2, [[1,0]] 
输出: [0,1]
解释: 总共有 2 门课程。要学习课程 1，你需要先完成课程 0。因此，正确的课程顺序为 [0,1] 。
```

示例 2:

```
输入: 4, [[1,0],[2,0],[3,1],[3,2]]
输出: [0,1,2,3] or [0,2,1,3]
解释: 总共有 4 门课程。要学习课程 3，你应该先完成课程 1 和课程 2。并且课程 1 和课程 2 都应该排在课程 0 之后。
     因此，一个正确的课程顺序是 [0,1,2,3] 。另一个正确的排序是 [0,2,1,3] 。
```

## 拓扑排序
给定一个包含 nn 个节点的有向图G，我们给出它的节点编号的一种排列，如果满足，那么称该排列是图 G 的「拓扑排序」
> 对于图G的任意一条有向边(u,v)，u在排列中都出现在v的前面

若G存在环，即图G不是有向无环图，则G不存在拓扑排序

## BFS解法
思路：若A依赖于B，则B到A连一条有向边，用队列维护一个所有入度为0的节点（即不依赖于其他任何一门课程），遍历到某一节点C后，将被C指向的节点的入度减一（一门受依赖的课程学完了），入度减为0则加入队列（所有受依赖的课程学完，可以进行学习了）
```
class Solution {
public:
    vector<int> findOrder(int numCourses, vector<vector<int>>& prerequisites) {
        //BFS
        vector<int> ans;
        vector<vector<int>> G(numCourses);
        vector<int> in_degree(numCourses, 0);
        for(auto item : prerequisites){
            G[item[1]].emplace_back(item[0]);
            in_degree[item[0]] ++;
        }

        queue<int> Q;
        for(int i = 0; i < numCourses; i++){
            if(in_degree[i] == 0){
                Q.push(i);
            }
        }
        
        while(!Q.empty()){
            int cur = Q.front();
            Q.pop();
            ans.push_back(cur);
            for(auto v : G[cur]){
                in_degree[v] --;
                if(in_degree[v] == 0){
                    Q.push(v);
                }
            }
        }

        if(ans.size() != numCourses)return {};
        return ans;
    }
};
```

## DFS解法
思路：若A依赖于B，则从A到B连一条有向边；将每个节点分为3种状态：
- “未访问”：还没有对遍历到这个节点
- “访问中”：正在由该节点进行DFS，但还未回溯回来，还有邻接的节点未访问
- “已访问”：该节点的所有边均已访问结束，加入结果栈中

每一轮选取一个“未访问”的节点u进行访问，标记为“访问中”，下面节点v会出现3中情况：
- “未访问”：对该节点进行DFS
- “访问中”：图存在环，不存在拓扑排序
- “已访问”：节点v已在栈中，那么节点u必定在v的后面，不影响拓扑排序结果，略过


```cpp
class Solution {
private:
    vector<vector<int>> edges;
    vector<int> result;
    vector<int> visited;
    bool valid = true;

    void dfs(int u){
        if(!valid)return;

        visited[u] = 1;
        for(auto v : edges[u]){
            if(visited[v] == 0){
                dfs(v);
                if(!valid)return;
            }
            else if(visited[v] == 1){
                valid = false;
                return;
            }
        }

        result.push_back(u);
        visited[u] = 2;
    }

public:
    vector<int> findOrder(int numCourses, vector<vector<int>>& prerequisites) {
        //DFS
        edges.resize(numCourses);
        visited.resize(numCourses);

        for(auto item : prerequisites){
            edges[item[0]].push_back(item[1]);
        }

        for(int i = 0; i < numCourses; i++){
            if(visited[i] == 0){
                dfs(i);
                if(!valid)break;
            }
        }
        
        if(!valid)return {};
        return result;
    }
};
```



