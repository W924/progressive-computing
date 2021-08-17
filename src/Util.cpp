#include "Util.h"
#include <vector>
#include <string>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace std;


vector<string> split(string str, string pattern)
{
    string::size_type pos;
    vector<string> result;
    str += pattern;
    int size = str.size();

    for (int i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if (pos < size)
        {
            string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}


vector<string> split2(string& s, const string& delim)
{   
    vector<string> ret;

    size_t last = 0;
    size_t index = s.find_first_of(delim, last);

    while (index != string::npos) {
        ret.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }

    if (index - last > 0) {
        ret.push_back(s.substr(last, index - last));
    }

    return ret;
}


double norm(const vector<double>& x) {
    double val = 0;
    for (size_t i = 0; i < x.size(); i++) {
        val += x[i] * x[i];
    }
    return sqrt(val);
}


bool isDateInRange(int start_year, int start_month, int start_day, int end_year, int end_month, int end_day, int year, int month, int day) {
    if (start_year == end_year) {
        if (start_month == end_month) {
            if (year == start_year && month == start_month && day >= start_day && day <= end_day) {
                return true;
            }
        }
        else if (start_month < end_month) {
            if (year == start_year) {
                if (month == start_month) {
                    if (day >= start_day) {
                        return true;
                    }
                }
                else if (month > start_month && month < end_month) {
                    return true;
                }
                else if (month == end_month) {
                    if (day <= end_day) {
                        return true;
                    }
                }
            }
        }
    }
    else if (start_year < end_year) {
        if (year == start_year) {
            if (month == start_month && day >= start_day || month > start_month) {
                return true;
            }
        }
        else if (year > start_year && year < end_year) {
            return true;
        }
        else if (year == end_year) {
            if (month == end_month && day <= end_day || month < end_month) {
                return true;
            }
        }
    }
    return false;
}


double vector_average(std::vector<double>& vec) {
    double sum = 0;
    for (int i = 0; i < vec.size(); i++) {
        sum += vec[i];
    }
    return sum / vec.size();
}