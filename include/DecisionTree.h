#pragma once
#include "DecisionTreeNode.h"
#include "SimpleNetwork.h"
#include <vector>
#include <string>
#include <map>

class DecisionTree
{
public:
	DecisionTreeNode* root = nullptr; // 决策树的根节点
	std::vector<DecisionTreeNode*> leaves; // 所有叶结点的指针
	std::map<int, DecisionTreeNode*> id_node;
	std::map<int, std::pair<double, double >> attribute_cost; // 简单验证时

	std::map<int, int> cost_trans; // 以防万一以后实验时，每个属性的传输量不同而且会变化（K不同且会变），可以将"每个属性的K"存在该变量中，K变时再传一次就好了
	void setCostTrans(std::map<int, int> cost_trans_val);

	void constructDecisionTree(std::string inFileName);
	void readCostMessage(std::string inFileName);
	double averageDepthOfLeaves();

	// 自定义传输以及处理代价的三种情况（简单验证）
	void traditionalDecision(std::string inFileName);
	void incrementalDecision_no_prior(std::string inFileName);
	void incrementalDecision_with_known_attribute(std::string inFileName, std::vector<int> known_attributes, bool random, int k); // 如果要随机选择每个样本已知属性的话，random传入true，k为随机属性个数

	// battery-power
	double traditionalDecision_Battery(std::string inFileName, SimpleNetwork network);
	double incrementalDecision_no_prior_self_control_Battery(std::string inFileName, SimpleNetwork network);
	double incrementalDecision_no_prior_RF_control_Battery(std::string inFileName, SimpleNetwork network);
	double incrementalDecision_random_known_attribute_self_control_Battery(std::string inFileName, SimpleNetwork network, std::vector<int> known_attributes);
	double incrementalDecision_random_known_attribute_RF_control_Battery(std::string inFileName, SimpleNetwork network, std::vector<int> known_attributes);
	double battery_power_known_attribute_new_self_control(std::string inFileName, SimpleNetwork network, std::vector<std::vector<int>> guided_attributes);
	double battery_power_known_attribute_new_RF_control(std::string inFileName, SimpleNetwork network, std::vector<std::vector<int>> guided_attributes);

	// RF-charging
	double RF_charging_tradition_1(std::string inFileName, SimpleNetwork network, int trans_size); // 同一个object的所有sensor批量调度
	double RF_charging_incremental(std::string inFileName, SimpleNetwork network, int trans_size); // 渐进型肯定是分开调度的
	double RF_charging_incremental_with_known_attribute(std::string inFileName, SimpleNetwork network, std::vector<int> known_attributes, int trans_size);
	double RF_charging_known_attribute_new(std::string inFileName, SimpleNetwork network, int trans_size);

	// energy-harvest
	double energy_harvest_tradition(std::string inFileName, SimpleNetwork network, double power_min, double power_max, int trans_size, int seed); // 同时调度object的全部sensor
	double energy_harvest_incremental(std::string inFileName, SimpleNetwork network, double power_min, double power_max, int trans_size, int seed);
	double energy_harvest_incremental_with_known_attribute(std::string inFileName, SimpleNetwork network, std::vector<int> known_attributes, double power_min, double power_max, int trans_size, int seed);
	double energy_harvest_known_attribute_new(std::string inFileName, SimpleNetwork network, double power_min, double power_max, int trans_size, int seed);

	// 新实验三
	double new_3_RF_charging_tradition(std::string inFileName, SimpleNetwork network);
	double new_3_RF_charging_incremental(std::string inFileName, SimpleNetwork network);
	double new_3_energy_harvest_tradition(std::string inFileName, SimpleNetwork network, double power_min, double power_max, int seed);
	double new_3_energy_harvest_incremental(std::string inFileName, SimpleNetwork network, double power_min, double power_max, int seed);

};
