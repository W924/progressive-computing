#include "DecisionTree.h"
#include "DecisionTreeNode.h"
#include "ConstDefine.h"
#include "Util.h"
#include "Sensor.h"
#include <vector>
#include <fstream>
#include <regex>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>

using namespace std;


void DecisionTree::constructDecisionTree(string inFileName) {
	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open file failure!" << endl;
	}
 
	// C++正则表达式匹配方括号要用 \\[, \\] 
	// C++正则表达式匹配双引号要用 \" , \"
	// C++正则表达式匹配 \ 要用 \\\\ 
	// 匹配中间节点，1：节点ID，2：划分属性，3：属性划分值，4：entropy|gini，5：entropy|gini value，6：样本总数，7：每个类别的样本数
	// 匹配叶结点，1：节点ID，2：entropy|gini，3：entropy|gini value，4：样本总数，5：每个类别的样本数
	// 匹配节点的父子关系，1：父节点ID，2：子节点ID
	string pattern_decisiontree_node = "([0-9]+) \\[label=\"X\\[([0-9]+)\\] <= (.+)\\\\n(entropy|gini) = (.+)\\\\nsamples = (.+)\\\\nvalue = \\[(.+)\\]\"\\] ;";
	string pattern_decisiontree_leaf = "([0-9]+) \\[label=\"(entropy|gini) = (.+)\\\\nsamples = (.+)\\\\nvalue = \\[(.+)\\]\"\\] ;";
	string pattern_decisiontree_arrow = "([0-9]+) -> ([0-9]+) (\\[.+\\] )?;";

	regex regex_decisiontree_node(pattern_decisiontree_node);
	regex regex_decisiontree_leaf(pattern_decisiontree_leaf);
	regex regex_decisiontree_arrow(pattern_decisiontree_arrow);
	smatch m;

	string line;
	while (getline(infile, line)) {
		if (regex_search(line, m, regex_decisiontree_node)) {
			// 确定中间节点的ID，划分属性，属性划分值，
			int id = atoi(m.str(1).c_str());
			int divide_attribute = atoi(m.str(2).c_str());
			double divide_value = atof(m.str(3).c_str());

			// 以防万一，中间节点上的样本信息也存一下
			int classification = 0;
			vector<int> candidate_class;

			string samples = m.str(7);
			vector<string> temp_vector = split(samples, ", ");
			int sample_max = 0;
			for (int i = 0; i < temp_vector.size(); i++) {
				int temp = atoi(temp_vector[i].c_str());
				if (temp > 0) {
					candidate_class.push_back(i);
				}
				if (temp > sample_max) {
					classification = i;
					sample_max = temp;
				}
			}

			DecisionTreeNode* new_node = new DecisionTreeNode(id, false, divide_attribute, divide_value, classification, candidate_class);
			id_node[id] = new_node;
		}
		else if (regex_search(line, m, regex_decisiontree_leaf)) {
		// 1：节点ID，2：entropy|gini，3：entropy|gini value，4：样本总数，5：每个类别的样本数
			int id = atoi(m.str(1).c_str());
			int classification = 0;
			vector<int> candidate_class;

			string samples = m.str(5);
			vector<string> temp_vector = split(samples, ", ");
			int sample_max = 0;
			for (int i = 0; i < temp_vector.size(); i++) {
				int temp = atoi(temp_vector[i].c_str());
				if (temp > 0) {
					candidate_class.push_back(i);
				}
				if (temp > sample_max) {
					classification = i;
					sample_max = temp;
				}
			}

			DecisionTreeNode* new_node = new DecisionTreeNode(id, true, -1, -1, classification, candidate_class); // 叶结点没有划分属性，这里用-1来代替
			id_node[id] = new_node;
			leaves.push_back(new_node);
		}
		else if (regex_search(line, m, regex_decisiontree_arrow)) {
			DecisionTreeNode* father = id_node[atoi(m.str(1).c_str())];
			DecisionTreeNode* child = id_node[atoi(m.str(2).c_str())];

			child->father_node = father;
			child->level = father->level + 1;
			if (father->true_node == nullptr) {
				father->true_node = child;
			}
			else {
				father->false_node = child;
			}
		}
	}
	root = id_node[0];
	infile.close();

	// 打印每个树节点的信息
	//map<int, DecisionTreeNode*>::iterator iter;
	//for (iter = id_node.begin(); iter != id_node.end(); iter++) {
	//	iter->second->printTreeNode();
	//}

}
void DecisionTree::readCostMessage(string inFileName) {
	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	string line;
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, "\t");
		attribute_cost[atoi(temp_vector[0].c_str())] = make_pair(atof(temp_vector[1].c_str()), atof(temp_vector[2].c_str()));
	}
	infile.close();

}
double DecisionTree::averageDepthOfLeaves() {
	double average = 0;
	double s1 = 0, s2 = 0;
	int n = leaves.size();
	for (int i = 0; i < n; i++) {
		average += (leaves[i]->level - average) / ((double)i + 1);
		s1 += (double)leaves[i]->level * leaves[i]->level;
		s2 += leaves[i]->level;
	}

	cout << "total tree node number: " << id_node.size() << endl;
	cout << "leaf node number: " << leaves.size() << endl;
	cout << "not-leaf node number: " << id_node.size() - leaves.size() << endl;
	cout << "mean depth of leaves: " << average << endl;
	cout << "variance depth of leaves: " << s1 / n - (s2 / n) * (s2 / n) << endl;

	return average;
}

void DecisionTree::setCostTrans(map<int, int> cost_trans_val) {
	cost_trans = cost_trans_val;
}

// 简单验证渐进计算是否有效时的函数
void DecisionTree::traditionalDecision(string inFileName) {
	cout.precision(10);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");

		// 传统计算模式的话，必须传输并计算检测对象的全部属性
		vector<double> sample_attributes;
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());
		for (int i = 0; i < temp_vector.size() - 1; i++) {
			sample_attributes.push_back(atof(temp_vector[i].c_str()));
			transmission_cost += attribute_cost[i].first;
			computation_cost += attribute_cost[i].second;
		}
		
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification  == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
	}
	infile.close();

	cout << "-------------- Traditional --------------" << endl;
	cout << "sample number: " << count << endl;
	cout << "Accracy: " << (double)accurate_number / count << endl;
	cout << "total transmission cost: " << transmission_cost << endl;
	cout << "total compution cost: " << computation_cost << endl;
	cout << "average transmission cost: " << transmission_cost / count << endl;
	cout << "average compution cost: " << computation_cost / count << endl;

}
void DecisionTree::incrementalDecision_no_prior(string inFileName) {
	cout.precision(10);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());
				transmission_cost += attribute_cost[node_attribute].first;
				computation_cost += attribute_cost[node_attribute].second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
	}
	infile.close();

	cout << "-------------- Incremental without prior information --------------" << endl;
	cout << "sample number: " << count << endl;
	cout << "Accracy: " << (double)accurate_number / count << endl;
	cout << "total transmission cost: " << transmission_cost << endl;
	cout << "total compution cost: " << computation_cost << endl;
	cout << "average transmission cost: " << transmission_cost / count << endl;
	cout << "average compution cost: " << computation_cost / count << endl;
}
void DecisionTree::incrementalDecision_with_known_attribute(std::string inFileName, std::vector<int> known_attributes, bool random, int k) {
	cout.precision(10);
	srand(0);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 已知属性
		map<int, double> sample_attributes;
		if (random) { // 若每个样本随机选择已知的属性
			vector<int> randomVec;
			int n = temp_vector.size() - 1;
			for (int i = 0; i < n; i++) {
				randomVec.push_back(i);
			}
			random_shuffle(randomVec.begin(), randomVec.end());
			for (int i = 0; i < k; i++) {
				sample_attributes[randomVec[i]] = atof(temp_vector[randomVec[i]].c_str());
			}
		}
		else { // 若所有样本的已知属性是固定的
			for (int i = 0; i < known_attributes.size(); i++) {
				sample_attributes[known_attributes[i]] = atof(temp_vector[known_attributes[i]].c_str());
			}
		}
		
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());
				transmission_cost += attribute_cost[node_attribute].first;
				computation_cost += attribute_cost[node_attribute].second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
	}
	infile.close();

	cout << "-------------- Incremental with known attribute --------------" << endl;
	cout << "sample number: " << count << endl;
	cout << "Accracy: " << (double)accurate_number / count << endl;
	cout << "total transmission cost: " << transmission_cost << endl;
	cout << "total compution cost: " << computation_cost << endl;
	cout << "average transmission cost: " << transmission_cost / count << endl;
	cout << "average compution cost: " << computation_cost / count << endl;
}


// 传统、渐进都使用自身电池能量（传统渐进都用电池），battery-power
double DecisionTree::traditionalDecision_Battery(string inFileName, SimpleNetwork network) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0;

	string line;
	while (getline(infile, line)) { // 939
		count++;
		vector<string> temp_vector = split(line, "\t");

		// 传统计算模式的话，必须传输并计算检测对象的全部属性
		vector<double> sample_attributes;
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());
		for (int i = 0; i < temp_vector.size() - 1; i++) {
			sample_attributes.push_back(atof(temp_vector[i].c_str()));
		}

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		for (int i = 0; i < sample_attributes.size(); i++) {
			Sensor sensor = Sensor(0, 0);
			transmission_cost += sensor.energyTransTradition(cost_trans[i], 1);
			computation_cost += 1;
		}
		
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == BATTERY_POWER_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- Battert-power: Traditional --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::incrementalDecision_no_prior_self_control_Battery(string inFileName, SimpleNetwork network) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				// 按需收集属性的代价，使用电池，且sensor自身提供接收控制包的能量
				Sensor sensor = Sensor(0, 0);
				transmission_cost += sensor.energyTransProg_WithControl(cost_trans[node_attribute]);
				computation_cost += 1;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == BATTERY_POWER_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- battery-power: incremental without prior information self control energy --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::incrementalDecision_no_prior_RF_control_Battery(string inFileName, SimpleNetwork network) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价
	double cost_prog_delay = 0; // RF充电的时延

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				// 需要RF充电桩唤醒工作的sensor
				vector<string> charging_sensors = network.object_attribute_sensor[object_id][node_attribute];
				cost_prog_delay += network.cost_control_time(charging_sensors);

				// 按需收集属性的代价，使用电池，RF充电桩提供接收控制包的能量
				Sensor sensor = Sensor(0, 0);
				transmission_cost += sensor.energyTransProg_NoControl(cost_trans[node_attribute]);
				computation_cost += 1;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == BATTERY_POWER_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- Battery-power: Incremental without prior information RF control energy --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "total delay time: " << cost_prog_delay << "s" << endl;
		cout << "average transmission cost: " << transmission_cost / count << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
		cout << "average delay time: " << cost_prog_delay / count << "s" << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::incrementalDecision_random_known_attribute_self_control_Battery(string inFileName, SimpleNetwork network, vector<int> known_attributes) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes; // 无论需不需要用到，都先随机生成一些随机属性

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == BATTERY_POWER_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];
		vector<int> random_attribute = random_attributes[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 已知属性
		map<int, double> sample_attributes;
		if (RANDOM_KNOWN_ATTRIBUTE) { // 若每个样本随机选择已知的属性
			for (int i = 0; i < random_attribute.size(); i++) {
				sample_attributes[random_attribute[i]] = atof(temp_vector[random_attribute[i]].c_str());
			}
		}
		else { // 若所有样本的已知属性是固定的
			for (int i = 0; i < known_attributes.size(); i++) {
				sample_attributes[known_attributes[i]] = atof(temp_vector[known_attributes[i]].c_str());
			}
		}

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				Sensor sensor = Sensor(0, 0);
				transmission_cost += sensor.energyTransProg_WithControl(cost_trans[node_attribute]);
				computation_cost += 1;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- Battert-power: Incremental with ";
		if (RANDOM_KNOWN_ATTRIBUTE) {
			cout << "random " << RANDOM_KNOWN_ATTRIBUTE_NUMBER;
		}
		else {
			cout << "[";
			for (int i = 0; i < known_attributes.size(); i++) {
				cout << known_attributes[i] << " ";
			}
			cout << "]";
		}
		cout << " known attribute self control energy --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::incrementalDecision_random_known_attribute_RF_control_Battery(string inFileName, SimpleNetwork network, vector<int> known_attributes) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价
	double cost_prog_delay = 0; // RF充电的时延

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes; // 无论需不需要用到，都先随机生成一些随机属性

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == BATTERY_POWER_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];
		vector<int> random_attribute = random_attributes[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 已知属性
		map<int, double> sample_attributes;
		if (RANDOM_KNOWN_ATTRIBUTE) { // 若每个样本随机选择已知的属性
			for (int i = 0; i < random_attribute.size(); i++) {
				sample_attributes[random_attribute[i]] = atof(temp_vector[random_attribute[i]].c_str());
			}
		}
		else { // 若所有样本的已知属性是固定的
			for (int i = 0; i < known_attributes.size(); i++) {
				sample_attributes[known_attributes[i]] = atof(temp_vector[known_attributes[i]].c_str());
			}
		}

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				// 需要RF充电桩唤醒工作的sensor
				vector<string> charging_sensors = network.object_attribute_sensor[object_id][node_attribute];
				cost_prog_delay += network.cost_control_time(charging_sensors);

				Sensor sensor = Sensor(0, 0);
				transmission_cost += sensor.energyTransProg_NoControl(cost_trans[node_attribute]);
				computation_cost += 1;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- Battert-power: Incremental with ";
		if (RANDOM_KNOWN_ATTRIBUTE) {
			cout << "random " << RANDOM_KNOWN_ATTRIBUTE_NUMBER;
		}
		else {
			cout << "[";
			for (int i = 0; i < known_attributes.size(); i++) {
				cout << known_attributes[i] << " ";
			}
			cout << "]";
		}
		cout << " known attribute RF control energy --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "total delay time: " << cost_prog_delay << "s" << endl;
		cout << "average transmission cost: " << transmission_cost / count << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
		cout << "average delay time: " << cost_prog_delay / count << "s" << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::battery_power_known_attribute_new_self_control(string inFileName, SimpleNetwork network, vector<vector<int>> guided_attributes) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		map<int, double> sample_attributes; // 当前已经知道的属性
		// 根据保质期产生已知属性
		vector<int> known_attributes = network.generate_known_attributes(object_id);
		for (int i = 0; i < known_attributes.size(); i++) {
			sample_attributes[known_attributes[i]] = atof(temp_vector[known_attributes[i]].c_str());
		}

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				// 按需收集属性的代价，使用电池，且sensor自身提供接收控制包的能量
				Sensor sensor = Sensor(0, 0);
				transmission_cost += cost_trans[node_attribute] * COST_TRANS_UNIT_8 + COST_TRANS_INITIAL + COST_TRANS_CONTROL;
				computation_cost += 1;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}

			// guided schedule
			if (guided_attributes.size() > 0) {
				bool flag1 = false;
				for (int i = 0; i < guided_attributes.size(); i++) {
					vector<int> guided_temp_vector = guided_attributes[i];
					bool flag2 = true;
					for (int j = 0; j < guided_temp_vector.size(); j++) {
						if (sample_attributes.count(guided_temp_vector[j]) == 0) {
							flag2 = false;
						}
					}
					if (flag2) {
						flag1 = true;
						break;
					}
				}
				if (flag1) break;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == BATTERY_POWER_SAMPLE_NUMBER) break;
	}
	infile.close();
	return transmission_cost / (count);
}
double DecisionTree::battery_power_known_attribute_new_RF_control(string inFileName, SimpleNetwork network, vector<vector<int>> guided_attributes) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		map<int, double> sample_attributes; // 当前已经知道的属性
		// 根据保质期产生已知属性
		vector<int> known_attributes = network.generate_known_attributes(object_id);
		for (int i = 0; i < known_attributes.size(); i++) {
			sample_attributes[known_attributes[i]] = atof(temp_vector[known_attributes[i]].c_str());
		}

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				// 按需收集属性的代价，使用电池，且sensor自身提供接收控制包的能量
				Sensor sensor = Sensor(0, 0);
				transmission_cost += cost_trans[node_attribute] * COST_TRANS_UNIT_8 + COST_TRANS_INITIAL;
				computation_cost += 1;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}

			// guided schedule
			if (guided_attributes.size() > 0) {
				bool flag1 = false;
				for (int i = 0; i < guided_attributes.size(); i++) {
					vector<int> guided_temp_vector = guided_attributes[i];
					bool flag2 = true;
					for (int j = 0; j < guided_temp_vector.size(); j++) {
						if (sample_attributes.count(guided_temp_vector[j]) == 0) {
							flag2 = false;
						}
					}
					if (flag2) {
						flag1 = true;
						break;
					}
				}
				if (flag1) break;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == BATTERY_POWER_SAMPLE_NUMBER) break;
	}
	infile.close();
	return transmission_cost / (count);
}


// 纯 RF-charging，传统型和渐进计算全都使用RF充电！
double DecisionTree::RF_charging_tradition_1(string inFileName, SimpleNetwork network, int trans_size) { // 每个object的所有 sensor 一起调度
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");

		vector<double> sample_attributes; // 既然所有属性都要获得的话，直接用 vector 就可以了
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());
		for (int i = 0; i < temp_vector.size() - 1; i++) {
			sample_attributes.push_back(atof(temp_vector[i].c_str()));
		}

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		// 计算充电桩充电的代价，传输代价用时间来表示，需要监测该object全部属性的sensor及其传输需要的能量
		vector<pair<string, double>> sensor_id_energyRequired;
		for (int i = 0; i < sample_attributes.size(); i++) {
			vector<string> temp_sensors = network.object_attribute_sensor[object_id][i];
			for (int j = 0; j < temp_sensors.size(); j++) {
				sensor_id_energyRequired.push_back(make_pair(temp_sensors[j], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
			}
		}

		// 从网络中获取代价
		pair<double, double> cost = network.rf_charging_cost(sensor_id_energyRequired);
		transmission_cost += cost.first;
		computation_cost += cost.second;
		
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == RF_CHARGING_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- RF-charging: Traditional --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::RF_charging_incremental(string inFileName, SimpleNetwork network, int trans_size) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				vector<pair<string, double>> sensor_id_energyRequired;
				vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
				for (int i = 0; i < temp_sensors.size(); i++) {
					sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
				}

				pair<double, double> cost = network.rf_charging_cost(sensor_id_energyRequired);
				transmission_cost += cost.first;
				computation_cost += cost.second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == RF_CHARGING_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- RF-charging: Incremental without prior information --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::RF_charging_incremental_with_known_attribute(string inFileName, SimpleNetwork network, vector<int> known_attributes, int trans_size) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价
	
	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes; // 无论需不需要用到，都先随机生成一些随机属性

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids .push_back(OBJECT_ID_FIXED);
		}
		
		if (count == RF_CHARGING_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	//for (int i = 0; i < object_ids.size(); i++) {
	//	cout << object_ids[i] << "  ";
	//}
	//cout << endl;
	//for (int i = 0; i < random_attributes.size(); i++) {
	//	vector<int> random_attribute = random_attributes[i];
	//	for (int j = 0; j < random_attribute.size(); j++) {
	//		cout << random_attribute[j] << "  ";
	//	}
	//	cout << endl;
	//}

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];
		vector<int> random_attribute = random_attributes[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 已知属性
		map<int, double> sample_attributes;
		if (RANDOM_KNOWN_ATTRIBUTE) { // 若每个样本随机选择已知的属性
			for (int i = 0; i < random_attribute.size(); i++) {
				sample_attributes[random_attribute[i]] = atof(temp_vector[random_attribute[i]].c_str());
			}
		}
		else { // 若所有样本的已知属性是固定的
			for (int i = 0; i < known_attributes.size(); i++) {
				sample_attributes[known_attributes[i]] = atof(temp_vector[known_attributes[i]].c_str());
			}
		}

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				vector<pair<string, double>> sensor_id_energyRequired;
				vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
				for (int i = 0; i < temp_sensors.size(); i++) {
					sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
				}

				pair<double, double> cost = network.rf_charging_cost(sensor_id_energyRequired);
				transmission_cost += cost.first;
				computation_cost += cost.second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- RF-charging: Incremental with ";
		if (RANDOM_KNOWN_ATTRIBUTE) {
			cout << "random " << RANDOM_KNOWN_ATTRIBUTE_NUMBER;
		}
		else {
			cout << "[";
			for (int i = 0; i < known_attributes.size(); i++) {
				cout << known_attributes[i] << " ";
			}
			cout << "]";
		}
		cout << " known attribute --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::RF_charging_known_attribute_new(string inFileName, SimpleNetwork network, int trans_size) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				double temp_time = network.sensors[network.object_attribute_sensor[object_id][node_attribute][0]].life_time;
				if (temp_time >= 0 && temp_time < LIFE_TIME) {
					// 如果在保质期之内，则不需要充电，不消耗时间
				}
				else {
					// 不在保质期之内，需要充电，且更新所有sensor的保质期
					vector<pair<string, double>> sensor_id_energyRequired;
					vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
					for (int i = 0; i < temp_sensors.size(); i++) {
						sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
					}

					pair<double, double> cost = network.rf_charging_cost(sensor_id_energyRequired);
					transmission_cost += cost.first;
					computation_cost += cost.second;
					network.update_life_time(temp_sensors, cost.first);
				}
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == RF_CHARGING_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- RF-charging: Incremental without prior information --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}


// energy-harvest，传统型和渐进都使用光能充电
double DecisionTree::energy_harvest_tradition(string inFileName, SimpleNetwork network, double power_min, double power_max, int trans_size, int seed) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes;
	vector<double> time_intervals;

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == ENERGY_HARVEST_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	// 随机生成记录之间的时间间隔
	for (int i = 0; i < file_content.size(); i++) {
		double random_time = ((double)rand() / RAND_MAX) * (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MAX - ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN) + ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN;
		time_intervals.push_back(random_time);
	}

	srand(seed);
	// 每条记录处理时，给所有的sensor随机出接收功率
	map<string, double> power_received;
	for (map<string, Sensor>::iterator iter = network.sensors.begin(); iter != network.sensors.end(); iter++) {
		double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
		power_received[iter->first] = temp_power * 1000; // 转换成 uW
	}

	//for (int i = 0; i < object_ids.size(); i++) {
	//	cout << object_ids[i] << "  ";
	//}
	//cout << endl;
	//for (int i = 0; i < random_attributes.size(); i++) {
	//	vector<int> random_attribute = random_attributes[i];
	//	for (int j = 0; j < random_attribute.size(); j++) {
	//		cout << random_attribute[j] << "  ";
	//	}
	//	cout << endl;
	//}
	//for (int i = 0; i < time_intervals.size(); i++) {
	//	cout << time_intervals[i] << ", ";
	//}
	//cout << endl;
	//for (map<string, double>::iterator iter = power_received.begin(); iter != power_received.end(); iter++) {
	//	cout << iter->second << "  ";
	//}
	//cout << endl;

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];	

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		vector<double> sample_attributes; // 既然所有属性都要获得的话，直接用 vector 就可以了
		for (int i = 0; i < temp_vector.size() - 1; i++) {
			sample_attributes.push_back(atof(temp_vector[i].c_str()));
		}

		// 计算充电桩充电的代价，传输代价用时间来表示，需要监测该object全部属性的sensor及其传输需要的能量
		vector<pair<string, double>> sensor_id_energyRequired;
		for (int i = 0; i < sample_attributes.size(); i++) {
			vector<string> temp_sensors = network.object_attribute_sensor[object_id][i];
			for (int j = 0; j < temp_sensors.size(); j++) {
				sensor_id_energyRequired.push_back(make_pair(temp_sensors[j], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
			}
		}

		// 从网络中获取代价
		pair<double, double> cost = network.energy_harvest_cost(sensor_id_energyRequired, power_received, power_min, power_max);
		transmission_cost += cost.first;
		computation_cost += cost.second;
		
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}

		// 每两条记录之间的时间间隔，这段时间要充电
		if (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL) {
			network.energy_harvest_receive_energy(time_intervals[line_number], power_received, power_min, power_max);
		}
		else {
			network.energy_harvest_receive_energy(ENERGY_HARVEST_RECORD_TIME_INTERVAL, power_received, power_min, power_max);
		}
        now_node->printTreeNode();
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- energy-harvest: Traditional ";
		cout << "[" << power_min << ", " << power_max << "] --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::energy_harvest_incremental(string inFileName, SimpleNetwork network, double power_min, double power_max, int trans_size, int seed) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes;
	vector<double> time_intervals;

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == ENERGY_HARVEST_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	// 随机生成记录之间的时间间隔
	for (int i = 0; i < file_content.size(); i++) {
		double random_time = ((double)rand() / RAND_MAX) * (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MAX - ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN) + ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN;
		time_intervals.push_back(random_time);
	}

	srand(seed);
	// 每条记录处理时，给所有的sensor随机出接收功率
	map<string, double> power_received;
	for (map<string, Sensor>::iterator iter = network.sensors.begin(); iter != network.sensors.end(); iter++) {
		double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
		power_received[iter->first] = temp_power * 1000; // 转换成 uW
	}

	//for (int i = 0; i < object_ids.size(); i++) {
	//	cout << object_ids[i] << "  ";
	//}
	//cout << endl;
	//for (int i = 0; i < random_attributes.size(); i++) {
	//	vector<int> random_attribute = random_attributes[i];
	//	for (int j = 0; j < random_attribute.size(); j++) {
	//		cout << random_attribute[j] << "  ";
	//	}
	//	cout << endl;
	//}
	//for (int i = 0; i < time_intervals.size(); i++) {
	//	cout << time_intervals[i] << ", ";
	//}
	//cout << endl;
	//for (map<string, double>::iterator iter = power_received.begin(); iter != power_received.end(); iter++) {
	//	cout << iter->second << "  ";
	//}
	//cout << endl;

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				vector<pair<string, double>> sensor_id_energyRequired;
				vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
				for (int i = 0; i < temp_sensors.size(); i++) {
					sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
				}

				pair<double, double> cost = network.energy_harvest_cost(sensor_id_energyRequired, power_received, power_min, power_max);
				transmission_cost += cost.first;
				computation_cost += cost.second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();

		// 每两条记录之间的时间间隔，这段时间要充电
		if (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL) {
			network.energy_harvest_receive_energy(time_intervals[line_number], power_received, power_min, power_max);
		}
		else {
			network.energy_harvest_receive_energy(ENERGY_HARVEST_RECORD_TIME_INTERVAL, power_received, power_min, power_max);
		}
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- energy-harvest: Incremental without prior information ";
		cout << "[" << power_min << ", " << power_max << "] --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::energy_harvest_incremental_with_known_attribute(string inFileName, SimpleNetwork network, vector<int> known_attributes, double power_min, double power_max, int trans_size, int seed) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes;
	vector<double> time_intervals;

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == ENERGY_HARVEST_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	// 随机生成记录之间的时间间隔
	for (int i = 0; i < file_content.size(); i++) {
		double random_time = ((double)rand() / RAND_MAX) * (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MAX - ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN) + ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN;
		time_intervals.push_back(random_time);
	}

	srand(seed);
	// 每条记录处理时，给所有的sensor随机出接收功率
	map<string, double> power_received;
	for (map<string, Sensor>::iterator iter = network.sensors.begin(); iter != network.sensors.end(); iter++) {
		double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
		power_received[iter->first] = temp_power * 1000; // 转换成 uW
	}

	//for (int i = 0; i < object_ids.size(); i++) {
	//	cout << object_ids[i] << "  ";
	//}
	//cout << endl;
	//for (int i = 0; i < random_attributes.size(); i++) {
	//	vector<int> random_attribute = random_attributes[i];
	//	for (int j = 0; j < random_attribute.size(); j++) {
	//		cout << random_attribute[j] << "  ";
	//	}
	//	cout << endl;
	//}
	//for (int i = 0; i < time_intervals.size(); i++) {
	//	cout << time_intervals[i] << ", ";
	//}
	//cout << endl;
	//for (map<string, double>::iterator iter = power_received.begin(); iter != power_received.end(); iter++) {
	//	cout << iter->second << "  ";
	//}
	//cout << endl;

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];
		vector<int> random_attribute = random_attributes[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 已知属性
		map<int, double> sample_attributes;
		if (RANDOM_KNOWN_ATTRIBUTE) { // 若每个样本随机选择已知的属性
			for (int i = 0; i < random_attribute.size(); i++) {
				sample_attributes[random_attribute[i]] = atof(temp_vector[random_attribute[i]].c_str());
			}
		}
		else { // 若所有样本的已知属性是固定的
			for (int i = 0; i < known_attributes.size(); i++) {
				sample_attributes[known_attributes[i]] = atof(temp_vector[known_attributes[i]].c_str());
			}
		}

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				vector<pair<string, double>> sensor_id_energyRequired;
				vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
				for (int i = 0; i < temp_sensors.size(); i++) {
					sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
				}

				pair<double, double> cost = network.energy_harvest_cost(sensor_id_energyRequired, power_received, power_min, power_max);
				transmission_cost += cost.first;
				computation_cost += cost.second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();

		// 每两条记录之间的时间间隔，这段时间要充电
		if (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL) {
			network.energy_harvest_receive_energy(time_intervals[line_number], power_received, power_min, power_max);
		}
		else {
			network.energy_harvest_receive_energy(ENERGY_HARVEST_RECORD_TIME_INTERVAL, power_received, power_min, power_max);
		}
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- energy-harvest: Incremental with ";
		if (RANDOM_KNOWN_ATTRIBUTE) {
			cout << "random " << RANDOM_KNOWN_ATTRIBUTE_NUMBER;
		}
		else {
			cout << "[";
			for (int i = 0; i < known_attributes.size(); i++) {
				cout << known_attributes[i] << " ";
			}
			cout << "]";
		}
		cout << " known attribute [" << power_min << ", " << power_max << "] --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}
double DecisionTree::energy_harvest_known_attribute_new(string inFileName, SimpleNetwork network, double power_min, double power_max, int trans_size, int seed) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes;
	vector<double> time_intervals;

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == ENERGY_HARVEST_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	// 随机生成记录之间的时间间隔
	for (int i = 0; i < file_content.size(); i++) {
		double random_time = ((double)rand() / RAND_MAX) * (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MAX - ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN) + ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN;
		time_intervals.push_back(random_time);
	}

	srand(seed);
	// 每条记录处理时，给所有的sensor随机出接收功率
	map<string, double> power_received;
	for (map<string, Sensor>::iterator iter = network.sensors.begin(); iter != network.sensors.end(); iter++) {
		double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
		power_received[iter->first] = temp_power * 1000; // 转换成 uW
	}

	//for (int i = 0; i < object_ids.size(); i++) {
	//	cout << object_ids[i] << "  ";
	//}
	//cout << endl;
	//for (int i = 0; i < random_attributes.size(); i++) {
	//	vector<int> random_attribute = random_attributes[i];
	//	for (int j = 0; j < random_attribute.size(); j++) {
	//		cout << random_attribute[j] << "  ";
	//	}
	//	cout << endl;
	//}
	//for (int i = 0; i < time_intervals.size(); i++) {
	//	cout << time_intervals[i] << ", ";
	//}
	//cout << endl;
	//for (map<string, double>::iterator iter = power_received.begin(); iter != power_received.end(); iter++) {
	//	cout << iter->second << "  ";
	//}
	//cout << endl;

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				double temp_time = network.sensors[network.object_attribute_sensor[object_id][node_attribute][0]].life_time;
				if (temp_time >= 0 && temp_time < LIFE_TIME) {
					// 如果在保质期之内，则不需要充电，不消耗时间
				}
				else {
					// 不在保质期之内，需要充电，且更新所有sensor的保质期
					vector<pair<string, double>> sensor_id_energyRequired;
					vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
					for (int i = 0; i < temp_sensors.size(); i++) {
						sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * trans_size));
					}

					pair<double, double> cost = network.energy_harvest_cost(sensor_id_energyRequired, power_received, power_min, power_max);
					transmission_cost += cost.first;
					computation_cost += cost.second;
					network.update_life_time(temp_sensors, cost.first);
				}
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();

		// 每两条记录之间的时间间隔，这段时间要充电
		if (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL) {
			network.energy_harvest_receive_energy(time_intervals[line_number], power_received, power_min, power_max);
		}
		else {
			network.energy_harvest_receive_energy(ENERGY_HARVEST_RECORD_TIME_INTERVAL, power_received, power_min, power_max);
		}
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- energy-harvest: Incremental without prior information ";
		cout << "[" << power_min << ", " << power_max << "] --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}


double DecisionTree::new_3_RF_charging_tradition(string inFileName, SimpleNetwork network) { // 每个object的所有 sensor 一起调度
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");

		vector<double> sample_attributes; // 既然所有属性都要获得的话，直接用 vector 就可以了
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());
		for (int i = 0; i < temp_vector.size() - 1; i++) {
			sample_attributes.push_back(atof(temp_vector[i].c_str()));
		}

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		// 计算充电桩充电的代价，传输代价用时间来表示，需要监测该object全部属性的sensor及其传输需要的能量
		vector<pair<string, double>> sensor_id_energyRequired;
		for (int i = 0; i < sample_attributes.size(); i++) {
			vector<string> temp_sensors = network.object_attribute_sensor[object_id][i];
			for (int j = 0; j < temp_sensors.size(); j++) {
				sensor_id_energyRequired.push_back(make_pair(temp_sensors[j], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * cost_trans[i]));
			}
		}

		// 从网络中获取代价
		pair<double, double> cost = network.rf_charging_cost(sensor_id_energyRequired);
		transmission_cost += cost.first;
		computation_cost += cost.second;

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == RF_CHARGING_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- RF-charging: Traditional --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}

double DecisionTree::new_3_RF_charging_incremental(string inFileName, SimpleNetwork network) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	string line;
	while (getline(infile, line)) {
		count++;
		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的
		string object_id;
		if (RANDOM_OBJECT) {
			int object_number = network.objects.size();
			object_id = "o" + to_string(rand() % object_number); // 随机选择网络中的 object
		}
		else {
			object_id = OBJECT_ID_FIXED;
		}

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				vector<pair<string, double>> sensor_id_energyRequired;
				vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
				for (int i = 0; i < temp_sensors.size(); i++) {
					sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * cost_trans[node_attribute]));
				}

				pair<double, double> cost = network.rf_charging_cost(sensor_id_energyRequired);
				transmission_cost += cost.first;
				computation_cost += cost.second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();
		if (count == RF_CHARGING_SAMPLE_NUMBER) break;
	}
	infile.close();

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- RF-charging: Incremental without prior information --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}

double DecisionTree::new_3_energy_harvest_tradition(string inFileName, SimpleNetwork network, double power_min, double power_max, int seed) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes;
	vector<double> time_intervals;

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == ENERGY_HARVEST_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	// 随机生成记录之间的时间间隔
	for (int i = 0; i < file_content.size(); i++) {
		double random_time = ((double)rand() / RAND_MAX) * (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MAX - ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN) + ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN;
		time_intervals.push_back(random_time);
	}

	srand(seed);
	// 每条记录处理时，给所有的sensor随机出接收功率
	map<string, double> power_received;
	for (map<string, Sensor>::iterator iter = network.sensors.begin(); iter != network.sensors.end(); iter++) {
		double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
		power_received[iter->first] = temp_power * 1000; // 转换成 uW
	}

	//for (int i = 0; i < object_ids.size(); i++) {
	//	cout << object_ids[i] << "  ";
	//}
	//cout << endl;
	//for (int i = 0; i < random_attributes.size(); i++) {
	//	vector<int> random_attribute = random_attributes[i];
	//	for (int j = 0; j < random_attribute.size(); j++) {
	//		cout << random_attribute[j] << "  ";
	//	}
	//	cout << endl;
	//}
	//for (int i = 0; i < time_intervals.size(); i++) {
	//	cout << time_intervals[i] << ", ";
	//}
	//cout << endl;
	//for (map<string, double>::iterator iter = power_received.begin(); iter != power_received.end(); iter++) {
	//	cout << iter->second << "  ";
	//}
	//cout << endl;

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		vector<double> sample_attributes; // 既然所有属性都要获得的话，直接用 vector 就可以了
		for (int i = 0; i < temp_vector.size() - 1; i++) {
			sample_attributes.push_back(atof(temp_vector[i].c_str()));
		}

		// 计算充电桩充电的代价，传输代价用时间来表示，需要监测该object全部属性的sensor及其传输需要的能量
		vector<pair<string, double>> sensor_id_energyRequired;
		for (int i = 0; i < sample_attributes.size(); i++) {
			vector<string> temp_sensors = network.object_attribute_sensor[object_id][i];
			for (int j = 0; j < temp_sensors.size(); j++) {
				sensor_id_energyRequired.push_back(make_pair(temp_sensors[j], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * cost_trans[i]));
			}
		}

		// 从网络中获取代价
		pair<double, double> cost = network.energy_harvest_cost(sensor_id_energyRequired, power_received, power_min, power_max);
		transmission_cost += cost.first;
		computation_cost += cost.second;

		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();

		// 每两条记录之间的时间间隔，这段时间要充电
		if (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL) {
			network.energy_harvest_receive_energy(time_intervals[line_number], power_received, power_min, power_max);
		}
		else {
			network.energy_harvest_receive_energy(ENERGY_HARVEST_RECORD_TIME_INTERVAL, power_received, power_min, power_max);
		}
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- energy-harvest: Traditional ";
		cout << "[" << power_min << ", " << power_max << "] --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}

double DecisionTree::new_3_energy_harvest_incremental(string inFileName, SimpleNetwork network, double power_min, double power_max, int seed) {
	cout.precision(10);
	srand(RANDOM_SEED);

	ifstream infile;
	infile.open(inFileName);
	if (!infile.is_open()) {
		cout << "Open " << inFileName << " fail!" << endl;
	}

	int count = 0;
	int accurate_number = 0;
	double transmission_cost = 0; // 传输总代价
	double computation_cost = 0; // 计算总代价

	vector<string> file_content;
	vector<string> object_ids;
	vector<vector<int>> random_attributes;
	vector<double> time_intervals;

	string line;
	while (getline(infile, line)) {
		count++;
		file_content.push_back(line);

		// 从异构网络中选择一个 object，假设此数据就是由该object采集来的，无论是不是随机的object，都要先随机生成一组随机的object_id
		int object_number = network.objects.size();
		string object_id = "o" + to_string(rand() % object_number);
		if (RANDOM_OBJECT) {
			object_ids.push_back(object_id);
		}
		else {
			object_ids.push_back(OBJECT_ID_FIXED);
		}

		if (count == ENERGY_HARVEST_SAMPLE_NUMBER) break;
	}
	infile.close();

	// 随机生成一些 已知属性，提前生成，不管需不需要，都提前生成！
	int attribute_number = split(file_content[0], "\t").size() - 1;
	for (int i = 0; i < file_content.size(); i++) {
		vector<int> randomVec;
		for (int j = 0; j < attribute_number; j++) {
			randomVec.push_back(j);
		}
		random_shuffle(randomVec.begin(), randomVec.end());

		vector<int> temp_ramdom_attribute;
		for (int j = 0; j < RANDOM_KNOWN_ATTRIBUTE_NUMBER; j++) {
			temp_ramdom_attribute.push_back(randomVec[j]);
		}
		random_attributes.push_back(temp_ramdom_attribute);
	}

	// 随机生成记录之间的时间间隔
	for (int i = 0; i < file_content.size(); i++) {
		double random_time = ((double)rand() / RAND_MAX) * (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MAX - ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN) + ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN;
		time_intervals.push_back(random_time);
	}

	srand(seed);
	// 每条记录处理时，给所有的sensor随机出接收功率
	map<string, double> power_received;
	for (map<string, Sensor>::iterator iter = network.sensors.begin(); iter != network.sensors.end(); iter++) {
		double temp_power = ((double)rand() / RAND_MAX) * (power_max - power_min) + power_min; // 这里的单位是 mW
		power_received[iter->first] = temp_power * 1000; // 转换成 uW
	}

	//for (int i = 0; i < object_ids.size(); i++) {
	//	cout << object_ids[i] << "  ";
	//}
	//cout << endl;
	//for (int i = 0; i < random_attributes.size(); i++) {
	//	vector<int> random_attribute = random_attributes[i];
	//	for (int j = 0; j < random_attribute.size(); j++) {
	//		cout << random_attribute[j] << "  ";
	//	}
	//	cout << endl;
	//}
	//for (int i = 0; i < time_intervals.size(); i++) {
	//	cout << time_intervals[i] << ", ";
	//}
	//cout << endl;
	//for (map<string, double>::iterator iter = power_received.begin(); iter != power_received.end(); iter++) {
	//	cout << iter->second << "  ";
	//}
	//cout << endl;

	for (int line_number = 0; line_number < file_content.size(); line_number++) {
		line = file_content[line_number];
		string object_id = object_ids[line_number];

		vector<string> temp_vector = split(line, "\t");
		int sample_classification = atoi(temp_vector[temp_vector.size() - 1].c_str());

		map<int, double> sample_attributes;
		DecisionTreeNode* now_node = root;
		while (now_node->isLeaf != true) {
			int node_attribute = now_node->divide_attribute;
			double node_value = now_node->divide_value;

			// 如果没有传输并处理过当前检测目标的该属性，按需收集属性
			if (sample_attributes.count(node_attribute) == 0) {
				sample_attributes[node_attribute] = atof(temp_vector[node_attribute].c_str());

				vector<pair<string, double>> sensor_id_energyRequired;
				vector<string> temp_sensors = network.object_attribute_sensor[object_id][node_attribute];
				for (int i = 0; i < temp_sensors.size(); i++) {
					sensor_id_energyRequired.push_back(make_pair(temp_sensors[i], COST_TRANS_INITIAL + COST_TRANS_UNIT_8 * cost_trans[node_attribute]));
				}

				pair<double, double> cost = network.energy_harvest_cost(sensor_id_energyRequired, power_received, power_min, power_max);
				transmission_cost += cost.first;
				computation_cost += cost.second;
			}

			if (sample_attributes[node_attribute] <= node_value) {
				now_node = now_node->true_node;
			}
			else {
				now_node = now_node->false_node;
			}
		}
		if (now_node->classification == sample_classification) {
			accurate_number++;
		}
        now_node->printTreeNode();

		// 每两条记录之间的时间间隔，这段时间要充电
		if (ENERGY_HARVEST_RECORD_RANDOM_INTERVAL) {
			network.energy_harvest_receive_energy(time_intervals[line_number], power_received, power_min, power_max);
		}
		else {
			network.energy_harvest_receive_energy(ENERGY_HARVEST_RECORD_TIME_INTERVAL, power_received, power_min, power_max);
		}
	}

	if (PRINT_COST_MESSAGE) {
		cout << "-------------- energy-harvest: Incremental without prior information ";
		cout << "[" << power_min << ", " << power_max << "] --------------" << endl;
		cout << "sample number: " << count << endl;
		cout << "Accracy: " << (double)accurate_number / count << endl;
		cout << "total transmission cost: " << transmission_cost << "s" << endl;
		cout << "total compution cost: " << computation_cost << endl;
		cout << "average transmission cost: " << transmission_cost / count << "s" << endl;
		cout << "average compution cost: " << computation_cost / count << endl;
	}
	return transmission_cost / (count);
}