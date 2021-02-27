## 原题回顾
给出两个 非空 的链表用来表示两个非负的整数。其中，它们各自的位数是按照 逆序 的方式存储的，并且它们的每个节点只能存储 一位 数字。

如果，我们将这两个数相加起来，则会返回一个新的链表来表示它们的和。

您可以假设除了数字 0 之外，这两个数都不会以 0 开头。

示例：


```
输入：(2 -> 4 -> 3) + (5 -> 6 -> 4)
输出：7 -> 0 -> 8
原因：342 + 465 = 807
```


## 学习笔记

 - 双链表的操作
 

```cpp
ListNode *p1 = l1, *p2 = l2；
while(p1 || p2){
	int x1 = p1?p1->val:0;
  	int x2 = p2?p2->val:0;
	//...
  	if(p1)p1 = p1->next;
  	if(p2)p2 = p2->next;
}
```

 - 头部节点的处理

```cpp
ListNode *head = new ListNode(0);
//...
ListNode *result = head->next;
delete head;
return result;
```

 - 特殊情况的考虑
 考虑一种特殊的情况：
 输入：[5] +[5]
 输出：[0, 1]
因此即使双链表均到尾部后，此时仍有进位的话应该再额外增添一个节点

## 实现代码

```cpp
/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        ListNode *head = new ListNode(0);
        ListNode *p1 = l1, *p2 = l2, *pr = head;
        int counter = 0;
        while(p1 || p2){
            int x1 = p1?p1->val:0;
            int x2 = p2?p2->val:0;
            int sum = counter+x1+x2;
            if(sum > 9){
                counter = 1;
                sum -= 10;
            }else{
                counter = 0;
            }
            pr->next = new ListNode(sum);
            pr = pr->next;
            if(p1)p1 = p1->next;
            if(p2)p2 = p2->next;
        }
        if(counter)
            pr->next = new ListNode(counter);
        ListNode *result = head->next;
        delete head;
        return result;
    }
};
```