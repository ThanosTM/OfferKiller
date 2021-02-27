## 原题回顾
根据一棵树的中序遍历与后序遍历构造二叉树。

注意:
你可以假设树中没有重复的元素。

例如，给出

中序遍历 inorder = [9,3,15,20,7]
后序遍历 postorder = [9,15,7,20,3]
返回如下的二叉树：

```cpp
    3
   / \
  9  20
    /  \
   15   7
```

通过次数58,813提交次数83,421

## 实现V1.0

```cpp
class Solution {
public:
    TreeNode* buildTree(vector<int>& inorder, vector<int>& postorder) {
        if(postorder.size() == 0 || inorder.size() == 0)return NULL;
        int root_value = postorder[postorder.size()-1];
        auto it = find(inorder.begin(), inorder.end(), root_value);
        TreeNode *root = new TreeNode(root_value);
        vector<int> left_vect1 = vector<int>(inorder.begin(), it);
        vector<int> left_vect2 = vector<int>(postorder.begin(), postorder.begin()+left_vect1.size());
        vector<int> right_vect1 = vector<int>(it+1, inorder.end());
        vector<int> right_vect2 = vector<int>(postorder.begin()+left_vect1.size(), postorder.end()-1);
        root->left = buildTree(left_vect1, left_vect2);
        root->right = buildTree(right_vect1, right_vect2);
        return root;
    }
}
```

> 执行用时: 88 ms 
> 内存消耗: 64.2 MB

## 性能优化
实现V1.0中的性能分析：每一次递归将构造出新的vector用于参数传递，这其中将对每个元素进行复制，不仅是性能还是内存开销巨大；是否能够一直沿用一开始的vector参数，而采用左右索引标记范围，则能够省下巨大的开销。

## 实现V2.0
```cpp
class Solution {
public:
    TreeNode* buildTree(vector<int>& inorder, vector<int>& postorder) {
        return build(inorder, 0, inorder.size()-1, postorder, 0, postorder.size()-1);
    }

    TreeNode *build(vector<int>& inorder, int leftin, int rightin, vector<int>& postorder, int leftpost, int rightpost){
        if(leftin > rightin)return NULL;
        int rootin = leftin;
        while(rootin <= rightin && inorder[rootin] != postorder[rightpost])rootin++;
        TreeNode *root = new TreeNode(postorder[rightpost]);
        root->left = build(inorder, leftin, rootin-1, postorder, leftpost, leftpost+rootin-leftin-1);
        root->right = build(inorder, rootin+1, rightin, postorder, leftpost+rootin-leftin, rightpost-1);
        return root;
    }
};
```