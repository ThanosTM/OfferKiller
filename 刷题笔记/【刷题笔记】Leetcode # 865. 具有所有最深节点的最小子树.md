## 原题回顾
给定一个根为root的二叉树，每个节点的深度是 该节点到根的最短距离 。

如果一个节点在 整个树 的任意节点之间具有最大的深度，则该节点是 最深的 。

一个节点的 子树 是该节点加上它的所有后代的集合。

返回能满足 以该节点为根的子树中包含所有最深的节点 这一条件的具有最大深度的节点。

```
输入：root = [3,5,1,6,2,0,8,null,null,7,4]
输出：[2,7,4]
解释：
我们返回值为 2 的节点，在图中用黄色标记。
在图中用蓝色标记的是树的最深的节点。
注意，节点 5、3 和 2 包含树中最深的节点，但节点 2 的子树最小，因此我们返回它。

输入：root = [1]
输出：[1]
解释：根节点是树中最深的节点。

输入：root = [0,1,3,null,2]
输出：[2]
解释：树中最深的节点为 2 ，有效子树为节点 2、1 和 0 的子树，但节点 2 的子树最小。
```

## 我的解法：
思路，层序遍历，同时记录每个结点的父节点，然后从最后一层开始向上找最近的公共父节点

```cpp
class Solution {
public:
    TreeNode* subtreeWithAllDeepest(TreeNode* root) {
        unordered_map<TreeNode *, TreeNode *> parent;
        queue<TreeNode *> q;
        q.push(root);
        parent[root] = nullptr;

        vector<TreeNode *> vt;
        while(!q.empty()){
            int size = q.size();
            vt.clear();
            for(int i = 0; i < size; i++){
                TreeNode *cur = q.front();
                q.pop();

                if(cur->left){
                    q.push(cur->left);
                    parent[cur->left] = cur;
                }
                if(cur->right){
                    q.push(cur->right);
                    parent[cur->right] = cur;
                }
                vt.push_back(cur);
            }
        }

        int size = vt.size();
        if(size == 1)return vt[0];
        while(1){
            bool finish = true;
            vt[0] = parent[vt[0]];
            for(int i = 1; i < size; i++){
                vt[i] = parent[vt[i]];
                if(vt[i] != vt[i-1])finish = false;
            }
            if(finish)break;
        }

        return vt[0];
    }
};
```

## 换个思路：寻找具有最深节点的最小公共父节点
#### 分别递归的获得左右子树的高度
1. 左右子树高度相同，说明当前节点就是那个 最小公共父节点
2. 左子树高度大于右子树高度， 递归判断 左子树
3. 右子树高度大于左子树高度 递归判断 右子树

```cpp
int GetDeep(TreeNode* root)
{
    if (!root) return -1;
    return 1 + max(GetDeep(root->left), GetDeep(root->right));
}
TreeNode* subtreeWithAllDeepest(TreeNode* root) {
    if(!root) 
        return nullptr;
    int left = GetDeep(root->left);
    int right = GetDeep(root->right);
    if (left == right) return root;
    else if(left > right)
    {
        return subtreeWithAllDeepest(root->left);
    }
    else
        return subtreeWithAllDeepest(root->right);
}
```

#### 进一步优化：让计算深度不重复进行

```cpp
TreeNode* GetDeep(TreeNode* root, int &deep)
{
    if (!root){
        deep = -1;
        return nullptr;
    }
    int deepl, deepr;
    TreeNode *resl = GetDeep(root->left, deepl);
    TreeNode *resr = GetDeep(root->right, deepr);
    if(deepl > deepr){
        deep = deepl + 1;
        return resl;
    }
    else if(deepl < deepr){
        deep = deepr + 1;
        return resr;
    }
    else{
        deep = deepr + 1;
        return root;
    }
}
TreeNode* subtreeWithAllDeepest(TreeNode* root) {
    if(!root) 
        return nullptr;
    int deep;
    return GetDeep(root, deep);
}
```



