## 原题描述
给定一个链表，判断链表中是否有环。

如果链表中有某个节点，可以通过连续跟踪 next 指针再次到达，则链表中存在环。 为了表示给定链表中的环，我们使用整数 pos 来表示链表尾连接到链表中的位置（索引从 0 开始）。 如果 pos 是 -1，则在该链表中没有环。注意：pos 不作为参数进行传递，仅仅是为了标识链表的实际情况。

如果链表中存在环，则返回 true 。 否则，返回 false 。


进阶：

你能用 O(1)（即，常量）内存解决此问题吗？

示例：

![image](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2018/12/07/circularlinkedlist.png)

```
输入：head = [3,2,0,-4], pos = 1
输出：true
解释：链表中有一个环，其尾部连接到第二个节点。
```

## 我的实现V1.0——哈希表

```cpp
class Solution {
public:
    bool hasCycle(ListNode *head) {
        unordered_map<ListNode *, int>map;
        ListNode *p = head;
        while(p){
            if(map[p] == 0)map[p]++;
            else return true;
            p = p->next;
        }
        return false;
    }
};
```

```
执行用时: 28 ms
内存消耗: 10.4 MB
```

#### 点评
时间复杂度为O(n)，空间复杂度为O(n)，显然不符合题目的进阶要求

## 我的实现V2.0——链表反向

```cpp
class Solution {
public:
    bool hasCycle(ListNode *head) {
        if(!head || !head->next)return false;
        ListNode *pre = head;
        ListNode *now = pre->next;
        pre->next = nullptr;
        while(now){
            if(now == head)return true;
            ListNode *next = now->next;
            now->next = pre;
            pre = now;
            now = next;
        }
        return false;
    }
};
```

```
执行用时: 8 ms
内存消耗: 7.4 MB
```

#### 点评
时间复杂度为O(n)，而空间复杂度降低至O(1)，勉强算是满足题目要求，缺点是改动了原有链表。

## 看看大佬是怎么实现的——快慢指针

```cpp
class Solution {
public:
    bool hasCycle(ListNode *head) {
        ListNode *fast = head;
        ListNode *slow = head;
        while(fast && fast->next){
            fast = fast->next->next;
            slow = slow->next;
            if(fast == slow)return true;
        }
        return false;
    }
};
```

```
执行用时: 8 ms
内存消耗: 7.5 MB
```

#### 点评
时间复杂度为O(n)，空间复杂度为O(1)，而且保留了原来的链表，是最优秀的实现方式了，并且下面将会看到，还能求出入环的第一个元素的位置。

## 环形链表的扩展
#### 题目描述
给定一个链表，返回链表开始入环的第一个节点。 如果链表无环，则返回 null。

为了表示给定链表中的环，我们使用整数 pos 来表示链表尾连接到链表中的位置（索引从 0 开始）。 如果 pos 是 -1，则在该链表中没有环。注意，pos 仅仅是用于标识环的情况，并不会作为参数传递到函数中。

说明：不允许修改给定的链表。

进阶：

你是否可以不用额外空间解决此题？

#### 题目分析
- 这次明确规定了不允许修改给定的链表，因此V2.0就不能使用了，V1.0的哈希表仍然能够使用但不满足进阶的需求。

- 考虑使用快慢指针时，两个指针走过路的差距


```
设入环前有 a 个元素，环内有 b 个元素，则题目所求即为第 a 个元素。
快指针一次走2格，慢指针一次走1格，相遇时正好套一圈，
此时若慢指针走了s步，则快指针走了f = 2s = s + b步。
下面将慢指针继续每次1格走，而快指针回到原点每次1步走，相遇处即为第a个元素
```

#### 实现代码

```cpp
class Solution {
public:
    ListNode *detectCycle(ListNode *head) {
        ListNode *fast = head;
        ListNode *slow = head;
        while(fast && fast->next){
            fast = fast->next->next;
            slow = slow->next;
            if(slow == fast){
                fast = head;
                while(slow != fast){
                    slow = slow->next;
                    fast = fast->next;
                }
                return slow;
            }
        }
        return nullptr;
    }
};
```



