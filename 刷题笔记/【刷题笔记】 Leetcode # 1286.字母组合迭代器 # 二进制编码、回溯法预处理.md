## 原题回顾
请你设计一个迭代器类，包括以下内容：

一个构造函数，输入参数包括：一个 有序且字符唯一 的字符串 characters（该字符串只包含小写英文字母）和一个数字 combinationLength 。
函数 next() ，按 字典序 返回长度为 combinationLength 的下一个字母组合。
函数 hasNext() ，只有存在长度为 combinationLength 的下一个字母组合时，才返回 True；否则，返回 False。
 

示例：

```cpp
// 创建迭代器 iterator
CombinationIterator iterator = new CombinationIterator("abc", 2); 


iterator.next(); // 返回 "ab"
iterator.hasNext(); // 返回 true
iterator.next(); // 返回 "ac"
iterator.hasNext(); // 返回 true
iterator.next(); // 返回 "bc"
iterator.hasNext(); // 返回 false
```

提示：

- 1 <= combinationLength <= characters.length <= 15
- 每组测试数据最多包含 10^4 次函数调用。
- 题目保证每次调用函数 next 时都存在下一个字母组合。

## 我的解答——回溯法预处理
题目给的自由发挥空间比较大，很明显有两种思路，也各有优缺点：
- 提前预处理，将所有可能都先给出来，构造函数开销大，内存占用大，而next()和hasNext()开销极小，属于空间换时间；
- 即时处理，即在next()中完成主要的运算，内存占用小，每次next()调用开销大

其实即时处理更加符合题目的本意，因为刷到此题的时候正好在看回溯法专项，因此自己写的是使用回溯法预处理的解法。

```cpp
class CombinationIterator {
private:
    string str;
    int comb_len;
    int cur_pos;
    vector<string> comb_vt;

public:
    CombinationIterator(string characters, int combinationLength) {
        str = characters;
        comb_len = combinationLength;
        cur_pos = 0;
        string comb_str(comb_len, '\0');
        backtracking(0, 0, comb_str);
    }

    void backtracking(int count, int pos, string &comb_str){
        if(count == comb_len){
            comb_vt.emplace_back(comb_str);
            return;
        }
        for(int i = pos; i < str.length(); i++){
            comb_str[count++] = str[i];
            backtracking(count, i+1, comb_str);
            count--;
        }
    }
    
    string next() {
        string res = (cur_pos < comb_vt.size()) ? comb_vt[cur_pos] : comb_vt[comb_vt.size()-1];
        cur_pos++;
        return res;
    }
    
    bool hasNext() {
        return cur_pos < comb_vt.size();
    }
};
```

## 即时处理：常规的思路——找规律

```
****************************************
a b c     =>     a b d     =>     a b e
    ^                ^
    d                e
****************************************
a b e     =>     a c d
  ^ ^
  c d
****************************************
a c d     =>     a c e     =>     a d e
    ^              ^ ^
    e              d e
****************************************
a d e     =>     b c d
^ ^ ^
b c d
****************************************
```

```cpp
class CombinationIterator {
private:
    string str;
    int comb_len;
    vector<int> pos;
    bool finished;

public:
    CombinationIterator(string characters, int combinationLength) {
        str = characters;
        comb_len = combinationLength;
        finished = false;
        for(int i = 0; i < comb_len; i++){
            pos.push_back(i);
        }
    }
    
    string next() {
        string res;
        for(auto item : pos){
            res += str[item];
        }

        int i;
        for(i = comb_len-1; i >= 0; i--){
            if(pos[i] < str.length()-comb_len+i){
                pos[i]++;
                break;
            }
        }

        if(i < 0){
            finished = true;
        }
        else{
            for(int j = i+1; j < comb_len; j++){
                pos[j] = pos[j-1]+1;
            }
        }
        return res;
    }
    
    bool hasNext() {
        return !finished;
    }
};
```

## 二进制编码
1011 -> 1表示取该位的数，故应该有combinationLen个的1

```cpp
class CombinationIterator {
private:
    string str;
    int comb_len;
    int key;

public:
    CombinationIterator(string characters, int combinationLength) {
        str = characters;
        comb_len = combinationLength;
        key = ((1 << combinationLength) - 1) << (str.length() - comb_len);
    }
    
    string next() {
        string res;
        while((key > (1 << comb_len) - 1) && CountOneBit(key) != comb_len){
            key--;
        }

        for(int i = str.length()-1; i >= 0; i--){
            if(key & (1 << i)){
                res += str[str.length() - 1 - i];
            }
        }
        key--;

        return res;
    }
    
    bool hasNext() {
        return key > (1 << comb_len) - 1;
    }

    int CountOneBit(int num){
        int count = 0;
        while(num != 0){
            count++;
            num &= (num-1);
        }
        return count;
    }
};
```





