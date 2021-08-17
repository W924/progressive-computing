#include "MultipleQuery.h"
#include "Util.h"
#include "ConstDefine.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>
#include <cmath>

using namespace std;

bool cmp(pair<int, double> a, pair<int, double> b) {
	if (a.second != b.second) {
		return a.second > b.second;
	}
	else {
		return a.first < b.first;
	}
}


void MultipleQuery::readAttributeCosts(string filePath) {
	string line;
	ifstream infile;

	infile.open(filePath + "attribute_cost_trans.txt");
	if (!infile.is_open()) {
		cout << "Open attribute_cost_trans.txt fail!" << endl;
	}
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ",");
		cost_trans[atoi(temp_vector[0].c_str())] = atoi(temp_vector[1].c_str());
	}
	infile.close();

	infile.open(filePath + "attribute_cost_proc.txt");
	if (!infile.is_open()) {
		cout << "Open attribute_cost_proc.txt fail!" << endl;
	}
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ",");
		cost_proc[atoi(temp_vector[0].c_str())] = atof(temp_vector[1].c_str());
	}
	infile.close();
}
void MultipleQuery::air_quality_readRecord(string inFileName) {
	ifstream infile;
	infile.open(inFileName);

	string line;
	getline(infile, line);
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ",");
		string time = temp_vector[1];

		string id = temp_vector[0];
		station_id.insert(id);

		vector<double> attribute_value;
		for (int i = 2; i < temp_vector.size(); i++) {
			attribute_value.push_back(atof(temp_vector[i].c_str()));
		}

		if (air_time_record.count(time) == 0) {
			air_time_record[time] = { make_pair(temp_vector[0], attribute_value) };
		}
		else {
			air_time_record[time].push_back(make_pair(temp_vector[0], attribute_value));
		}
	}
	infile.close();
}


// 实验 6.1
double MultipleQuery::RF_tradition(string inFileName, int attribute_number) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;
	map<string, vector<pair<double, double>>> station_sensors_energy_power;

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute;
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		random_query_attributes.push_back(query_attribute);

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
		}
		station_sensors_energy_power[*iter] = sensors_energy_power;
	}

	int count = 0;
	double transmission_cost_total = 0;
	int total_result_number = 0;
	int compution_cost_total = 0;

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
		vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值

		// 循环次数最多不会超过查询属性的个数
		for (int i = 0; i < query_attribute.size(); i++) {
			int temp_attribute = query_attribute[i];
			double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

			double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

			// 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
			double max_charging_time = 0;
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				double energy_now = station_sensors_energy_power[id][temp_attribute].first;
				double power = station_sensors_energy_power[id][temp_attribute].second;
				if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
					double charging_time = (energy_required - energy_now) / power;
					if (charging_time > max_charging_time) max_charging_time = charging_time;
				}
			}

			transmission_cost_total += max_charging_time;
			compution_cost_total += records.size();

			// 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
			for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
				string id = iter->first;
				for (int j = 0; j < iter->second.size(); j++) {
					double power = station_sensors_energy_power[id][j].second;
					// 充电
					station_sensors_energy_power[id][j].first += power * max_charging_time;
				}
			}

			// 工作的sensor消耗电量
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				station_sensors_energy_power[id][temp_attribute].first -= energy_required;
			}

			// 传统没有过滤
		}
		total_result_number += records.size();
		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}

	if (AIR_PRINT_MESSAGE) {
		//cout << "total result number: " << total_result_number << endl;
		cout << "average compution cost: " << (double)compution_cost_total / count << endl;
	}

	return transmission_cost_total / (count);
}
double MultipleQuery::RF_tradition_file(std::string inFileName, std::string attribute_path){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];
    map<string, vector<pair<double, double>>> station_sensors_energy_power;


    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
        }
        station_sensors_energy_power[*iter] = sensors_energy_power;
    }

    int count = 0;
    double transmission_cost_total = 0;
    int total_result_number = 0;
    int compution_cost_total = 0;

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }

    string line;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

        vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
        vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值

        // 循环次数最多不会超过查询属性的个数
        for (int i = 0; i < query_attribute.size(); i++) {
            int temp_attribute = query_attribute[i];
            double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

            double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

            // 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
            double max_charging_time = 0;
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                double energy_now = station_sensors_energy_power[id][temp_attribute].first;
                double power = station_sensors_energy_power[id][temp_attribute].second;
                if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
                    double charging_time = (energy_required - energy_now) / power;
                    if (charging_time > max_charging_time) max_charging_time = charging_time;
                }
            }

            transmission_cost_total += max_charging_time;
            compution_cost_total += records.size();

            // 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
            for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
                string id = iter->first;
                for (int j = 0; j < iter->second.size(); j++) {
                    double power = station_sensors_energy_power[id][j].second;
                    // 充电
                    station_sensors_energy_power[id][j].first += power * max_charging_time;
                }
            }

            // 工作的sensor消耗电量
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                station_sensors_energy_power[id][temp_attribute].first -= energy_required;
            }

            // 传统没有过滤
        }
        total_result_number += records.size();
        count++;

        for(auto a :records){
            cout << '(' << a.first << ':';
            for(auto b: a.second) cout << b << ','; cout << "),";
        }
        cout << endl;

        if (count == random_query_attributes.size()) break;
    }

    if (AIR_PRINT_MESSAGE) {
        //cout << "total result number: " << total_result_number << endl;
        cout << "average compution cost: " << (double)compution_cost_total / count << endl;
    }

    return transmission_cost_total / (count);
}

double MultipleQuery::RF_sequential(string inFileName, int attribute_number) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;
	map<string, vector<pair<double, double>>> station_sensors_energy_power;

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute;
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		random_query_attributes.push_back(query_attribute);

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
		}
		station_sensors_energy_power[*iter] = sensors_energy_power;
	}

	int count = 0;
	double transmission_cost_total = 0;
	int total_result_number = 0;
	int compution_cost_total = 0;

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
		vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值
		
		// 循环次数最多不会超过查询属性的个数
		for (int i = 0; i < query_attribute.size(); i++) {
			int temp_attribute = query_attribute[i];
			double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

			double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

			// 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
			double max_charging_time = 0;
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				double energy_now = station_sensors_energy_power[id][temp_attribute].first;
				double power = station_sensors_energy_power[id][temp_attribute].second;
				if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
					double charging_time = (energy_required - energy_now) / power;
					if (charging_time > max_charging_time) max_charging_time = charging_time;
				}
			}

			transmission_cost_total += max_charging_time;
			compution_cost_total += records.size();

			// 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
			for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
				string id = iter->first;
				for (int j = 0; j < iter->second.size(); j++) {
					double power = station_sensors_energy_power[id][j].second;
					// 充电
					station_sensors_energy_power[id][j].first += power * max_charging_time;
				}
			}

			// 工作的sensor消耗电量
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				station_sensors_energy_power[id][temp_attribute].first -= energy_required;
			}

			// 过滤掉不满足条件的station
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					iter = records.erase(iter);
					continue;
				}
				else {
					iter++;
				}
			}

			if (records.empty()) {
				break;
			}
		}
		total_result_number += records.size();
		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}

	if (AIR_PRINT_MESSAGE) {
		//cout << "total result number: " << total_result_number << endl;
		cout << "average compution cost: " << (double)compution_cost_total / count << endl;
	}

	return transmission_cost_total / (count);
}
double MultipleQuery::RF_sequential_file(std::string inFileName, std::string attribute_path){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];
    map<string, vector<pair<double, double>>> station_sensors_energy_power;



    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
        }
        station_sensors_energy_power[*iter] = sensors_energy_power;
    }

    int count = 0;
    double transmission_cost_total = 0;
    int total_result_number = 0;
    int compution_cost_total = 0;

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }

    string line;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

        vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
        vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值

        // 循环次数最多不会超过查询属性的个数
        for (int i = 0; i < query_attribute.size(); i++) {
            int temp_attribute = query_attribute[i];
            double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

            double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

            // 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
            double max_charging_time = 0;
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                double energy_now = station_sensors_energy_power[id][temp_attribute].first;
                double power = station_sensors_energy_power[id][temp_attribute].second;
                if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
                    double charging_time = (energy_required - energy_now) / power;
                    if (charging_time > max_charging_time) max_charging_time = charging_time;
                }
            }

            transmission_cost_total += max_charging_time;
            compution_cost_total += records.size();

            // 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
            for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
                string id = iter->first;
                for (int j = 0; j < iter->second.size(); j++) {
                    double power = station_sensors_energy_power[id][j].second;
                    // 充电
                    station_sensors_energy_power[id][j].first += power * max_charging_time;
                }
            }

            // 工作的sensor消耗电量
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                station_sensors_energy_power[id][temp_attribute].first -= energy_required;
            }

            // 过滤掉不满足条件的station
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    iter = records.erase(iter);
                    continue;
                }
                else {
                    iter++;
                }
            }

            if (records.empty()) {
                break;
            }
        }
        total_result_number += records.size();
        count++;

        for(auto a :records){
            cout << '(' << a.first << ':';
            for(auto b: a.second) cout << b << ','; cout << "),";
        }
        cout << endl;

        if (count == random_query_attributes.size()) break;
    }

    if (AIR_PRINT_MESSAGE) {
        //cout << "total result number: " << total_result_number << endl;
        cout << "average compution cost: " << (double)compution_cost_total / count << endl;
    }

    return transmission_cost_total / (count);
}

double MultipleQuery::RF_sel(std::string inFileName, int attribute_number) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;
	map<string, vector<pair<double, double>>> station_sensors_energy_power;

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute;
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		random_query_attributes.push_back(query_attribute);

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
		}
		station_sensors_energy_power[*iter] = sensors_energy_power;
	}

	int count = 0;
	double transmission_cost_total = 0;
	int total_result_number = 0;
	int compution_cost_total = 0;

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> random_query_attribute = random_query_attributes[count];
		sort(random_query_attribute.begin(), random_query_attribute.end());
		vector<int> query_n = n_vector[count];

		// 计算所有查询属性的 Sel
		vector<int> query_attribute;
		vector<pair<int, double>> sel_or_pri_value;

		for (int i = 0; i < random_query_attribute.size(); i++) {
			int temp_attribute = random_query_attribute[i];

			int temp_count = 0;
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					temp_count++;
				}
			}
			double value = 1 - double(temp_count) / records.size();
			sel_or_pri_value.push_back(make_pair(temp_attribute, value));
		}
		sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);
		
		for (int i = 0; i < sel_or_pri_value.size(); i++) {
			query_attribute.push_back(sel_or_pri_value[i].first);
		}

		// 循环次数最多不会超过查询属性的个数
		for (int i = 0; i < query_attribute.size(); i++) {
			int temp_attribute = query_attribute[i];
			double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

			double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

			// 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
			double max_charging_time = 0;
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				double energy_now = station_sensors_energy_power[id][temp_attribute].first;
				double power = station_sensors_energy_power[id][temp_attribute].second;
				if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
					double charging_time = (energy_required - energy_now) / power;
					if (charging_time > max_charging_time) max_charging_time = charging_time;
				}
			}

			transmission_cost_total += max_charging_time;
			compution_cost_total += records.size();

			// 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
			for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
				string id = iter->first;
				for (int j = 0; j < iter->second.size(); j++) {
					double power = station_sensors_energy_power[id][j].second;
					// 充电
					station_sensors_energy_power[id][j].first += power * max_charging_time;
				}
			}

			// 工作的sensor消耗电量
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				station_sensors_energy_power[id][temp_attribute].first -= energy_required;
			}

			// 过滤掉不满足条件的station
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					iter = records.erase(iter);
					continue;
				}
				else {
					iter++;
				}
			}

			if (records.empty()) {
				break;
			}
		}
		total_result_number += records.size();

		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}

	if (AIR_PRINT_MESSAGE) {
		//cout << "total result number: " << total_result_number << endl;
		cout << "average compution cost: " << (double)compution_cost_total / count << endl;
	}

	return transmission_cost_total / (count);
}
double MultipleQuery::RF_sel_file(std::string inFileName, std::string attribute_path){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];
    map<string, vector<pair<double, double>>> station_sensors_energy_power;



    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
        }
        station_sensors_energy_power[*iter] = sensors_energy_power;
    }

    int count = 0;
    double transmission_cost_total = 0;
    int total_result_number = 0;
    int compution_cost_total = 0;

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }

    string line;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

        vector<int> random_query_attribute = random_query_attributes[count];
        sort(random_query_attribute.begin(), random_query_attribute.end());
        vector<int> query_n = n_vector[count];

        // 计算所有查询属性的 Sel
        vector<int> query_attribute;
        vector<pair<int, double>> sel_or_pri_value;

        for (int i = 0; i < random_query_attribute.size(); i++) {
            int temp_attribute = random_query_attribute[i];

            int temp_count = 0;
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    temp_count++;
                }
            }
            double value = 1 - double(temp_count) / records.size();
            sel_or_pri_value.push_back(make_pair(temp_attribute, value));
        }
        sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);

        for (int i = 0; i < sel_or_pri_value.size(); i++) {
            query_attribute.push_back(sel_or_pri_value[i].first);
        }

        // 循环次数最多不会超过查询属性的个数
        for (int i = 0; i < query_attribute.size(); i++) {
            int temp_attribute = query_attribute[i];
            double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

            double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

            // 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
            double max_charging_time = 0;
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                double energy_now = station_sensors_energy_power[id][temp_attribute].first;
                double power = station_sensors_energy_power[id][temp_attribute].second;
                if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
                    double charging_time = (energy_required - energy_now) / power;
                    if (charging_time > max_charging_time) max_charging_time = charging_time;
                }
            }

            transmission_cost_total += max_charging_time;
            compution_cost_total += records.size();

            // 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
            for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
                string id = iter->first;
                for (int j = 0; j < iter->second.size(); j++) {
                    double power = station_sensors_energy_power[id][j].second;
                    // 充电
                    station_sensors_energy_power[id][j].first += power * max_charging_time;
                }
            }

            // 工作的sensor消耗电量
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                station_sensors_energy_power[id][temp_attribute].first -= energy_required;
            }

            // 过滤掉不满足条件的station
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    iter = records.erase(iter);
                    continue;
                }
                else {
                    iter++;
                }
            }

            if (records.empty()) {
                break;
            }
        }
        total_result_number += records.size();

        for(auto a :records){
            cout << '(' << a.first << ':';
            for(auto b: a.second) cout << b << ','; cout << "),";
        }
        cout << endl;

        count++;
        if (count == random_query_attributes.size()) break;
    }

    if (AIR_PRINT_MESSAGE) {
        //cout << "total result number: " << total_result_number << endl;
        cout << "average compution cost: " << (double)compution_cost_total / count << endl;
    }

    return transmission_cost_total / (count);
}


double MultipleQuery::energy_harvest_tradition(string inFileName, double power_min, double power_max, int seed, int attribute_number) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;
	map<string, vector<pair<double, double>>> station_sensors_energy_power;

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute;
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		random_query_attributes.push_back(query_attribute);

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	srand(seed);
	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
			sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
		}
		station_sensors_energy_power[*iter] = sensors_energy_power;
	}

	int count = 0;
	double transmission_cost_total = 0;
	int total_result_number = 0;
	int compution_cost_total = 0;

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
		vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值

		// 循环次数最多不会超过查询属性的个数
		for (int i = 0; i < query_attribute.size(); i++) {
			int temp_attribute = query_attribute[i];
			double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

			double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

			// 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
			double max_charging_time = 0;
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				double energy_now = station_sensors_energy_power[id][temp_attribute].first;
				double power = station_sensors_energy_power[id][temp_attribute].second;
				if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
					double charging_time = (energy_required - energy_now) / power;
					if (charging_time > max_charging_time) max_charging_time = charging_time;
				}
			}

			transmission_cost_total += max_charging_time;
			compution_cost_total += records.size();

			// 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
			for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
				string id = iter->first;
				for (int j = 0; j < iter->second.size(); j++) {
					double power = station_sensors_energy_power[id][j].second;
					// 充电
					station_sensors_energy_power[id][j].first += power * max_charging_time;
				}
			}

			// 工作的sensor消耗电量
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				station_sensors_energy_power[id][temp_attribute].first -= energy_required;
			}

			// 传统没有过滤
		}
		total_result_number += records.size();
		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}

	if (AIR_PRINT_MESSAGE) {
		//cout << "total result number: " << total_result_number << endl;
		cout << "average compution cost: " << (double)compution_cost_total / count << endl;
	}

	return transmission_cost_total / (count);
}
double MultipleQuery::energy_harvest_tradition_file(string inFileName, double power_min, double power_max, int seed, std::string attribute_path){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];
    map<string, vector<pair<double, double>>> station_sensors_energy_power;



    srand(seed);
    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
            sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
        }
        station_sensors_energy_power[*iter] = sensors_energy_power;
    }

    int count = 0;
    double transmission_cost_total = 0;
    int total_result_number = 0;
    int compution_cost_total = 0;

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }

    string line;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

        vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
        vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值

        // 循环次数最多不会超过查询属性的个数
        for (int i = 0; i < query_attribute.size(); i++) {
            int temp_attribute = query_attribute[i];
            double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

            double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

            // 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
            double max_charging_time = 0;
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                double energy_now = station_sensors_energy_power[id][temp_attribute].first;
                double power = station_sensors_energy_power[id][temp_attribute].second;
                if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
                    double charging_time = (energy_required - energy_now) / power;
                    if (charging_time > max_charging_time) max_charging_time = charging_time;
                }
            }

            transmission_cost_total += max_charging_time;
            compution_cost_total += records.size();

            // 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
            for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
                string id = iter->first;
                for (int j = 0; j < iter->second.size(); j++) {
                    double power = station_sensors_energy_power[id][j].second;
                    // 充电
                    station_sensors_energy_power[id][j].first += power * max_charging_time;
                }
            }

            // 工作的sensor消耗电量
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                station_sensors_energy_power[id][temp_attribute].first -= energy_required;
            }

            // 传统没有过滤
        }
        total_result_number += records.size();
        count++;

        for(auto a :records){
            cout << '(' << a.first << ':';
            for(auto b: a.second) cout << b << ','; cout << "),";
        }
        cout << endl;

        if (count == random_query_attributes.size()) break;
    }

    if (AIR_PRINT_MESSAGE) {
        //cout << "total result number: " << total_result_number << endl;
        cout << "average compution cost: " << (double)compution_cost_total / count << endl;
    }

    return transmission_cost_total / (count);
}

double MultipleQuery::energy_harvest_sequential(string inFileName, double power_min, double power_max, int seed, int attribute_number) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;
	map<string, vector<pair<double, double>>> station_sensors_energy_power;

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute;
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		random_query_attributes.push_back(query_attribute);

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	srand(seed);
	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
			sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
		}
		station_sensors_energy_power[*iter] = sensors_energy_power;
	}

	int count = 0;
	double transmission_cost_total = 0;
	int total_result_number = 0;
	int compution_cost_total = 0;

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
		vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值

		// 循环次数最多不会超过查询属性的个数
		for (int i = 0; i < query_attribute.size(); i++) {
			int temp_attribute = query_attribute[i];
			double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

			double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

			// 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
			double max_charging_time = 0;
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				double energy_now = station_sensors_energy_power[id][temp_attribute].first;
				double power = station_sensors_energy_power[id][temp_attribute].second;
				if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
					double charging_time = (energy_required - energy_now) / power;
					if (charging_time > max_charging_time) max_charging_time = charging_time;
				}
			}

			transmission_cost_total += max_charging_time;
			compution_cost_total += records.size();

			// 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
			for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
				string id = iter->first;
				for (int j = 0; j < iter->second.size(); j++) {
					double power = station_sensors_energy_power[id][j].second;
					// 充电
					station_sensors_energy_power[id][j].first += power * max_charging_time;
				}
			}

			// 工作的sensor消耗电量
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				station_sensors_energy_power[id][temp_attribute].first -= energy_required;
			}

			// 过滤掉不满足条件的station
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					iter = records.erase(iter);
					continue;
				}
				else {
					iter++;
				}
			}

			if (records.empty()) {
				break;
			}
		}
		total_result_number += records.size();
		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}

	if (AIR_PRINT_MESSAGE) {
		//cout << "total result number: " << total_result_number << endl;
		cout << "average compution cost: " << (double)compution_cost_total / count << endl;
	}

	return transmission_cost_total / (count);
}
double MultipleQuery::energy_harvest_sequential_file(string inFileName, double power_min, double power_max, int seed, std::string attribute_path){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];
    map<string, vector<pair<double, double>>> station_sensors_energy_power;



    srand(seed);
    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
            sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
        }
        station_sensors_energy_power[*iter] = sensors_energy_power;
    }

    int count = 0;
    double transmission_cost_total = 0;
    int total_result_number = 0;
    int compution_cost_total = 0;

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }

    string line;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

        vector<int> query_attribute = random_query_attributes[count]; // 本次查询的查询属性！
        vector<int> query_n = n_vector[count]; // 本次查询的n，其中包括七个属性的n值

        // 循环次数最多不会超过查询属性的个数
        for (int i = 0; i < query_attribute.size(); i++) {
            int temp_attribute = query_attribute[i];
            double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

            double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

            // 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
            double max_charging_time = 0;
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                double energy_now = station_sensors_energy_power[id][temp_attribute].first;
                double power = station_sensors_energy_power[id][temp_attribute].second;
                if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
                    double charging_time = (energy_required - energy_now) / power;
                    if (charging_time > max_charging_time) max_charging_time = charging_time;
                }
            }

            transmission_cost_total += max_charging_time;
            compution_cost_total += records.size();

            // 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
            for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
                string id = iter->first;
                for (int j = 0; j < iter->second.size(); j++) {
                    double power = station_sensors_energy_power[id][j].second;
                    // 充电
                    station_sensors_energy_power[id][j].first += power * max_charging_time;
                }
            }

            // 工作的sensor消耗电量
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                station_sensors_energy_power[id][temp_attribute].first -= energy_required;
            }

            // 过滤掉不满足条件的station
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    iter = records.erase(iter);
                    continue;
                }
                else {
                    iter++;
                }
            }

            if (records.empty()) {
                break;
            }
        }
        total_result_number += records.size();
        count++;

        for(auto a :records){
            cout << '(' << a.first << ':';
            for(auto b: a.second) cout << b << ','; cout << "),";
        }
        cout << endl;

        if (count == random_query_attributes.size()) break;
    }

    if (AIR_PRINT_MESSAGE) {
        //cout << "total result number: " << total_result_number << endl;
        cout << "average compution cost: " << (double)compution_cost_total / count << endl;
    }

    return transmission_cost_total / (count);
}

double MultipleQuery::energy_harvest_sel(string inFileName, double power_min, double power_max, int seed, int attribute_number) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;
	map<string, vector<pair<double, double>>> station_sensors_energy_power;

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute;
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		random_query_attributes.push_back(query_attribute);

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	srand(seed);
	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
			sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
		}
		station_sensors_energy_power[*iter] = sensors_energy_power;
	}

	int count = 0;
	double transmission_cost_total = 0;
	int total_result_number = 0;
	int compution_cost_total = 0;

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> random_query_attribute = random_query_attributes[count];
		sort(random_query_attribute.begin(), random_query_attribute.end());
		vector<int> query_n = n_vector[count];

		// 计算所有查询属性的 Sel
		vector<int> query_attribute;
		vector<pair<int, double>> sel_or_pri_value;

		for (int i = 0; i < random_query_attribute.size(); i++) {
			int temp_attribute = random_query_attribute[i];

			int temp_count = 0;
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					temp_count++;
				}
			}
			double value = 1 - double(temp_count) / records.size();
			sel_or_pri_value.push_back(make_pair(temp_attribute, value));
		}
		sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);

		for (int i = 0; i < sel_or_pri_value.size(); i++) {
			query_attribute.push_back(sel_or_pri_value[i].first);
		}

		// 循环次数最多不会超过查询属性的个数
		for (int i = 0; i < query_attribute.size(); i++) {
			int temp_attribute = query_attribute[i];
			double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

			double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

			// 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
			double max_charging_time = 0;
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				double energy_now = station_sensors_energy_power[id][temp_attribute].first;
				double power = station_sensors_energy_power[id][temp_attribute].second;
				if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
					double charging_time = (energy_required - energy_now) / power;
					if (charging_time > max_charging_time) max_charging_time = charging_time;
				}
			}

			transmission_cost_total += max_charging_time;
			compution_cost_total += records.size();

			// 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
			for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
				string id = iter->first;
				for (int j = 0; j < iter->second.size(); j++) {
					double power = station_sensors_energy_power[id][j].second;
					// 充电
					station_sensors_energy_power[id][j].first += power * max_charging_time;
				}
			}

			// 工作的sensor消耗电量
			for (int j = 0; j < records.size(); j++) {
				string id = records[j].first;
				station_sensors_energy_power[id][temp_attribute].first -= energy_required;
			}

			// 过滤掉不满足条件的station
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					iter = records.erase(iter);
					continue;
				}
				else {
					iter++;
				}
			}

			if (records.empty()) {
				break;
			}
		}
		total_result_number += records.size();

		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}

	if (AIR_PRINT_MESSAGE) {
		//cout << "total result number: " << total_result_number << endl;
		cout << "average compution cost: " << (double)compution_cost_total / count << endl;
	}

	return transmission_cost_total / (count);
}
double MultipleQuery::energy_harvest_sel_file(string inFileName, double power_min, double power_max, int seed, std::string attribute_path){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];
    map<string, vector<pair<double, double>>> station_sensors_energy_power;


    srand(seed);
    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
            sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
        }
        station_sensors_energy_power[*iter] = sensors_energy_power;
    }

    int count = 0;
    double transmission_cost_total = 0;
    int total_result_number = 0;
    int compution_cost_total = 0;

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }

    string line;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

        vector<int> random_query_attribute = random_query_attributes[count];
        sort(random_query_attribute.begin(), random_query_attribute.end());
        vector<int> query_n = n_vector[count];

        // 计算所有查询属性的 Sel
        vector<int> query_attribute;
        vector<pair<int, double>> sel_or_pri_value;

        for (int i = 0; i < random_query_attribute.size(); i++) {
            int temp_attribute = random_query_attribute[i];

            int temp_count = 0;
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    temp_count++;
                }
            }
            double value = 1 - double(temp_count) / records.size();
            sel_or_pri_value.push_back(make_pair(temp_attribute, value));
        }
        sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);

        for (int i = 0; i < sel_or_pri_value.size(); i++) {
            query_attribute.push_back(sel_or_pri_value[i].first);
        }

        // 循环次数最多不会超过查询属性的个数
        for (int i = 0; i < query_attribute.size(); i++) {
            int temp_attribute = query_attribute[i];
            double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

            double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

            // 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
            double max_charging_time = 0;
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                double energy_now = station_sensors_energy_power[id][temp_attribute].first;
                double power = station_sensors_energy_power[id][temp_attribute].second;
                if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
                    double charging_time = (energy_required - energy_now) / power;
                    if (charging_time > max_charging_time) max_charging_time = charging_time;
                }
            }

            transmission_cost_total += max_charging_time;
            compution_cost_total += records.size();

            // 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
            for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
                string id = iter->first;
                for (int j = 0; j < iter->second.size(); j++) {
                    double power = station_sensors_energy_power[id][j].second;
                    // 充电
                    station_sensors_energy_power[id][j].first += power * max_charging_time;
                }
            }

            // 工作的sensor消耗电量
            for (int j = 0; j < records.size(); j++) {
                string id = records[j].first;
                station_sensors_energy_power[id][temp_attribute].first -= energy_required;
            }

            // 过滤掉不满足条件的station
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    iter = records.erase(iter);
                    continue;
                }
                else {
                    iter++;
                }
            }

            if (records.empty()) {
                break;
            }
        }
        total_result_number += records.size();

        for(auto a :records){
            cout << '(' << a.first << ':';
            for(auto b: a.second) cout << b << ','; cout << "),";
        }
        cout << endl;

        count++;
        if (count == random_query_attributes.size()) break;
    }

    if (AIR_PRINT_MESSAGE) {
        //cout << "total result number: " << total_result_number << endl;
        cout << "average compution cost: " << (double)compution_cost_total / count << endl;
    }


    return transmission_cost_total / (count);
}


// 实验 6.2
void MultipleQuery::RF(string inFileName, int attribute_number, int k) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;

	map<string, vector<pair<double, double>>> station_sensors_energy_power_tradition; // 传统的
	map<string, vector<pair<double, double>>> station_sensors_energy_power_sen; // 渐进顺序
	map<string, vector<pair<double, double>>> station_sensors_energy_power_guided; // 渐进guided
	map<string, vector<pair<double, double>>> station_sensors_energy_power_pri; // 渐进pri

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute; // 在6.2实验中不用随机生成查询属性了
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		//random_query_attributes.push_back(query_attribute);
		random_query_attributes.push_back({4,1,5,3});

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	// 更新这四个查询属性的传输代价
	vector<int> random_attribute = random_query_attributes[0];
	cost_trans[random_attribute[0]] = 16;
	for (int i = 1; i < random_attribute.size(); i++) {
		int temp_attribute = random_attribute[i];
		if (k == 1) {
			cost_trans[temp_attribute] = 16;
		}
		else {
			cost_trans[temp_attribute] = 16 * i * k;
		}
		
	}

	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
		}
		station_sensors_energy_power_tradition[*iter] = sensors_energy_power;
	}

	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
		}
		station_sensors_energy_power_sen[*iter] = sensors_energy_power;
	}

	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
		}
		station_sensors_energy_power_guided[*iter] = sensors_energy_power;
	}

	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
		}
		station_sensors_energy_power_pri[*iter] = sensors_energy_power;
	}

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}
	
	int count = 0;
	double total_sen = 0;
	double total_guided = 0;
	double total_pri = 0;

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> random_query_attribute = random_query_attributes[count]; // 传统和按condition的就用这个就可以
		vector<int> query_n = n_vector[count];

		vector<int> temp_random_query_attribute = random_query_attribute;
		sort(temp_random_query_attribute.begin(), temp_random_query_attribute.end());

		vector<int> pri_query_attribute;
		vector<pair<int, double>> sel_or_pri_value;

		for (int i = 0; i < temp_random_query_attribute.size(); i++) {
			int temp_attribute = temp_random_query_attribute[i];

			int temp_count = 0;
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					temp_count++;
				}
			}
			double value = (1 - double(temp_count) / records.size()) / cost_trans[temp_attribute];
			sel_or_pri_value.push_back(make_pair(temp_attribute, value));
		}
		sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);

		for (int i = 0; i < sel_or_pri_value.size(); i++) {
			pri_query_attribute.push_back(sel_or_pri_value[i].first);
		}

		//cout << "random attributes: ";
		//for (int i = 0; i < random_query_attribute.size(); i++) {
		//	cout << random_query_attribute[i] << ", ";
		//}
		//cout << endl;

		//cout << "pri attributes: ";
		//for (int i = 0; i < pri_query_attribute.size(); i++) {
		//	cout << pri_query_attribute[i] << ", ";
		//}
		//cout << endl;

		double tradition_time = charging(line, station_sensors_energy_power_tradition, random_query_attribute, query_n, true);
		double sen_time = charging(line, station_sensors_energy_power_sen, random_query_attribute, query_n, false);
		double guided_time = charging(line, station_sensors_energy_power_guided, temp_random_query_attribute, query_n, false);
		double pri_time = charging(line, station_sensors_energy_power_pri, pri_query_attribute, query_n, false);

		//cout << tradition_time << "  " << sen_time << "  " << pri_time << endl;
		total_sen += sen_time / tradition_time;
		total_guided += guided_time / tradition_time;
		total_pri += pri_time / tradition_time;

		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}

	cout << "RF sequential: " << total_sen / count << endl;
	cout << "RF guided: " << total_guided / count << endl;
	cout << "RF pri: " << total_pri / count << endl;
}
void MultipleQuery::RF_file(string inFileName, std::string attribute_path, int k){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];
    //cout << "here1 " << random_query_attributes.size() << " " << n_vector.size() << endl;

    map<string, vector<pair<double, double>>> station_sensors_energy_power_tradition; // 传统的
    map<string, vector<pair<double, double>>> station_sensors_energy_power_sen; // 渐进顺序
    map<string, vector<pair<double, double>>> station_sensors_energy_power_guided; // 渐进guided
    map<string, vector<pair<double, double>>> station_sensors_energy_power_pri; // 渐进pri


    // 更新这四个查询属性的传输代价
    //cout << "here2" << endl;
    vector<int> random_attribute = random_query_attributes[0];
    cost_trans[random_attribute[0]] = 16;
    for (int i = 1; i < random_attribute.size(); i++) {
        int temp_attribute = random_attribute[i];
        if (k == 1) {
            cost_trans[temp_attribute] = 16;
        }
        else {
            cost_trans[temp_attribute] = 16 * i * k;
        }

    }
    //cout << "here3" << endl;

    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
        }
        station_sensors_energy_power_tradition[*iter] = sensors_energy_power;
    }

    //cout << "here4" << endl;
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
        }
        station_sensors_energy_power_sen[*iter] = sensors_energy_power;
    }

    //cout << "here5" << endl;
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
        }
        station_sensors_energy_power_guided[*iter] = sensors_energy_power;
    }
    //cout << "here6" << endl;

    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            sensors_energy_power.push_back(make_pair(0, 4725)); // RF情形下sensor的接收功率都是4725uW
        }
        station_sensors_energy_power_pri[*iter] = sensors_energy_power;
    }
    //cout << "here7" << endl;

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }
    //cout << "here8" << endl;

    int count = 0;
    double total_sen = 0;
    double total_guided = 0;
    double total_pri = 0;

    string line;
    //cout << "here9" << endl;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];
        //cout << "here9.1" << endl;

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");
        //cout << "here9.2" << endl;

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        //cout << "here9.3" << endl;
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据
        //cout << "here9.31" << endl;

//        cout << count << " " << random_query_attributes.size() << endl;
        vector<int> random_query_attribute = random_query_attributes[count]; // 传统和按condition的就用这个就可以
        //cout << "here9.32" << endl;
        vector<int> query_n = n_vector[count];
        //cout << "here9.33" << endl;
        vector<int> temp_random_query_attribute = random_query_attribute;
        //cout << "here9.34" << endl;
        sort(temp_random_query_attribute.begin(), temp_random_query_attribute.end());
        //cout << "here9.35" << endl;

        vector<int> pri_query_attribute;
        vector<pair<int, double>> sel_or_pri_value;

        //cout << "here9.4" << endl;
        for (int i = 0; i < temp_random_query_attribute.size(); i++) {
            int temp_attribute = temp_random_query_attribute[i];

            int temp_count = 0;
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    temp_count++;
                    cout << '(' << (*iter).first << ':';
                    for(auto a: (*iter).second) cout << a << ','; cout << "),";
                }
            } cout << endl;
            double value = (1 - double(temp_count) / records.size()) / cost_trans[temp_attribute];
            sel_or_pri_value.push_back(make_pair(temp_attribute, value));
        }
        //cout << "here9.5" << endl;
        sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);
        //cout << "here9.6" << endl;

        for (int i = 0; i < sel_or_pri_value.size(); i++) {
            pri_query_attribute.push_back(sel_or_pri_value[i].first);
        }
        //cout << "here9.7" << endl;

        //cout << "random attributes: ";
        //for (int i = 0; i < random_query_attribute.size(); i++) {
        //	cout << random_query_attribute[i] << ", ";
        //}
        //cout << endl;

        //cout << "pri attributes: ";
        //for (int i = 0; i < pri_query_attribute.size(); i++) {
        //	cout << pri_query_attribute[i] << ", ";
        //}
        //cout << endl;

        //cout << "here10" << endl;
        double tradition_time = charging(line, station_sensors_energy_power_tradition, random_query_attribute, query_n, true);
        double sen_time = charging(line, station_sensors_energy_power_sen, random_query_attribute, query_n, false);
        double guided_time = charging(line, station_sensors_energy_power_guided, temp_random_query_attribute, query_n, false);
        double pri_time = charging(line, station_sensors_energy_power_pri, pri_query_attribute, query_n, false);

        //cout << "here11" << endl;
//        cout << tradition_time << "  " << sen_time << "  " << pri_time << endl;

        tradition_time = tradition_time ? tradition_time : 0.1;
        sen_time = sen_time ? sen_time : 0.1;
        guided_time = guided_time ? guided_time : 0.1;
        pri_time = pri_time ? pri_time : 0.1;

        total_sen += sen_time / tradition_time;
        total_guided += guided_time / tradition_time;
        total_pri += pri_time / tradition_time;

        //cout << "here12" << endl;
        count++;
        if (count == random_query_attributes.size()) break;
    }

    //cout << "here12" << endl;
    cout << "RF sequential: " << total_sen / count << endl;
    cout << "RF guided: " << total_guided / count << endl;
    cout << "RF pri: " << total_pri / count << endl;
}

void MultipleQuery::energy_harvest(string inFileName, double power_min, double power_max, int seed, int attribute_number, int k) {
	srand(AIR_RANDOM_SEED);

	vector<vector<int>> random_query_attributes; // 顺序的话，这个就是查询顺序
	vector<vector<int>> n_vector;

	map<string, vector<pair<double, double>>> station_sensors_energy_power_tradition; // 传统的
	map<string, vector<pair<double, double>>> station_sensors_energy_power_sen; // 渐进顺序
	map<string, vector<pair<double, double>>> station_sensors_energy_power_pri; // 渐进pri

	// 首先生成每次查询的查询属性，以及查询条件中的n
	for (int i = 0; i < AIR_SAMPLE_NUMBER; i++) {
		vector<int> attributes_all = { 0,1,2,3,4,5,6 };
		random_shuffle(attributes_all.begin(), attributes_all.end());

		// 生成每次查询的查询属性
		int k = attribute_number;
		vector<int> query_attribute;
		for (int j = 0; j < k; j++) {
			query_attribute.push_back(attributes_all[j]);
		}
		//random_query_attributes.push_back(query_attribute);
		random_query_attributes.push_back({ 4,1,5,3 });

		// 每次查询的查询条件中的 n
		vector<int> query_n;
		for (int j = 0; j < 7; j++) {
			vector<int> n_all = { 1,2,3 };
			random_shuffle(n_all.begin(), n_all.end());
			query_n.push_back(n_all[0]);
		}
		n_vector.push_back(query_n);
	}

	// 更新这四个查询属性的传输代价
	vector<int> random_attribute = random_query_attributes[0];
	cost_trans[random_attribute[0]] = 16;
	for (int i = 1; i < random_attribute.size(); i++) {
		int temp_attribute = random_attribute[i];
		if (k == 1) {
			cost_trans[temp_attribute] = 16;
		}
		else {
			cost_trans[temp_attribute] = 16 * i * k;
		}
	}

	srand(seed);
	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
			sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
		}
		station_sensors_energy_power_tradition[*iter] = sensors_energy_power;
	}

	srand(seed);
	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
			sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
		}
		station_sensors_energy_power_sen[*iter] = sensors_energy_power;
	}

	srand(seed);
	// 然后给初始化每个station的sensor的初始能量以及功率
	for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
		string station_name = *iter;
		vector<pair<double, double>> sensors_energy_power;
		for (int i = 0; i < 7; i++) { // 一共7个sensor
			double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
			sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
		}
		station_sensors_energy_power_pri[*iter] = sensors_energy_power;
	}

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!";
	}

	int count = 0;
	double total_sen = 0;
	double total_pri = 0;

	string line;
	while (getline(infile, line)) {
		vector<string> line_split = split(line, "\t");
		string time = line_split[0];
		string mean_str = line_split[2];
		string variance_str = line_split[3];

		mean_str = mean_str.substr(1);
		mean_str.replace(mean_str.size() - 1, 1, "");
		variance_str = variance_str.substr(1);
		variance_str.replace(variance_str.size() - 1, 1, "");
		vector<string> temp_vector_mean = split(mean_str, ", ");
		vector<string> temp_vector_variance = split(variance_str, ", ");

		vector<double> mean, variance; // 该时刻的方差，标准差
		for (int i = 0; i < temp_vector_mean.size(); i++) {
			mean.push_back(atof(temp_vector_mean[i].c_str()));
			variance.push_back(atof(temp_vector_variance[i].c_str()));
		}
		vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

		vector<int> random_query_attribute = random_query_attributes[count]; // 传统和按condition的就用这个就可以
		vector<int> query_n = n_vector[count];

		vector<int> temp_random_query_attribute = random_query_attribute;
		sort(temp_random_query_attribute.begin(), temp_random_query_attribute.end());

		vector<int> pri_query_attribute;
		vector<pair<int, double>> sel_or_pri_value;

		for (int i = 0; i < temp_random_query_attribute.size(); i++) {
			int temp_attribute = temp_random_query_attribute[i];

			int temp_count = 0;
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					temp_count++;
				}
			}
			double value = (1 - double(temp_count) / records.size()) / cost_trans[temp_attribute];
			sel_or_pri_value.push_back(make_pair(temp_attribute, value));
		}
		sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);

		for (int i = 0; i < sel_or_pri_value.size(); i++) {
			pri_query_attribute.push_back(sel_or_pri_value[i].first);
		}

		double tradition_time = charging(line, station_sensors_energy_power_tradition, random_query_attribute, query_n, true);
		double sen_time = charging(line, station_sensors_energy_power_sen, random_query_attribute, query_n, false);
		double pri_time = charging(line, station_sensors_energy_power_pri, pri_query_attribute, query_n, false);

		total_sen += sen_time / tradition_time;
		total_pri += pri_time / tradition_time;

		count++;
		if (count == AIR_SAMPLE_NUMBER) break;
	}
	cout << "sequential: " << total_sen / count << endl;
	cout << "pri: " << total_pri / count << endl;
}
void MultipleQuery::energy_harvest_file(string inFileName, double power_min, double power_max, int seed, std::string attribute_path, int k){
    srand(AIR_RANDOM_SEED);

    vector<vector<vector<int>>> params = get_params(attribute_path);
    vector<vector<int>> random_query_attributes = params[0]; // 顺序的话，这个就是查询顺序
    vector<vector<int>> n_vector = params[1];

    map<string, vector<pair<double, double>>> station_sensors_energy_power_tradition; // 传统的
    map<string, vector<pair<double, double>>> station_sensors_energy_power_sen; // 渐进顺序
    map<string, vector<pair<double, double>>> station_sensors_energy_power_pri; // 渐进pri



    // 更新这四个查询属性的传输代价
    vector<int> random_attribute = random_query_attributes[0];
    cost_trans[random_attribute[0]] = 16;
    for (int i = 1; i < random_attribute.size(); i++) {
        int temp_attribute = random_attribute[i];
        if (k == 1) {
            cost_trans[temp_attribute] = 16;
        }
        else {
            cost_trans[temp_attribute] = 16 * i * k;
        }
    }

    srand(seed);
    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
            sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
        }
        station_sensors_energy_power_tradition[*iter] = sensors_energy_power;
    }

    srand(seed);
    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
            sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
        }
        station_sensors_energy_power_sen[*iter] = sensors_energy_power;
    }

    srand(seed);
    // 然后给初始化每个station的sensor的初始能量以及功率
    for (set<string>::iterator iter = station_id.begin(); iter != station_id.end(); iter++) {
        string station_name = *iter;
        vector<pair<double, double>> sensors_energy_power;
        for (int i = 0; i < 7; i++) { // 一共7个sensor
            double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
            sensors_energy_power.push_back(make_pair(0, temp_power * 1000));
        }
        station_sensors_energy_power_pri[*iter] = sensors_energy_power;
    }

    ifstream infile;
    infile.open(inFileName);
    if (!infile.is_open()) {
        cout << "Open " << inFileName << " fail!";
    }

    int count = 0;
    double total_sen = 0;
    double total_pri = 0;

    string line;
    while (getline(infile, line)) {
        vector<string> line_split = split(line, "\t");
        string time = line_split[0];
        string mean_str = line_split[2];
        string variance_str = line_split[3];

        mean_str = mean_str.substr(1);
        mean_str.replace(mean_str.size() - 1, 1, "");
        variance_str = variance_str.substr(1);
        variance_str.replace(variance_str.size() - 1, 1, "");
        vector<string> temp_vector_mean = split(mean_str, ", ");
        vector<string> temp_vector_variance = split(variance_str, ", ");

        vector<double> mean, variance; // 该时刻的方差，标准差
        for (int i = 0; i < temp_vector_mean.size(); i++) {
            mean.push_back(atof(temp_vector_mean[i].c_str()));
            variance.push_back(atof(temp_vector_variance[i].c_str()));
        }
        vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

        vector<int> random_query_attribute = random_query_attributes[count]; // 传统和按condition的就用这个就可以
        vector<int> query_n = n_vector[count];

        vector<int> temp_random_query_attribute = random_query_attribute;
        sort(temp_random_query_attribute.begin(), temp_random_query_attribute.end());

        vector<int> pri_query_attribute;
        vector<pair<int, double>> sel_or_pri_value;

        for (int i = 0; i < temp_random_query_attribute.size(); i++) {
            int temp_attribute = temp_random_query_attribute[i];

            int temp_count = 0;
            for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); iter++) {
                if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) < query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
                    temp_count++;
                    cout << '(' << (*iter).first << ':';
                    for(auto a: (*iter).second) cout << a << ','; cout << "),";
                }
            }cout << endl;
            double value = (1 - double(temp_count) / records.size()) / cost_trans[temp_attribute];
            sel_or_pri_value.push_back(make_pair(temp_attribute, value));
        }
        sort(sel_or_pri_value.begin(), sel_or_pri_value.end(), cmp);

        for (int i = 0; i < sel_or_pri_value.size(); i++) {
            pri_query_attribute.push_back(sel_or_pri_value[i].first);
        }

        double tradition_time = charging(line, station_sensors_energy_power_tradition, random_query_attribute, query_n, true);
        double sen_time = charging(line, station_sensors_energy_power_sen, random_query_attribute, query_n, false);
        double pri_time = charging(line, station_sensors_energy_power_pri, pri_query_attribute, query_n, false);

        total_sen += sen_time / tradition_time;
        total_pri += pri_time / tradition_time;

        count++;
        if (count == random_query_attributes.size()) break;
    }
    cout << "sequential: " << total_sen / count << endl;
    cout << "pri: " << total_pri / count << endl;
}


double MultipleQuery::charging(string line, map<string, vector<pair<double, double>>>& station_sensors_energy_power, vector<int> query_attribute, vector<int> query_n, bool isTra) {
	vector<string> line_split = split(line, "\t");
	string time = line_split[0];
	string mean_str = line_split[2];
	string variance_str = line_split[3];

	mean_str = mean_str.substr(1);
	mean_str.replace(mean_str.size() - 1, 1, "");
	variance_str = variance_str.substr(1);
	variance_str.replace(variance_str.size() - 1, 1, "");
	vector<string> temp_vector_mean = split(mean_str, ", ");
	vector<string> temp_vector_variance = split(variance_str, ", ");

	vector<double> mean, variance; // 该时刻的方差，标准差
	for (int i = 0; i < temp_vector_mean.size(); i++) {
		mean.push_back(atof(temp_vector_mean[i].c_str()));
		variance.push_back(atof(temp_vector_variance[i].c_str()));
	}
	vector<pair<string, vector<double>>> records = air_time_record[time]; // 该时刻采集数据的station及其采集上来的数据

	double transmission_cost_total = 0;
	
	// 循环次数最多不会超过查询属性的个数
	for (int i = 0; i < query_attribute.size(); i++) {
		int temp_attribute = query_attribute[i];
		double standard_value = mean[temp_attribute] + query_n[temp_attribute] * sqrt(variance[temp_attribute]);

		double energy_required = cost_trans[temp_attribute] * unit_8_cost_trans + initial_cost_trans; // 传输该属性需要的能量

		// 计算传输代价（充电时间），谁需要传数据，就看谁的充电时间
		double max_charging_time = 0;
		for (int j = 0; j < records.size(); j++) {
			string id = records[j].first;
			double energy_now = station_sensors_energy_power[id][temp_attribute].first;
			double power = station_sensors_energy_power[id][temp_attribute].second;
			if (energy_now < energy_required) { // 当前电量低于所需电量时，才需要充电
				double charging_time = (energy_required - energy_now) / power;
				if (charging_time > max_charging_time) max_charging_time = charging_time;
			}
		}

		transmission_cost_total += max_charging_time;

		// 根据本次传输代价来给所有的sensor充电，给36 * 7个sensor充电
		for (map<string, vector<pair<double, double>>>::iterator iter = station_sensors_energy_power.begin(); iter != station_sensors_energy_power.end(); iter++) {
			string id = iter->first;
			for (int j = 0; j < iter->second.size(); j++) {
				double power = station_sensors_energy_power[id][j].second;
				// 充电
				station_sensors_energy_power[id][j].first += power * max_charging_time;
			}
		}

		// 工作的sensor消耗电量
		for (int j = 0; j < records.size(); j++) {
			string id = records[j].first;
			station_sensors_energy_power[id][temp_attribute].first -= energy_required;
		}

		if (!isTra) { // 传统的不需要过滤
			// 过滤掉不满足条件的station
			for (vector<pair<string, vector<double>>>::iterator iter = records.begin(); iter != records.end(); ) {
				if (abs((*iter).second[temp_attribute] - mean[temp_attribute]) >= query_n[temp_attribute] * sqrt(variance[temp_attribute])) {
					iter = records.erase(iter);
					continue;
				}
				else {
					iter++;
				}
			}

			if (records.empty()) {
				break;
			}
		}
	}
	return transmission_cost_total;
}

vector<vector<vector<int>>> MultipleQuery::get_params(std::string file_path){
    ifstream ifs(file_path, ios::in);
    string line(1000, 0);  // todo: may be not enough?

    vector<vector<int>> query_attributes;
    vector<vector<int>> n_vector;

    while(getline(ifs, line)){
        vector<string> line_split = split(line, "\t");  // query attributes
        vector<int> tmp;
        for(string& a: line_split) tmp.emplace_back(stoi(a));
        query_attributes.emplace_back(tmp);
        tmp.clear();

        getline(ifs, line);
        line_split = split(line, "\t");  // n_vector
        for(string& a: line_split) tmp.emplace_back(stoi(a));
        n_vector.emplace_back(tmp);
    }

    ifs.close();

    return {query_attributes, n_vector};
}