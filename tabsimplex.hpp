/************************************************
*                                               *
* License: Apache License 2.0                   *
* Author: Dario Ostuni <dario.ostuni@gmail.com> *
*                                               *
************************************************/

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

namespace ostuni
{
using namespace std;

static pair<double, vector<double>> tabsimplex(vector<vector<double>>& tableau)
{
    for(size_t i = 0; i < tableau.size() - 1; i++)
    {
        assert(tableau[i].size() == tableau[i + 1].size());
    }
    vector<size_t> base_variables(tableau.size() - 1, -1);
    for(size_t i = 0; i < tableau[0].size(); i++)
    {
        size_t ones = 0;
        size_t pos = 0;
        bool ok = true;
        for(size_t j = 1; j < tableau.size(); j++)
        {
            if(tableau[j][i] == 1.0)
            {
                ones++;
                pos = j - 1;
            }
            else if(tableau[j][i] != 0.0)
            {
                ok = false;
                break;
            }
        }
        if(!ok)
            continue;
        if(ones == 1)
            base_variables[pos] = i;
    }
    while(true)
    {
        auto it = find_if(tableau[0].begin(), tableau[0].end() - 1, [](const double& x) { return x < 0.0; });
        if(it == tableau[0].end() - 1)
            break;
        size_t entering_var = it - tableau[0].begin();
        bool at_least_one = false;
        double min_fraction = INFINITY;
        size_t leaving_var = size_t(-1);
        size_t row_pivot = size_t(-1);
        for(size_t i = 1; i < tableau.size(); i++)
        {
            if(tableau[i][entering_var] <= 0.0)
                continue;
            assert(tableau[i].back() >= 0.0);
            if(tableau[i].back() / tableau[i][entering_var] < min_fraction ||
               (tableau[i].back() / tableau[i][entering_var] <= min_fraction && base_variables[i - 1] < leaving_var))
            {
                at_least_one = true;
                min_fraction = tableau[i].back() / tableau[i][entering_var];
                leaving_var = base_variables[i - 1];
                row_pivot = i - 1;
            }
        }
        if(!at_least_one)
            assert(!"Indefinite problem!");
        double scaling_factor = tableau[row_pivot + 1][entering_var];
        for(size_t i = 0; i < tableau[row_pivot + 1].size(); i++)
        {
            tableau[row_pivot + 1][i] /= scaling_factor;
        }
        for(size_t i = 0; i < tableau.size(); i++)
        {
            if(i == row_pivot + 1)
                continue;
            scaling_factor = tableau[i][entering_var];
            for(size_t j = 0; j < tableau[i].size(); j++)
            {
                tableau[i][j] = -scaling_factor * tableau[row_pivot + 1][j] + tableau[i][j];
            }
        }
        base_variables[row_pivot] = entering_var;
    }
    double value = -tableau[0].back();
    vector<double> vars(tableau[0].size() - 1);
    for(size_t i = 0; i < base_variables.size(); i++)
    {
        vars[base_variables[i]] = tableau[i + 1].back();
    }
    return make_pair(value, vars);
}
}
