#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#include "parser.h"


using namespace std;

bool DPLL();
bool CheckSuccess();
bool CheckFailure();
bool CheckUnitClause();
void BCP();
void Init();
void AssignX(int index, int val, int c_idx);
void UnassignX(int lvl);
void UnAssignLitAtLevel(int lvl);
void Backtrack(int lvl);
void Update2Literal(int x_idx, int c_idx);
void ConflictLearning(int c_idx);

int GetHeuristicLiteral();
int GetLitValue(int lit);
pair<int, int> GetLitDecisionLevel(int lit);
int GetLitAntecedent(int lit);
int GetClConflictStatus(vector<int>);
vector<int> GetFirstUIP(int c_idx);
vector<int> Resolve(vector<int>, vector<int>, int lit);

void OutputX();
void Output2Lit();
void OutputPN();
void OutputAsgn();
void OutputAntc();
void OutputVec(vector<int>);
void OutputBreak(string);
void OutputCls();

/* Inmutable Variables */
vector<vector<int> > cls; 
vector<vector<int> > pw;
vector<vector<int> > nw;
int x_num;

string in_filename;
string out_filename;

/* Mutable Variables */
vector<int> x_val;
vector<vector<int> > _2lit; 

vector<vector<int> > _asgn;
vector<vector<int> > _antc;
bool is_backtracking;
int cur_lvl;
int back_lvl;
int c_con_idx;


int main(int argc, char **argv)
{

    // TODO: Result should under same dir
    in_filename  = argv[1];
    /* in_filename = "./benchmarks/SAT/sanity/sanity2.cnf"; */
    out_filename = "result.sat";

    // Init
    Init();

    cout << "x_num: " << x_num << endl;

    while (true) {
        cur_lvl = 0;
        back_lvl = -1;
        bool res  = DPLL();
        if (!res && back_lvl == 0) {
            UnAssignLitAtLevel(cur_lvl);
            cout << "Need restrat" << endl;
            continue;
        }
        else if (res) {
            cout << "Res: Has solution" << endl;
            for (int i = 1; i <= x_num; i++) {
                if (x_val[i] == 0) 
                    cout << i*-1 << " ";
                else
                    cout << i << " ";
            }
            return 1;
        }
        else {
            cout << "Res: GGGGGGGGGGGG" << endl;
            return 0;
        }
    }

    /* // DPLL */
    /* // TODO: Write output to file */
    /* if (DPLL()) { */
    /*     cout << "Res: Has solution" << endl; */
    /*     OutputX(); */
    /* } */
    /* else { */
    /*     cout << "Res: GGGGGGGGGGGG" << endl; */
    /* } */

    return 0;
}


bool DPLL()
{

    // Do BCP
    BCP();

    // Check success or fail
    if (CheckSuccess()) {
        return true;
    }    
    if (CheckFailure()) {
        cout << "FAIL" << endl;

        OutputAsgn();
        OutputX();

        if (cur_lvl != 0) 
            ConflictLearning(c_con_idx); 

        return false;
    }

    // No result, make decision and go go further lvl
    cur_lvl++;

    int idx = GetHeuristicLiteral();    
    int val = 1;

    while (true) {
        cout << " # DECIDE x" << idx << " as " << val << " dur " << cur_lvl << endl;
        AssignX(idx, val, -1);
        OutputAsgn();
        OutputX();
        if (DPLL()) {
            return true;
        }
        else {
            /* cout << "cur_lvl: " << cur_lvl << endl; */
            /* cout << "back_lvl: " << back_lvl << endl; */
            UnAssignLitAtLevel(cur_lvl);
            
            // Check if reach distination level
            if (cur_lvl > back_lvl) {
                cur_lvl--;
                return false;
            }
            else {
                back_lvl = -1;
                cout << "WILL ARRIVED" << endl;
                /* val = (val == 0)? 1: 0; */
            }
        }
    }

}

void UnAssignLitAtLevel(int lvl)
{
    // Set x_val to -1
    for (int i = 0; i < _asgn[lvl].size(); i++) {
        int idx = _asgn[lvl][i]; 
        x_val[idx] = -1;
    }

    // Clean assignment and antc
    _asgn[cur_lvl].clear();
    _antc[cur_lvl].clear();
}

void AssignX(int x_lit, int val, int c_idx)
{
    int x_idx = abs(x_lit);

    // Assign 
    x_val[x_idx] = val;

    // Record
    _asgn[cur_lvl].push_back(x_idx); 
    _antc[cur_lvl].push_back(c_idx);


    /* OutputAsgn(); */
    /* OutputX(); */

    // If is one, iterate neg_watchlist 
    // Else, iterate pos_watchlist
    bool is_one = (val == 1); 
    if (is_one) {
        for (int i = 0; i < nw[x_idx].size(); i++) {
            /* cout << "UPDATE " << x_lit << " at " << nw[x_idx][i] << endl; */
            /* cout << "Idx: " << nw[x_idx][i] << endl; */
            /* cout << _2lit.size() << endl; */
            Update2Literal(x_idx*-1, nw[x_idx][i]);
        }
    }
    else {
        for (int i = 0; i < pw[x_idx].size(); i++) {
            /* cout << "UPDATE " << x_lit << " at " << pw[x_idx][i] << endl; */
            /* cout << "Idx: " << pw[x_idx][i] << endl; */
            /* cout << _2lit.size() << endl; */
            Update2Literal(x_idx, pw[x_idx][i]);
        }
    }

}

// Find if x_lit in the c_idx and try to change
void Update2Literal(int x_lit, int c_idx)
{

    /* OutputBreak("FF"); */
    int x_first_lit  = _2lit[c_idx][0];
    int x_second_lit = _2lit[c_idx][1];
    bool is_first_lit_changed = (x_lit == x_first_lit);

    /* cout << "x_lit " << x_lit << endl; */
    /* cout << "watch " << x_first_lit << " " << x_second_lit << endl; */
    // If not watching
    if (x_lit != x_first_lit && x_lit != x_second_lit) 
        return;


    // Iterate over this clause
    // I. If is non-watching literal
    // II. If is non-0 literal
    // III. Check which lit should bechanged
    /* cout << "Try to change" << endl; */
    /* OutputBreak("GG"); */
    for (int i = 0; i < cls[c_idx].size(); i++) {
        int x_temp_lit = cls[c_idx][i];

        if (x_temp_lit != x_first_lit && x_temp_lit != x_second_lit) {
            /* cout << "x_t val: " << GetLitValue(x_temp_lit); */ 
            if (GetLitValue(x_temp_lit) != 0) {
                if (is_first_lit_changed) {
                    /* cout << "CHANGE from " << _2lit[c_idx][0] << " to "<< x_temp_lit  << endl; */
                    _2lit[c_idx][0] = x_temp_lit;
                }
                else { 
                    /* cout << "CHANGE from " << _2lit[c_idx][1] << " to "<< x_temp_lit  << endl; */
                    _2lit[c_idx][1] = x_temp_lit;
                }
                
                /* Output2Lit(); */
                return;
            }
        }
    }

    /* OutputBreak("HH"); */
    /* Output2Lit(); */
    return;
}
bool CheckSuccess()
{

    // Every clause should have at least a 1
    for (int c_idx = 0; c_idx < _2lit.size(); c_idx++) {
        int first_lit  = _2lit[c_idx][0];
        int second_lit = _2lit[c_idx][1];

        /* cout << "First " << GetLitValue(first_lit) << endl; */
        /* cout << "Secon " << GetLitValue(second_lit) << endl; */
        if ((GetLitValue(first_lit) != 1) && (GetLitValue(second_lit) != 1)) {
            return false;
        }
    }

    return true;

}

bool CheckFailure()
{

    // If a clause has (0, 0) -> fail
    for (int c_idx = 0; c_idx < _2lit.size(); c_idx++) {
        int first_lit  = _2lit[c_idx][0];
        int second_lit = _2lit[c_idx][1];

        if ((GetLitValue(first_lit) == 0) && (GetLitValue(second_lit) == 0)) {
            c_con_idx = c_idx;
            return true;
        }
    }

    return false;

}

bool CheckUnitClause() 
{
    /* Output2Lit(); */
    /* if (cls.size() == _2lit.size()) */
        /* cout << "Tha'ts right"; */

    /* for (int c_idx = _2lit.size() - 1; c_idx >= 0; c_idx--) { */
    for (int c_idx = 0; c_idx < cls.size(); c_idx++) {
        int first_lit  = _2lit[c_idx][0];
        int second_lit = _2lit[c_idx][1];
        int first_idx  = abs(first_lit);
        int second_idx = abs(second_lit);


        /* if (c_idx == cls.size()-1) { */
        /*     cout << "-----Last clause idx " << cls.size()-1 << endl; */
        /*     OutputVec(cls[_2lit.size()-1]); */
        /*     cout << "-----X" << endl; */
        /*     OutputX(); */
        /* } */

        /* if (c_idx == cls.size() - 1) { */
        /*     cout << "Fir " << first_lit << " val " << GetLitValue(first_lit) << endl; */
        /*     cout << "Sec " << second_lit << " val " << GetLitValue(second_lit) << endl; */
        /* } */

        // Unit clause
        if (second_lit == 0 && GetLitValue(first_lit) == -1) {
            cout << "# IMPLY U x" << first_idx << " as " << !(first_lit < 0)  << " dur " << cur_lvl << endl;
            (first_lit < 0)? AssignX(first_idx, 0, c_idx): AssignX(first_idx, 1, c_idx);
            return true;
        }

        // First lit unassign
        if (GetLitValue(first_lit) == -1 && GetLitValue(second_lit) == 0 ) {
            cout << "# IMPLY P x" << first_idx << " as " << !(first_lit < 0)  << " dur " << cur_lvl << endl;
            cout << "[" << c_idx << "] ";
            OutputVec(cls[c_idx]);
            (first_lit < 0)? AssignX(first_idx, 0, c_idx): AssignX(first_idx, 1, c_idx);
            /* AssignX(first_lit, (first_lit > 0)); */
            return true;
        }

        // Second lit unassign
        if (GetLitValue(first_lit) == 0  && GetLitValue(second_lit) == -1) {
            cout << "# IMPLY P x" << second_idx <<" as " << !(second_lit < 0) << " dur " << cur_lvl  <<  endl;
            cout << "[" << c_idx << "] ";
            OutputVec(cls[c_idx]);
            (second_lit < 0)? AssignX(second_idx, 0, c_idx): AssignX(second_idx, 1, c_idx);
            /* AssignX(second_lit, (second_lit > 0)); */
            return true;
        }

    }

    return false;

}

int GetHeuristicLiteral() 
{
    for (int x = 1; x <= x_num; x++) {
        if (x_val[x] == -1) {
            return x;
        }
    }
    
    return -1;
}

int GetLitValue(int lit)
{
    int val = x_val[abs(lit)];

    if ((val == 0 && lit < 0) || (val == 1 && lit > 0)) {
        return 1;
    }
    else if ((val == 1 && lit < 0) || (val == 0 && lit > 0)) {
        return 0;
    }
    else {
        return -1;
    }
}

void BCP() 
{

    // Recursive searching for IMP clause
    // Check whether literal is 0
    while (CheckUnitClause()) {}
}

void ConflictLearning(int c_idx) 
{
    // Get firstUIP
    if (cls.size() > 851) {
        int first_lit  = _2lit[850][0];
        int second_lit = _2lit[850][1];

        for (int i = 0; i < cls[850].size(); i++) {
            int lit = cls[850][i];
            cout << lit << "(" << GetLitValue(lit) << ")" << " ";
        }
        cout << endl;
    }
    cout << "-----Conflict ";
    OutputVec(cls[c_idx]);
    vector<int> v = GetFirstUIP(c_idx);

    // Add new learning clause
    cls.push_back(v);
    _2lit.push_back(vector<int>(2));
    OutputAsgn();
    cout << "-----Learned c_idx " << cls.size()-1 << " ";
    OutputVec(v);
    /* OutputCls(); */

    // Iterate
    int w_lit = 0;
    int w_lit2 = 0;
    int w_lit2_j = -1;
    for (int i = 0; i < v.size(); i++) {
        int lit = v[i];
        int idx = abs(lit);
        int lvl = GetLitDecisionLevel(v[i]).first;
        int j = GetLitDecisionLevel(v[i]).second;

        // Update pw nw
        if (lit > 0) {
            pw[idx].push_back(cls.size()-1); 
        }
        else {
            nw[idx].push_back(cls.size()-1); 
        }

        // Calculate return lvl and MUST watch lit
        if (lvl != cur_lvl) {
            /* cout << "lit: " << lit << endl; */
            /* cout << "--back_lvl " << back_lvl << endl; */
            /* cout << "--cur_lvl " << cur_lvl << endl; */
            /* cout << "--lvl " << lvl << endl; */
            if (lvl >= back_lvl) {
                back_lvl = lvl; 
                if (j > w_lit2_j)
                    w_lit2 = lit;
            }
        }
        else {
            w_lit = lit;
            cout << "Should imply " << w_lit << endl;
        }
    }

    // If C - {p} is empty
    if (v.size() == 1) {
        back_lvl = 0;
        _2lit.back()[0] = v[0];
        _2lit.back()[1] = 0;
    }
    else {
        cout << "Watch " << w_lit << " " << w_lit2 << endl;
        _2lit.back()[0] = w_lit;
        _2lit.back()[1] = w_lit2;
    }
    cout << "Shoud back to " << back_lvl << endl;

    return;
}

vector<int> GetFirstUIP(int c_idx)
{
    int lit;
    int cnt = 0;
    vector<int> v = cls[c_idx];

    cout << "Clause----- ";
    OutputVec(v);
    while ((lit = GetClConflictStatus(v)) != -1) {

        cout << "Resolve --- " << lit << endl;
        /* OutputVec(v); */
        OutputVec(cls[GetLitAntecedent(lit)]);

        v = Resolve(v, cls[GetLitAntecedent(lit)], lit); 

        /* if (++cnt == 50) */ 
        /*     break; */

        cout << "Roung " << cnt << " Get Clause----- ";
        OutputVec(v);
    }

    return v;
}

// Check if c_idx have more than 2 same-lvl lit 
// If yes return x_idx of most recent assigned one
// Else return -1
int GetClConflictStatus(vector<int> v)
{
    int cnt = 0;
    int max_j = -1;
    int max_j_lit = -1;
    /* OutputBreak("------Check cl concflict"); */
    /* OutputVec(v); */

    for (int i = 0; i < v.size(); i++) {
        int lit = v[i];
        pair<int, int> p = GetLitDecisionLevel(lit); 

        if (p.first == -1) {
            cout << "HOLY" << endl;
        }

        /* cout << "v[i]: " << setw(3) << v[i] << " i: " << p.first << " j: " << p.second << endl; */
        if (p.first == cur_lvl) {
            cnt++;        
            /* cout << "max_j: " << max_j << endl; */
            /* cout << "lit: " << max_j_lit << endl; */
            /* cout << "----" << endl; */

            if (p.second > max_j) {
                max_j = p.second;
                max_j_lit = lit;
            }
        }
    }

    /* if (cnt >= 2) */
        /* cout << "Pick " << abs(max_j_lit) << endl; */
    return (cnt >= 2)? max_j_lit: -1;
}

// Check which level are the lit been dicided or implied
pair<int, int> GetLitDecisionLevel(int lit)
{
    int idx = abs(lit);

    for (int i = 0; i < _asgn.size(); i++) {
        for (int j = 0; j < _asgn[i].size(); j++) {
            if (idx == _asgn[i][j]) 
                return make_pair(i, j);
        }
    }

    return make_pair(-1, -1);
}

int GetLitAntecedent(int lit)
{
    int idx = abs(lit);

    for (int i = 0; i < _asgn.size(); i++) {
        for (int j = 0; j < _asgn[i].size(); j++) {
            if (idx == _asgn[i][j]) {

                /* cout << "Find antc " << _antc[i][j] << endl; */
                return _antc[i][j];
            } 
        }
    }

    return -1;
}

vector<int> Resolve(vector<int> c, vector<int> a, int lit)
{
    vector<int> v;
    int idx = abs(lit);

    /* cout << "Resolving " << endl; */
    /* cout << "Lit " << lit << endl; */
    /* OutputVec(c); */
    /* OutputVec(a); */
    /* cout << "---1" << endl; */

    for (int i = 0; i < c.size(); i++) {
        int t_lit = c[i];
        int t_idx = abs(t_lit);
        if (t_idx != idx) {
            // For example lit = x1
            // I.   find x1
            // II.  fint -x1
            // III. no x1
            /* if (find(v.begin(), v.end(), t_lit) != v.end()) { */
            /*     continue; */
            /* } */
            /* else if (find(v.begin(), v.end(), t_lit*-1) != v.end()) { */
            /*     v.erase(remove(v.begin(), v.end(), t_lit*-1), v.end()); */

            /* } */
            /* else { */
                v.push_back(t_lit); 
            /* } */
        } 
    }

    for (int i = 0; i < a.size(); i++) {
        int t_lit = a[i];
        int t_idx = abs(t_lit);
        if (t_idx != idx) {
            // For example lit = x1
            // I.   find x1
            // II.  fint -x1
            // III. no x1
            if (find(v.begin(), v.end(), t_lit) != v.end()) {
                continue;
            }
            /* else if (find(v.begin(), v.end(), t_lit*-1) != v.end()) { */
            /*     v.erase(remove(v.begin(), v.end(), t_lit*-1), v.end()); */

            /* } */
            else {
                v.push_back(t_lit); 
            }
        } 
    }

    return v;
}

void Init()
{
    // Parse from source cnf file
    parse_DIMACS_CNF(cls, x_num, in_filename.c_str());

    // Watch first 2 literal for each clause
    _2lit.resize(cls.size(), vector<int>());
    for (int i = 0; i < cls.size(); i++) {
        /* vector<int> v(2); */
        vector<int> v = _2lit[i];

        // Unit clause
        if (cls[i].size() == 1) {
            _2lit[i].push_back(cls[i][0]);
            _2lit[i].push_back(0); 
        }
        else {
            _2lit[i].push_back(cls[i][0]);
            _2lit[i].push_back(cls[i][1]);
        }

        /* _2lit.push_back(v); */
    }

    // Init x value
    x_val.resize(x_num+1, -1);

    // Init nw pw
    pw.resize(x_num+1, vector<int>());
    nw.resize(x_num+1, vector<int>());
    for (int i = 0; i < cls.size() ; i++) {
        for (int j = 0; j < cls[i].size(); j++) {
            int x_idx = abs(cls[i][j]);

            if (cls[i][j] > 0) {
                pw[x_idx].push_back(i);
            }
            else {
                nw[x_idx].push_back(i);
            }

        }
    }

    // Init level start from 1
    _asgn.resize(x_num, vector<int>());
    _antc.resize(x_num, vector<int>());

}

void OutputX()
{
    for (int i = 1; i <= x_num; i++) {
        if (x_val[i] == 0) 
            cout << setw(4) << i*-1 << " ";
        else if (x_val[i] == 1)
            cout << setw(4) << i << " ";
        else 
            cout << setw(4) << "?" << " ";
        

        if (i % 10 == 0) 
            cout << endl;
    }

    /* for (int i = 1; i <= x_num; i++) { */
    /*     cout << setw(3) << x_val[i] << " "; */
    /*     if (i % 10 == 0) */
    /*         cout << endl; */
    /* } */

    cout << endl;
}

void Output2Lit()
{
    for (int i = 0; i < cls.size(); i++) {
        cout << "[" << i << "] " << _2lit[i][0] << " " << _2lit[i][1] << endl;
    }
}

void OutputPN()
{
    cout << "PW-------" << endl;
    for (int i = 1; i <= x_num; i++) {
        cout << "[" << i << "] ";
        for (int j = 0; j < pw[i].size(); j++) {
            cout << pw[i][j] << " ";
        }
        cout << endl;
    }

    cout << "NW-------" << endl;
    for (int i = 1; i <= x_num; i++) {
        cout << "[" << i << "] ";
        for (int j = 0; j < nw[i].size(); j++) {
            cout << nw[i][j] << " ";
        }
        cout << endl;
    }
}

void OutputAsgn()
{
    for (int i = 0; i < cur_lvl+1; i++) {
        cout << "[" << i << "]"; 
        for (int j = 0; j < _asgn[i].size(); j++) {
            /* cout << _asgn[i][j] << "(" << _antc[i][j] << ")" << " "; */
            cout << _asgn[i][j] <<  " ";
            if (j% 10 == 0 && j!= 0) 
                cout << endl;
        }
        cout << endl; 
    }
}

void OutputAntc()
{
    for (int i = 0; i < _antc.size(); i++) {
        cout << "[" << i << "]"; 
        for (int j = 0; j < _antc[i].size(); j++) {
            cout << _antc[i][j] << " ";
        }
        cout << endl; 
    }
}

void OutputVec(vector<int> v)
{
    for (int i = 0; i < v.size(); i++) {
        cout << v[i] << " "; 
    }
    cout << endl;
}

void OutputBreak(string str)
{
    cout << str << endl;
}

void OutputCls()
{
    for (int i = 0; i < cls.size(); i++) {
        cout << "[" << i << "]"; 
        for (int j = 0; j < cls[i].size(); j++) {
            cout << cls[i][j] << " ";
        }
        cout << endl;
    }
}
