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
#include <iostream>
#include <tuple>
#include <vector>

namespace ostuni
{
using namespace std;

template <typename T>
static ostream& operator<<(ostream& o, const vector<T>& v)
{
    o << "[";
    for(size_t i = 0; i < v.size(); i++)
    {
        o << v[i];
        if(i != v.size() - 1)
            o << ", ";
    }
    o << "]";
    return o;
}

static tuple<double, vector<double>, vector<size_t>> tabsimplex(vector<vector<double>>& tableau)
{
    for(size_t i = 0; i < tableau.size() - 1; i++)
    {
        assert(tableau[i].size() == tableau[i + 1].size());
    }
    for(size_t i = 1; i < tableau.size(); i++)
    {
        if(tableau[i].back() < 0.0)
        {
            for(size_t j = 0; j < tableau[i].size(); j++)
                tableau[i][j] *= -1.0;
        }
    }
    vector<size_t> base_variables(tableau.size() - 1, -1);
    for(size_t i = 0; i < tableau[0].size() - 1; i++)
    {
        size_t ones = 0;
        size_t pos = 0;
        bool ok = true;
        for(size_t j = 0; j < tableau.size(); j++)
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
    if(count(base_variables.begin(), base_variables.end(), -1))
    {
        size_t vars_to_add = tableau.size() - 1;
        vector<double> add_gradient(vars_to_add + tableau[0].size());
        vector<double> backup_gradient = tableau[0];
        for(size_t i = 0; i < vars_to_add; i++)
        {
            add_gradient[i + tableau[0].size() - 1] = 1;
        }
        tableau[0] = add_gradient;
        for(size_t i = 1; i < tableau.size(); i++)
        {
            vector<double> add_vars(vars_to_add);
            add_vars[i - 1] = 1;
            tableau[i].insert(tableau[i].end() - 1, add_vars.begin(), add_vars.end());
        }
        for(size_t i = 0; i < tableau[0].size(); i++)
        {
            double sum = 0;
            for(size_t j = 1; j < tableau.size(); j++)
            {
                sum += tableau[j][i];
            }
            tableau[0][i] -= sum;
        }
        auto ret = tabsimplex(tableau);
        if(get<0>(ret) != 0.0)
            assert(!"Cannot find a valid starting point");
        base_variables = get<2>(ret);
        tableau[0] = backup_gradient;
        if(any_of(base_variables.begin(), base_variables.end(), [&backup_gradient](const size_t& x) {
               return x >= backup_gradient.size() - 1;
           }))
        {
            for(int k = 0; k < (int)base_variables.size(); k++)
            {
                if(base_variables[k] < backup_gradient.size() - 1)
                    continue;
                bool all_zeroes = true;
                size_t first_nonzero;
                for(size_t j = 0; j < tableau[k + 1].size() - 1 - vars_to_add; j++)
                {
                    if(tableau[k + 1][j])
                    {
                        all_zeroes = false;
                        first_nonzero = j;
                        break;
                    }
                }
                if(all_zeroes)
                {
                    base_variables.erase(base_variables.begin() + k);
                    tableau.erase(tableau.begin() + k + 1);
                    k--;
                    continue;
                }
                size_t row_pivot = k;
                size_t entering_var = first_nonzero;
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
        }
        for(size_t i = 0; i < tableau.size(); i++)
        {
            tableau[i].erase(tableau[i].end() - 1 - vars_to_add, tableau[i].end() - 1);
        }
        tableau[0] = backup_gradient;
        for(size_t i = 0; i < base_variables.size(); i++)
        {
            double scale_factor = tableau[0][base_variables[i]];
            for(size_t j = 0; j < tableau[0].size(); j++)
            {
                tableau[0][j] -= scale_factor * tableau[i + 1][j];
            }
        }
    }
    size_t cnt = 0;
    while(true)
    {
#ifdef OSTUNI_DEBUG
        cout << "Iteration #" << cnt++ << endl;
        cout << "Value: " << -tableau[0].back() << endl;
        cout << "CCR: [";
        for(size_t i = 0; i < tableau[0].size() - 1; i++)
        {
            cout << tableau[0][i];
            if(i != tableau[0].size() - 2)
                cout << ", ";
        }
        cout << "]" << endl;
        cout << "Variables: [";
        vector<double> tmp_vars(tableau[0].size() - 1);
        for(size_t i = 0; i < base_variables.size(); i++)
        {
            tmp_vars[base_variables[i]] = tableau[i + 1].back();
        }
        for(size_t i = 0; i < tmp_vars.size(); i++)
        {
            cout << tmp_vars[i];
            if(i != tmp_vars.size() - 1)
                cout << ", ";
        }
        cout << "]" << endl;
        cout << endl;
#endif
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
    return make_tuple(value, vars, base_variables);
}

static void gausselim(vector<vector<double>>& tableau)
{
    for(size_t i = 1; i < tableau.size(); i++)
    {
        for(size_t j = i + 1; j < tableau.size(); j++)
        {
            double scale_factor = tableau[j][i - 1] / tableau[i][i - 1];
            if(isnan(scale_factor) || isinf(scale_factor))
                continue;
            for(size_t k = i - 1; k < tableau[i].size(); k++)
            {
                tableau[j][k] -= scale_factor * tableau[i][k];
            }
        }
    }
    for(size_t i = 1; i < tableau.size(); i++)
    {
        if(count(tableau[i].begin(), tableau[i].end(), 0) == tableau[i].size())
        {
            tableau.erase(tableau.begin() + i);
            i--;
        }
    }
}
}
