#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

void print();
bool backtrack(int r, int c);
bool validate(int r, int c, int d);

using namespace std;

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

            // Validate number
            if (validate(r, c, d)) {

                S[r][c] = d;
                if (backtrack(r, c+1)) {
                    return true;
                }
                S[r][c] = 0;
            }
        }

        // No solution found
        return false;
    }
}

bool validate(int r, int c, int d)
{
    // Check row and col
    for (int i = 0; i < length; ++i) {
        if ((S[r][i] == d) || (S[i][c] == d)) { 
            return false; 
        }
    }

    // Check square
    int size = sqrt(length);
    int box_r = floor(r/3) * size;
    int box_c = floor(c/3) * size;

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            if (S[box_r+r][box_c+c] == d) {
                return false;
            }
        }
    }

    return true; 
}

void print()
{
    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < length; ++j) {
            cout << S[i][j];
            if (j == length-1) 
                cout << endl;
            else 
                cout << " ";
            
        } 
    }
}

