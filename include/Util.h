#pragma once
#include <vector>
#include <string>


std::vector<std::string> split(std::string str, std::string pattern);


std::vector<std::string> split2(std::string& s, const std::string& delim);


double norm(const std::vector<double>& x);


bool isDateInRange(int start_year, int start_month, int start_day, int end_year, int end_month, int end_day, int year, int month, int day);


double vector_average(std::vector<double>& vec);