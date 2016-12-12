#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

void print();
void output_sat();
void output_puz(string res);
void parse_line();
void parse_file();
void parse_result();
void make_cell_cls(int r, int c);
void make_bloc_cls(int idx, int num);
void make_rc_cls(int idx, int num);
int get_rcn_encode(int r, int c, int n);
int* get_rcn_decode(int code);
bool is_solvable();

int N = 0;                      // Sudoku edge size
int n = 0;                      // Sudoku block size
int cls_idx = 0;
int G[100][100];                  // Unsolved Sudoku grid
int _G[100][100];                 // Solved Sudoku grid
string lines[100];
string cls[1000000] = { "" };
stringstream ss;
fstream unsol_puz, sol_puz;
fstream unsol_sat, sol_sat;


int main(int argc, char* argv[])
{

    string cmd;
    string unsol_sat_filename = "unsol_sat";
    string sol_sat_filename = "unsol_sat.sat";
    string unsol_puz_filename = string(argv[1]);
    string sol_puz_filename = string(argv[2]);
    string minisat_bin = string(argv[3]);

    // Open files
    unsol_puz.open(unsol_puz_filename, ios::in);
    unsol_sat.open(unsol_sat_filename, ios::out);

    // Parsing
    parse_line();
    parse_file();

    // For each cell
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            make_cell_cls(r, c);
        }
    }

    // For each row, col, block 
    for (int idx = 0; idx < N; idx++) {
        for (int num = 0; num < N; num++) {
            make_rc_cls(idx, num);
            make_bloc_cls(idx, num);
        }
    }

    // Output and close unsol SAT and puz file 
    output_sat();

    // Close files
    unsol_sat.close();
    unsol_puz.close();

    // Call minisat solver
    cmd = "./" + minisat_bin + " " + unsol_sat_filename + " " + sol_sat_filename;
    system(cmd.c_str());

    // Open solved SAT file and output puzzle file
    sol_sat.open(sol_sat_filename, ios::in);
    sol_puz.open(sol_puz_filename, ios::out);

    // Decode from CNF 
    is_solvable() ? output_puz("SUCCESS") : output_puz("FAIL");

    // Close sol SAT and puz file
    sol_sat.close();
    sol_puz.close();    

    /* // Clean up */
    /* cmd = "rm " + unsol_sat_filename + " " + sol_sat_filename; */
    /* system(cmd.c_str()); */

    return 0;
}

int get_rcn_encode(int r, int c, int n)
{
    return r*N*N + c*N + n + 1;  // Shift from 0~728 to 1~729
}

int* get_rcn_decode(int code)
{
    int* ans = new int[3];

    code -= 1;              // Shift from 1~729 to 0~728

    ans[2] = code%N + 1;    // Shift from 0~8 to 1~9
    code /= N;
    ans[1] = code%N;
    code /= N;
    ans[0] = code%N;

    return ans;
}

bool is_solvable()
{
    string res;
    ss << sol_sat.rdbuf();
    ss >> res;

    if (res == "SAT") {
        int val = 0;

        // Parse result
        while (ss >> val) {
            if (val > 0) {
                int* ans = get_rcn_decode(val);
                int r = ans[0], c = ans[1], n = ans[2];
                _G[r][c] = n;
            }
        }

        return true;
    }
    else {
        return false;
    }


}

void parse_line()
{
    while (getline(unsol_puz, lines[N])) N++;
    n = sqrt(N);
}

void parse_file()
{
    for (int i = 0; i < N; ++i) {
        ss << lines[i];
        for (int j = 0; j < N; ++j) {
            ss >> G[i][j];
        } 
        ss.str("");
        ss.clear();
    }
}

void output_sat()
{
    unsol_sat << "p cnf " << N*N*N << " " << cls_idx << endl;

    for (int i = 0; i < cls_idx; i++) {
        unsol_sat  << cls[i] << endl;
    }

}

void output_puz(string res)
{
    if (res == "SUCCESS") {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                sol_puz << _G[i][j] << " "; 
            }
            sol_puz << endl;
        }
    }
    else {
        cout << "NO" << endl;
        sol_puz << "NO ANSWER" << endl; 
    }
}

void make_rc_cls(int idx, int num)
{
    /* ROW */
    // At least one 
    for (int i = 0; i < N; i++) {
        cls[cls_idx] += to_string(get_rcn_encode(idx, i, num)) + " ";
    } 
    cls[cls_idx++] += "0";

    // At most one
    for (int i = 0; i < N; i++) {
        for (int j = i+1; j < N; j++) {
            cls[cls_idx] += "-" + to_string(get_rcn_encode(idx, i, num)) + " ";
            cls[cls_idx] += "-" + to_string(get_rcn_encode(idx, j, num)) + " ";
            cls[cls_idx++] += "0";
        }
    }

    /* COL */
    // At least one 
    for (int i = 0; i < N; i++) {
        cls[cls_idx] += to_string(get_rcn_encode(i, idx, num)) + " ";
    } 
    cls[cls_idx++] += "0";

    // At most one
    for (int i = 0; i < N; i++) {
        for (int j = i+1; j < N; j++) {
            cls[cls_idx] += "-" + to_string(get_rcn_encode(i, idx, num)) + " ";
            cls[cls_idx] += "-" + to_string(get_rcn_encode(j, idx, num)) + " ";
            cls[cls_idx++] += "0";
        }
    }

}

void make_bloc_cls(int idx, int num)
{
    
    int r_start = floor(idx/n) * n;
    int c_start = (idx%n) * n;

    int ls_idx = 0;
    int* r_ls = new int[N*(N-1)/2];
    int* c_ls = new int[N*(N-1)/2];

    // At least one
    for (int r = r_start; r < r_start + n; r++) {
        for (int c = c_start; c < c_start + n; c++) {
            cls[cls_idx] += to_string(get_rcn_encode(r, c, num)) + " ";
            r_ls[ls_idx] = r;
            c_ls[ls_idx] = c;
            ls_idx++;
        } 
    }
    cls[cls_idx++] += " 0";

    // At most one
    for (int i = 0; i < ls_idx; i++) {
        for (int j = i+1; j < ls_idx; j++) {
            string p1 = to_string(get_rcn_encode(r_ls[i], c_ls[i], num));
            string p2 = to_string(get_rcn_encode(r_ls[j], c_ls[j], num));

            cls[cls_idx] += "-" + p1 + " "; 
            cls[cls_idx] += "-" + p2 + " "; 
            cls[cls_idx++] += "0";
        } 
    }

}

void make_cell_cls(int r, int c)
{
    // For each cell
    if (G[r][c] != 0) {
        cls[cls_idx++] = to_string(get_rcn_encode(r, c, G[r][c]-1)) + " 0";
    }
    else {
        // At least one
        for (int num = 0; num < N; num++) {
            cls[cls_idx] += to_string(get_rcn_encode(r, c, num)) + " ";
        }
        cls[cls_idx++] += "0";

        // At most one
        for (int i = 0; i < N; i++) {
            for (int j = i+1; j < N; j++) {
                cls[cls_idx] += "-" + to_string(get_rcn_encode(r, c, i)) + " ";        
                cls[cls_idx] += "-" + to_string(get_rcn_encode(r, c, j)) + " ";        
                cls[cls_idx++] += "0";
            }
        } 
    }
}

void print()
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << _G[i][j] << " "; 
        }
        cout << endl;
    }
}


