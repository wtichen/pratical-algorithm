#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define sunny 0
#define foggy 1
#define rainy 2

#define yes 1
#define no  0

using namespace std;

void init(vector<double>&, vector<vector<double> >&, vector<vector<double> >&);
void parse_file(string, int&, vector<string>&, vector<string>&);
void output_file(string, float, vector<string>&);
pair<double, int> get_p_and_weather(double, double, double);

int main(int argc, char** argv)
{

    // Variable
    int day;
    double c_sunny, c_foggy, c_rainy; 
    double l_sunny, l_foggy, l_rainy; 
    string in_fn(argv[1]);
    string out_fn(argv[2]);
    vector<string> guess_weather;
    vector<string> actual_weather;
    vector<string> wear_coat;
    vector<double> S;
    vector<vector<double> > W;
    vector<vector<double> > C;

    vector<vector<int> > P;

    init(S, W, C);

    parse_file(in_fn, day, actual_weather, wear_coat); 
    
    P.resize(3, vector<int>(day, -1));

    for (int d = 0; d < day; d++) 
    {
        int is_yes = (wear_coat[d] == "yes");
        pair<double, int> p1, p2, p3;


        if (d == 0) 
        {
            c_sunny = S[sunny] + C[sunny][is_yes];
            c_foggy = S[foggy] + C[foggy][is_yes];
            c_rainy = S[rainy] + C[rainy][is_yes];
            
            P[0][d] = sunny;
            P[1][d] = foggy;
            P[2][d] = rainy;
        } 
        else 
        {
            l_sunny = c_sunny;
            l_foggy = c_foggy;
            l_rainy = c_rainy;

            p1 = get_p_and_weather(
                        l_sunny + W[sunny][sunny], 
                        l_foggy + W[foggy][sunny], 
                        l_rainy + W[rainy][sunny]
                    );

            p2 = get_p_and_weather(
                        l_sunny + W[sunny][foggy], 
                        l_foggy + W[foggy][foggy], 
                        l_rainy + W[rainy][foggy]
                    );

            p3 = get_p_and_weather(
                        l_sunny + W[sunny][rainy], 
                        l_foggy + W[foggy][rainy], 
                        l_rainy + W[rainy][rainy]
                    );

            P[0][d] = p1.second;
            P[1][d] = p2.second;
            P[2][d] = p3.second;

            c_sunny = p1.first + C[sunny][is_yes];
            c_foggy = p2.first + C[foggy][is_yes];
            c_rainy = p3.first + C[rainy][is_yes];

        }
    }



    int idx, cnt = 0;
    double _max = max(max(c_sunny, c_foggy), c_rainy);

    if (_max == c_sunny) 
        idx = sunny;
    else if (_max == c_foggy)
        idx = foggy;
    else 
        idx = rainy;

    // Backtrack and reverse
    for (int d = day-1; d >= 0; d--) 
    {
        if (idx == sunny)
            guess_weather.push_back("sunny");
        else if (idx == foggy)
            guess_weather.push_back("foggy");
        else
            guess_weather.push_back("rainy");

        idx = P[idx][d];
    }
    reverse(guess_weather.begin(), guess_weather.end());

    // Caculate rate
    for (int i = 0; i < guess_weather.size(); i++)
    {
        if (guess_weather[i] == actual_weather[i])  cnt++;
    }

    // File output
    output_file(out_fn, (float)cnt/day, guess_weather);

    return 0;
}

pair<double, int> get_p_and_weather(double s, double f, double r)
{
    pair<double, int> p;

    p.first = max(max(s, f), r);

    if (p.first == s) 
    {
        p.second = sunny;
    }
    else if (p.first == f)
    {
        p.second = foggy;
    }
    else
    {
        p.second = rainy;
    }

    return p;
}

void init(vector<double>& S, vector<vector<double> >& W, vector<vector<double> >& C)
{
    S.resize(3);
    W.resize(3, vector<double>(3, 0.0));
    C.resize(3, vector<double>(2, 0.0));

    S[sunny] = log(0.50);
    S[foggy] = log(0.25);
    S[rainy] = log(0.25);

    W[sunny][sunny] = log(0.80);
    W[sunny][foggy] = log(0.15);
    W[sunny][rainy] = log(0.05);

    W[foggy][sunny] = log(0.20);
    W[foggy][foggy] = log(0.50);
    W[foggy][rainy] = log(0.30);

    W[rainy][sunny] = log(0.20);
    W[rainy][foggy] = log(0.20);
    W[rainy][rainy] = log(0.60);

    C[sunny][no]  = log(0.9);
    C[sunny][yes] = log(0.1);

    C[foggy][no]  = log(0.7);
    C[foggy][yes] = log(0.3);

    C[rainy][no]  = log(0.2);
    C[rainy][yes] = log(0.8);

}

void parse_file(string fn, int& d, vector<string>& w, vector<string>& c)
{

    string line;
    fstream fs;
     
    fs.open(fn.c_str(), fstream::in);

    // Get day num and remove empty line
    fs >> d ;
    getline(fs, line);

    // Get actual weather and yes|no
    while (getline(fs, line)) {

        string item;
        stringstream ss(line);

        getline(ss, item, ',');
        w.push_back(item);

        getline(ss, item, ',');
        c.push_back(item);
        
    }

    fs.close();

}

void output_file(string fn, float r, vector<string>& v)
{
    fstream fs;
    
    fs.open(fn.c_str(), fstream::out);

    fs << r << endl;

    for (int i = 0; i < v.size(); i++) {
        fs << v[i] << endl;
    }

}

double find_max(double a, double b, double c)
{
    return max(max(a, b), c);
}

double find_min(double a, double b, double c)
{
    return min(min(a, b), c);
}
