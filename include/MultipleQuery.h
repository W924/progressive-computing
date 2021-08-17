#pragma once
#include "Sensor.h"

#include <map>
#include <vector>
#include <string>
#include <set>
#include <fstream>

using namespace std;

class MultipleQuery
{public:

	// 北京空气质量气象站，简单验证渐进计算
	std::map<int, int> cost_trans; // 每个属性需要传输的数据量 / 8，也就是参数 k
	std::map<int, double> cost_proc; // 每个属性的处理代价

	double initial_cost_trans = 105.6; // 唤醒，等ACK，传输报文表头等能量总和，uJ
	double unit_8_cost_trans = 12.9; // sensor每传输 8 字节的能量开销,uJ
	double control_cost_trans = 55.2; // 接收一个控制包需要的能量，uJ
	double transmitter_const = 55.2 / 4725;

	std::map<std::string, std::vector<std::pair<std::string, std::vector<double>>>> air_time_record; // 每个时刻的记录，并包含station的ID
	std::set<std::string> station_id;

	void air_quality_readRecord(std::string inFileName);
	void readAttributeCosts(std::string filePath);

	// 实验六: energy-harvest 6.1

	double RF_tradition(std::string inFileName, int attribute_number);
	double RF_sequential(std::string inFileName, int attribute_number);
	double RF_sel(std::string inFileName, int attribute_number);

	double energy_harvest_tradition(std::string inFileName, double power_min, double power_max, int seed, int attribute_number);
	double energy_harvest_sequential(std::string inFileName, double power_min, double power_max, int seed, int attribute_number);
	double energy_harvest_sel(std::string inFileName, double power_min, double power_max, int seed, int attribute_number);

    double RF_tradition_file(std::string inFileName, std::string attribute_path);
    double RF_sequential_file(std::string inFileName, std::string attribute_path);
    double RF_sel_file(std::string inFileName, std::string attribute_path);

    double energy_harvest_tradition_file(std::string inFileName, double power_min, double power_max, int seed, std::string attribute_path);
    double energy_harvest_sequential_file(std::string inFileName, double power_min, double power_max, int seed, std::string attribute_path);
    double energy_harvest_sel_file(std::string inFileName, double power_min, double power_max, int seed, std::string attribute_path);



	// 实验6.2

	void RF(std::string inFileName, int attribute_number, int k);
	void energy_harvest(std::string inFileName, double power_min, double power_max, int seed, int attribute_number, int k);

    void RF_file(std::string inFileName, std::string attribute_path, int k);
    void energy_harvest_file(std::string inFileName, double power_min, double power_max, int seed, std::string attribute_path, int k);

	double charging(string line, map<string, vector<pair<double, double>>>& station_sensors_energy_power, vector<int> query_attribute, vector<int> query_n, bool isTra);

    vector<vector<vector<int>>> get_params(std::string file_path);

};
