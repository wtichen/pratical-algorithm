#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <unistd.h>

using namespace std;

void print();
void output_sat();
void output_puz();
void make_cls(string opt, int idx, int num);
int get_num_pos(string opt, int idx, int num);
int get_rcn_encode(int r, int c, int n);
int* get_rcn_decode(int code);

int N = 0, n = 0;
int cls_idx = 0;
int G[16][16];
string lines[16];
string cls[3000000] = { "" };
stringstream ss;
fstream unsol_puz, sol_puz;
fstream unsol_sat, sol_sat;

string unsol_sat_filename = "unsol_sat";
string sol_sat_filename = "sol_sat";

int main(int argc, char* argv[])
{
    // argc handle

    // Initialization

    // Open files
    unsol_puz.open(argv[1], ios::in);
    unsol_sat.open(unsol_sat_filename, ios::out);
    
    // Calculate length
    while (getline(unsol_puz, lines[N])) N++;
    n = sqrt(N);
    
    // Parse lines to G matrix
    for (int i = 0; i < N; ++i) {
        ss << lines[i];
        for (int j = 0; j < N; ++j) {
            ss >> G[i][j];
        } 
        ss.str("");
        ss.clear();
    }

    // Prefill
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (G[r][c] != 0) {
                cls[cls_idx++] = to_string(r) + to_string(c) + to_string(G[r][c]) + " 0";
            }
        }
    }

    // Generate row cls
    for (int r = 0; r < N; r++) {
        for (int num = 1; num <= N; num++) {
            int num_pos = get_num_pos("ROW", r, num);

            if (num_pos == -1) {
                make_cls("ROW", r, num);
                 
            }
        }
    }

    // Generate col cls
    for (int c = 0; c < N; c++) {
        for (int num = 1; num <= N; num++) {
            int num_pos = get_num_pos("COL", c, num);

            if (num_pos == -1) {
                make_cls("COL", c, num);

            } 
        }
    }

    // Generate bloc cls
    for (int b = 0; b < N; b++) {
        for (int num = 1; num <= N; num++) {
            int num_pos = get_num_pos("BLOC", b, num);

            if (num_pos == -1) {
                make_cls("BLOC", b, num);
            } 
        }
    }

    // One cell for one num
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {

            if (G[r][c] == 0) {
                // At least one
                // for (int num = 1; num <= N; num++) {
                    // cls[cls_idx] += to_string(r+1) + to_string(c+1) + to_string(num) + " ";
                // }

                // cls[cls_idx++] += "0";

                // At most one
                for (int i = 1; i <= N; i++) {
                    for (int j = i+1; j <= N; j++) {
                        cls[cls_idx] += "-" + to_string(r) + to_string(c) + to_string(i) + " ";        
                        cls[cls_idx] += "-" + to_string(r) + to_string(c) + to_string(j) + " ";        
                        cls[cls_idx++] += "0";
                    }
                } 
            }

        }
    }



    output_sat();
    unsol_sat.close();
    unsol_puz.close();

    // Call minisat solver
    system("./minisat unsol_sat sol_sat");
    // execlp("./minisat", unsol_sat_filename, sol_sat_filename, NULL);
    // exec
    

    sol_sat.open(sol_sat_filename, ios::in);
    sol_puz.open(argv[2], ios::out);

    // Decode from CNF 
    int val;
    string res;

    ss << sol_sat.rdbuf();
    ss >> res;
    if (res == "SAT") {
        while (ss >> val) {
            if (val > 0) {
                cout << val << " ";
            }
        }
    }

    return 0;

}

void make_cls(string opt, int idx, int num)
{
    if (opt == "ROW") {
        // At least one 
        for (int i = 0; i < N; i++) {
            if (G[idx][i] == 0) {
                cls[cls_idx] += to_string(idx) + to_string(i) + to_string(num) + " "; 
            }
        } 

        cls[cls_idx++] += "0";
            
        // At most one
        for (int i = 0; i < N; i++) {
            for (int j = i+1; j < N; j++) {
                if (G[idx][i] == 0 && G[idx][j] == 0) {
                    cls[cls_idx] += "-" + to_string(idx) + to_string(i) + to_string(num) + " ";
                    cls[cls_idx] += "-" + to_string(idx) + to_string(j) + to_string(num) + " ";
                    cls[cls_idx++] += "0";
                }
           }
        }
    } 

    if (opt == "COL") {
        // At least one 
        for (int i = 0; i < N; i++) {
            if (G[i][idx] == 0) {
                cls[cls_idx] += to_string(i) + to_string(idx) + to_string(num) + " "; 
            }
        } 

        cls[cls_idx++] += "0";

        // At most one
        for (int i = 0; i < N; i++) {
            for (int j = i+1; j < N; j++) {
                if (G[i][idx] == 0 && G[j][idx] == 0) {
                    cls[cls_idx] += "-" + to_string(i) + to_string(idx) + to_string(num) + " ";
                    cls[cls_idx] += "-" + to_string(j) + to_string(idx) + to_string(num) + " ";
                    cls[cls_idx++] += "0";
                }
            }
        }
    } 

    if (opt == "BLOC") {
        int r_start = floor(idx/n) * n;
        int c_start = (idx%n) * n;
        int rc_ls_idx = 0;
        string *rc_ls = new string[N*(N-1)/2];
        
        // At least one
        for (int r = r_start; r < r_start + n; r++) {
            for (int c = c_start; c < c_start + n; c++) {
                if (G[r][c] == 0) {
					cls[cls_idx] += to_string(r) + to_string(c) + to_string(num) + " ";
					rc_ls[rc_ls_idx++] = to_string(r) + to_string(c);
                }
            } 
        }

        cls[cls_idx++] += " 0";

        // At most one
        for (int i = 0; i < rc_ls_idx; i++) {
            for (int j = i+1; j < rc_ls_idx; j++) {
                string p1 = rc_ls[i];
                string p2 = rc_ls[j];


                // if (G[r1][c1] == 0 && G[r2][c2] == 0) {
                    cls[cls_idx] += "-" + p1 + to_string(num) + " "; 
                    cls[cls_idx] += "-" + p2 + to_string(num) + " "; 
                    cls[cls_idx++] += "0";
                // }

            } 
        }

    }
}

int get_rcn_code(int r, int c, int n)
{
    return r*N*N + c*N + n;
}

int get_num_pos(string opt, int idx, int num)
{

    // Search row
    if (opt == "ROW") {
        for (int i = 0; i < N; i++) {
            if (G[idx][i] == num) {
                return i;
            }
        }
    }

    // Search col
    if (opt == "COL") {
        for (int i = 0; i < N; i++) {
            if (G[i][idx] == num) {
                return i;
            }
        }
    }

    if (opt == "BLOC") {
        int r_start = floor(idx/n) * n;
        int c_start = (idx%n) * n;

        for (int i = r_start; i < r_start + n; i++) {
            for (int j = c_start; j < c_start + n; j++) {
                if (G[i][j] == num) {
                    return 1;
                }
            } 
        }

    }

    return -1;
}

void output_sat()
{
    unsol_sat << "p cnf " << N*N*N << " " << cls_idx << endl;

    for (int i = 0; i < cls_idx; i++) {
        unsol_sat  << cls[i] << endl;
    }

}

void print()
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << G[i][j] << " "; 
        }
        cout << endl;
    }
}


