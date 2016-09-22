#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

using namespace std;

void print();
bool backtrack(int r, int c);
int validate(int r, int c, int d);

int S[25][25];
int length = 0;
string lines[25];
stringstream ss;

int main(void)
{
    // Calculate length
    while (getline(cin, lines[length])) length++;
    
    // Parse lines to S matrix
    for (int i = 0; i < length; ++i) {
        ss << lines[i];
        for (int j = 0; j < length; ++j) {
            ss >> S[i][j];
        } 
        ss.str("");
        ss.clear();
    }

    // cout << "Length: " << length << endl;

    // Sweet Backtracking
    if (backtrack(0, 0) == true) 
        print();
    else 
        cout << "NO" << endl;        
    
    return 0;
}

bool backtrack(int r, int c)
{
    // Last cell 
    if (r == length-1 && c == length)  
        return true;

    // Jump to next row
    if (c == length) {
        c = 0;
        r++;
    }


    // Check if the cell has default numver
    if (S[r][c] != 0) {
        return backtrack(r, c+1);
    } 
    else {
        // Iterate all possible number
        for (int d = 1; d <= length; ++d) {
            // cout << "----------" << endl;
            // cout << "(" << r << " " << c << ")" << endl;
            // print();

            // Validate number
            int res = validate(r, c, d);
            if (res == 0) {
                // cout << "assign " << d << endl;

                S[r][c] = d;
                if (backtrack(r, c+1)) {
                    return true;
                }
                S[r][c] = 0;
            }
            // else {
                // cout << "val fail " << res << " with num " << d << endl;
            // }
        }

        // No solution found
        return false;
    }
}

int validate(int r, int c, int d)
{
    // Check row and col
    for (int i = 0; i < length; ++i) {
        if ((S[r][i] == d) || (S[i][c] == d)) { 
            // cout << "ri " << S[r][i] << endl;
            // cout << "ic " << S[i][c] << endl;
            // cout << d << endl;
            return 1; 
        }
    }

    // Check square
    int box_r = floor(r/3);
    int box_c = floor(c/3);
    int size = sqrt(length);
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (S[box_r*size+i][box_c*size+j] == d) {
                // cout << box_r << endl;
                // cout << box_c << endl;
                // cout << size << endl;
                return 2;
            }
        } 
    }

    return 0; 
}

void print()
{
    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < length; ++j) {
            cout << S[i][j] << " ";
        } 
        cout << endl;
    }
}

