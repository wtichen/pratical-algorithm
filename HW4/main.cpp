#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

double get_path_p(vector<double>&);

int main(void)
{
    double p;
    string cl;

    while (cin >> p >> cl)
    {
        double p_sum = 0.0;
        vector<double> p_path = vector<double>(); 

        if (p == 0.0 && cl == "0") 
            break;

        for (int i = 0; i < int(cl.size()); i++) 
        {
            // If first paratheses, ignre
            if (i == 0 || i == cl.size() - 1)
                continue;

            if (cl[i] == '(')
            {
                if (isalpha(cl[i-2])) 
                    p_path.push_back(1.0-p);
                else
                    p_path.push_back(p);
                    
            }
            else if (cl[i] == ')')
            {
                p_path.pop_back();
            }
            else if (cl[i] == '1')
            {
                if (cl[i+1] == ')') 
                {
                    p_path.push_back(p);
                    p_sum += get_path_p(p_path);
                    p_path.pop_back();

                }
                else
                {
                    p_path.push_back(1.0-p);
                    p_sum += get_path_p(p_path);
                    p_path.pop_back();
                }
            }
        }

        // Output
        cout << fixed;
        cout << setprecision(6);
        cout << p_sum << endl;
    }
    
    return 0;
}


double get_path_p(vector<double>& p_path)
{
    double p_sum = 1.0;

    for (int i = 0; i < int(p_path.size()); i++) 
    {
        p_sum *= p_path[i];
    }

    return p_sum;
}
