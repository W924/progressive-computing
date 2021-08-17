#include "PreProcess.h"
#include "Util.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

using namespace std;

void HT_Sensor_sample_average(std::string inFileName, int sensor_num, int d) {
	ifstream infile;
	ofstream outfile;

	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Error: Open " << inFileName << "Fail!" << endl;
	}

	outfile.open(inFileName + "_" + to_string(d));
	outfile.setf(ios::fixed, ios::floatfield);
	outfile.precision(6);

	string id = "0";
	int count = 0;
	double sum = 0;
	vector<double> average_value(sensor_num, 0);

	string line;
	getline(infile, line);
	while (getline(infile, line)) {
		count++;
		vector<string> sensors_value = split(line, "  ");

		string new_id = sensors_value[0];
		
		if (new_id == id && count < d) { // 求平均数
			for (int i = 2; i < sensors_value.size() - 1; i++) { // size - 1是因为，处理的文件中每一行最后都有"  "，按我们的split()函数分割出的会多一个空字符串
				average_value[i - 2] += (atof(sensors_value[i].c_str()) - average_value[i - 2]) / count;
			}
		} else if (new_id == id && count == d) {  // 每d个求完平均数写回文件
			outfile << id;
			for (int i = 0; i < average_value.size(); i++) {
				outfile << "\t" << average_value[i];
			}
			outfile << endl;
			count = 0;
			average_value.assign(sensor_num, 0);
		} else { // 遇到新的ID，将上一个ID剩余数据的平均数写回文件，该条记录作为新ID的第一条
			outfile << id;
			for (int i = 0; i < average_value.size(); i++) {
				outfile << "\t" << average_value[i];
			}
			outfile << endl;

			id = new_id;
			count = 1;
			average_value.assign(sensor_num, 0);
			for (int i = 2; i < sensors_value.size() - 1; i++) {
				average_value[i - 2] = atof(sensors_value[i].c_str());
			}
		}
	}

	// 最后合并的记录写回文件
	outfile << id;
	for (int i = 0; i < average_value.size(); i++) {
		outfile << "\t" << average_value[i];
	}
	outfile << endl;

	infile.close();
	outfile.close();
}


void wilt_transfer_csv_to_txt(std::string inFileName) {
	ifstream infile;
	ofstream outfile;

	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open File Failure!" << endl;
	}

	outfile.open("./input/wilt/wilt_train.txt");

	map<string, int> classifications;
	classifications["w"] = 1;
	classifications["n"] = 0;

	string line;
	getline(infile, line);
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ",");
		for (int i = 1; i < temp_vector.size(); i++) {
			outfile << temp_vector[i] << " ";
		}
		outfile << classifications[temp_vector[0]] << endl;
	}
	infile.close();
	outfile.close();

}


void network_precision(string inFileName, string outFileName) {
	ifstream infile;
	ofstream outfile;

	infile.open(inFileName);
	outfile.open(outFileName);
	outfile.setf(ios::fixed, ios::floatfield);
	outfile.precision(6);
	
	string line;
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ":");
		vector<string> temp_vector_1 = split(temp_vector[0], ",");
		vector<string> temp_vector_2 = split(temp_vector[1], ",");

		outfile << temp_vector_1[0] << "," << atof(temp_vector_1[1].c_str()) << "," << atof(temp_vector_1[2].c_str()) << ":";
		outfile << temp_vector_2[0] << "," << atof(temp_vector_2[1].c_str()) << "," << atof(temp_vector_2[2].c_str()) << ":";
		outfile << atof(temp_vector[2].c_str()) << endl;
	}
	infile.close();
	outfile.close();
}


void Air_preprocess(string inFileName) {
	ifstream infile;
	infile.open(inFileName);
	ofstream outfile;
	outfile.open("./output/test.txt");

	map<string, int> counts;
	string line;
	while (getline(infile, line)) {
		vector<string> temp_vector = split2(line, ",");
		bool flag = true;
		for (int i = 0; i < temp_vector.size(); i++) {
			if (temp_vector[i] == "") {
				flag = false;
				break;
			}
		}
		if (flag) {
			outfile << line << endl;
		}
	}

	infile.close();
	outfile.close();
}


void beach_empty_record(std::string inFileName) {
	ifstream infile;
	infile.open(inFileName);
	ofstream outfile;
	outfile.open("./output/test.txt");

	map<string, int> counts;
	string line;
	while (getline(infile, line)) {
		vector<string> temp_vector = split2(line, ",");
		bool flag = true;
		for (int i = 2; i < temp_vector.size(); i++) {
			if (temp_vector[i] == "") {
				flag = false;
				break;
			}
		}
		if (flag) {
			outfile << line << endl;
		}
	}

	infile.close();
	outfile.close();
}