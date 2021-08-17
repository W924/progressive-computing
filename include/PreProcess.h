#pragma once
#include <string>
#include <vector>

void HT_Sensor_sample_average(std::string inFileName, int sensor_num, int d);

void wilt_transfer_csv_to_txt(std::string inFileName);

void network_precision(std::string inFileName, std::string outfileName);

void Air_preprocess(std::string inFileName);

void beach_empty_record(std::string inFileName);