#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "parser.h"

using namespace std;

bool DPLL();
void make_implication(int* &XI, int &XI_idx);
bool check_success();
bool check_fail();
void init();
void update(int x);
int get_MOM_literal();
bool is_X_in_cl(int idx, int val);
int abs(int n);

/* Const Variable */
static const int MOM_THRESHHOLD = 3;

/* Global Variable */
vector<vector<int> > cls; 
fstream sol;
int cls_size;
int X_max;
int* X;
int* cl_val;
int* cl_len;
int* cl_omega;

int main(int argc, char **argv)
{
    string in_fn  = argv[1];
    string out_fn = in_fn.substr(0, in_fn.find(".")) + ".sat";
    /* string out_fn = in_fn.substr(in_fn.find_last_of("/")+1, in_fn.find(".") - in_fn.find_last_of("/")) + "sat"; */
    
    parse_DIMACS_CNF(cls, X_max, in_fn.c_str());
    /* parse_DIMACS_CNF(cls, X_max, "./benchmarks/SAT/sanity/sanity2.cnf"); */
    cls_size = cls.size();


    X = new int[X_max+1];
    cl_val = new int[cls_size+1];
    cl_len = new int[cls_size+1];
    cl_omega = new int[cls_size+1];

    init();

    sol.open(out_fn.c_str(), ios::out);
    if (DPLL()) {
        sol << "s SATISFIABLE" << endl;
        sol << "v ";

        for (int i = 1; i < X_max+1; i++) {
            if (X[i] == 0) sol << "-";
            sol << i << " ";
        }
        sol << " 0";

    } else {
        sol << "s UNSATISFIABLE" << endl;
    }
    sol.close();

    return 0;
}


void output_X()
{
    for (int i = 1; i < X_max+1; i++) {
        cout << "x" << i << ": " << X[i] << " ";
    }
    cout << endl;

    for (int i = 0; i < cls_size; i++) {
        cout << setw(5) << "cl" << i << " "; 

        for (int j = 0; j < cl_len[i]; j++) {
            cout << setw(3) << cls[i][j] << " " ;
        }

        cout << " val: " << cl_val[i] << endl;
    }
}

bool DPLL()
{

    /* output_X(); */

    int XI_idx_old = -1;
    int XI_idx = 0;
    int* XI = new int[X_max];

    while(XI_idx != XI_idx_old) {
        /* cout << "Old: " << XI_idx_old << endl; */
        /* cout << "New: " << XI_idx << endl; */
        XI_idx_old = XI_idx;
        make_implication(XI, XI_idx);
    }


    if (check_success()) 
        return true;

    if (check_fail()) {
        /* cout << "## FAIl FAIL FAIL" << endl; */

        /* cout << "## RESTORING "; */
        for (int i = 1; i < XI[0]; i++) {
            /* cout << XI[i] << " "; */
            X[XI[i]] = -1;


        }
        /* cout << endl; */

        return false;
    }



    int x = get_MOM_literal();
    X[x] = 0;
    /* cout << "## ASSIGN x" << x << " AS 0" << endl; */
    update(x*-1);

    if (DPLL())
        return true;
    else {
        X[x] = 1;
        /* cout << "## ASSIGN x" << x << " AS 1" << endl; */
        update(x*1);
        return DPLL();
    }

}

void init()
{
    for (int i = 0; i < cls_size+1; i++) {
        cl_val[i] = -1;
        cl_len[i] = cls[i].size();
        cl_omega[i] = cls[i].size();
    }


    for (int i = 0; i < X_max+1; i++) {
        X[i] = -1;
    }
}

void update(int xx)
{
    /* for (int i = 0; i < cls_size; i++) { */

    /*     // Update cl containing x */
    /*     if (is_X_in_cl(i, x)) { */
    /*         cl_omega[i]--; */
    /*         cl_val[i] = 1; */
    /*     } */

    /*     // Update cl containing -x */
    /*     if (is_X_in_cl(i, x*-1)) { */
    /*         cl_omega[i]--; */
    /*         if (cl_omega[i] == 0 && cl_val[i] != 1) { */
    /*             cl_val[i] = 0; */
    /*         } */
    /*     } */

    /* } */

    for (int i = 0; i < cls_size; i++) {
        for (int j = 0; j < cl_len[i]; j++) {
            int x = cls[i][j];
            int idx = abs(x);

            if ((X[idx] == 0 && x < 0) || (X[idx] == 1 && x > 0)) {
                cl_val[i] = 1;
                break;
            }
            else if (X[idx] == -1) {
                cl_val[i] = -1;
                break;
            }
            else {
                cl_val[i] = 0;
            }
        }
    }
}

int get_MOM_literal()
{
    int* cnt = new int[cls_size+1];

    // Init
    for (int i = 0; i < cls_size+1; i++) {
        cnt[i] = 0;
    }

    // Count for each X
    for (int i = 0; i < cls_size; i++) {

        // Check threshhold
        if (cl_omega[i] <= MOM_THRESHHOLD) {
            for (int j = 0; j < cl_len[i]; j++) {
                int x = abs(cls[i][j]);
                /* cout << x << " "; */

                // Check for +x only and check unassigned
                if (X[x] == -1) {
                    cnt[x]++; 
                }
            } 
        }

        /* cout << endl; */
    }

    // Select most frequent one
    int idx = 1, max = 0;
    for (int i = 1; i < cls_size+1; i++) {
        if (cnt[i] > max) {
            max = cnt[i]; 
            idx = i;
        }
    }

    delete [] cnt;

    return idx;
}

bool is_X_in_cl(int idx, int val)
{
    for (int i = 0; i < cl_len[idx]; i++) {
        if (cls[idx][i] == val)
            return true;
    }

    return false;
}


void make_implication(int* &XI, int &XI_idx)
{

    for (int i = 0; i < cls_size; i++) {
        if (cl_omega[i] == 1 && cl_val[i] == 0) {

            for (int j = 0; j < cl_len[i]; j++) {
                int x = cls[i][j];

                // If unassigned
                if (X[abs(x)] == -1) {
                    
                    XI[XI_idx++] = abs(x);

                    if (x < 0) {
                        X[abs(x)] = 0;
                        cout << "## IMPLY x" << abs(x) << " AS 0 AT CLS" << i << endl;
                        update(abs(x)*-1);
                    } 
                    else {
                        X[abs(x)] = 1;
                        cout << "## IMPLY x" << abs(x) << " AS 1 AT CLA" << i << endl;
                        update(abs(x)*1);
                    }
                }
            }
        }
    }

    XI[0] = XI_idx;
}

bool check_success() 
{
    for (int i = 0; i < cls_size; i++) {

        // if have 0(false) -1(unassign) -> no success
        if (cl_val[i] != 1) 
            return false;
    }

    return true;
}

bool check_fail()
{
    for (int i = 0; i < cls_size; i++) {
        
        // Have 0, fail
        if (cl_val[i] == 0) {
            return true;
        }
    }

    return false;
}

int abs(int n)
{
    return n > 0 ? n: -n;
}
