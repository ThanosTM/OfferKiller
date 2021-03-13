/**
 * @brief：【剑指offer P199】输入一组字母，输出字符的所有组合——回溯法
 * @example：输入"abc"，打印"a", "b", "c", "ab", "ac", "bc", "abc"
 * @author：seu_zhouyi
 */

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

void combines();
void combines_recursive(int index, int count, int m, string &sub);

string S = "abcd";
vector<string> list;

int main(){
    combines();
    for(auto item : list){
        cout << item << endl;
    }
    return 0;
}

/**
 * @brief：从N个数中取m个数的不同组合，1 <= m <= n
 */
void combines(){
    int S_len = S.length();
    assert(S_len > 0);
    
    for(int m = 1; m <= S_len; m++){
        string sub;
        sub.resize(m);
        combines_recursive(0, 0, m, sub);
    }
}

/**
 * @param index ：从第index个数开始取
 * @param count ：已经取了count个字母了
 * @param m     ：一共要取m个字母
 * @param sub   ：现有的字符串
 */
void combines_recursive(int index, int count, int m, string &sub){
    //取够了
    if(count == m){
        list.push_back(sub);
        return;
    }

    //全部取上都不够了
    if(index + m - count > S.length()){
        return;
    }

    //还有剩余，固定index继续向下回溯
    for(int i = index; i < S.length(); i++){
        sub[count++] = S[i];
        combines_recursive(i + 1, count, m, sub);
        count--;
    }
}