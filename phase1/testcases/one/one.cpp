#include <bits/stdc++.h>
using namespace std;

class Solution {
public:
    void solveSudoku(vector<vector<char>>& board) {
        vector<int> rows(9,511);
        vector<int> cols(9,511);
        vector<int> boxes(9,511);
        auto set = [&](int i, int j, int num)->void{
            board[i][j] = '0' + num;
            num--;
            rows[i] ^= 1<<num;
            cols[j] ^=1<<num;
            boxes[i/3*3+j/3] ^= 1<<num;
        };
        auto unset = [&](int i, int j, int num)->void{
            board[i][j] = '.';
            num--;
            rows[i] |= 1<<num;
            cols[j] |=1<<num;
            boxes[i/3*3+j/3] |= 1<<num;
        };
        auto check = [&](int i, int j, int num)->bool{
            num--;
            return boxes[i/3*3+j/3]&cols[j]&rows[i]&(1<<num);
        };
        for(int i=0;i<9;i++){
            for(int j=0;j<9;j++){
                if(board[i][j]!='.'){
                    int num = board[i][j] - '0';
                    set(i,j,num);
                }
            }
        }
        function<bool(int)> mark = [&](int n)->bool{
            if(n==81){
                return true;
            }
            int i = n / 9;
            int j = n % 9;
            if(board[i][j]!='.'){
                return mark(n+1);
            }
            for(int num=1;num<=9;num++){
                if(check(i,j,num)){
                    set(i,j,num);
                    if(mark(n+1)){
                        return true;
                    }
                    unset(i,j,num);
                }
            }
            return false;
        };
        mark(0);  
    }
};