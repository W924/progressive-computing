#include "Util.h"
#include "PreProcess.h"
#include "DecisionTree.h"
#include "SimpleNetwork.h"
#include "MultipleQuery.h"
#include "ConstDefine.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <strings.h>
#include <map>
#include <algorithm>
#include <iomanip>
#include <getopt.h>

using namespace std;

void original_main();
void print_help_informamtion();
MultipleQuery dataset_deal_query(const string& dataset);
DecisionTree dataset_deal_recognition(const string& dataset, string attribute);
SimpleNetwork get_network(const string& dataset, string attribute);
map<int, int> get_cost_trans(string file_path);
vector<string> split(string s, char a);
void function_execute_recognition_file(DecisionTree dt, string dataset, string file_path, int function_choice, SimpleNetwork network);
void function_execute_recognition_cmd(DecisionTree dt, string dataset, string query_data, int function_choice, SimpleNetwork network);
void function_execute_query_file(MultipleQuery mq, string dataset, string file_path, int function_choice);
void function_execute_query_cmd(MultipleQuery mq, string dataset, string query_data, string query_limits, int function_choice);
string replace(string s, char from, char to);

int main(int argc, char *argv[]) {
    // argv[1-]: requirement; dataset; input mode; input; function choice
    // argc should be no less than 6 currently, on 2021/6/27

//    original_main();

    if(argc < 2){
        cout << "Too few parameters. Use \"help\" to get more information." << endl;
        print_help_informamtion();
        return 0;
    }

    // choose function
    int recognition = strcasecmp(argv[1], "recognition");
    int query = strcasecmp(argv[1], "query");
    int help = strcasecmp(argv[1], "help");

    if(!help){
        print_help_informamtion();
        return 0;
    }

    // 单目标识别
    else if(!recognition){
        if(argc < 6) {
            cout << "Too few parameters. Use \"help\" to get more information." << endl;
            return 0;
        }

        string dataset = argv[2];
        DecisionTree dt = dataset_deal_recognition(dataset, "2-8");  // build, get 2 cost
        SimpleNetwork network = get_network(dataset, "2-8");
        
        // choose input mode
        int file_input = strcasecmp(argv[3], "-f");
        int cmd_input = strcasecmp(argv[3], "-c");

        // 文件输入
        if(!file_input){
            string file_path = argv[4];
            int function_choice = stoi(argv[5]);
            function_execute_recognition_file(dt, dataset, file_path, function_choice, network);  // todo: do this
        }

        // 命令行直接输入
        else if(!cmd_input){
            string params_str = argv[4];
            int function_choice = stoi(argv[5]);
            function_execute_recognition_cmd(dt, dataset, params_str, function_choice, network);  // todo: do this
        }
    }

    // 多目标查询
    else if(!query){
        if(argc < 6){
            cout << "Too few parameters. Use \"help\" to get more information." << endl;
            return 0;
        }

        string dataset = argv[2];
        MultipleQuery mq = dataset_deal_query(dataset);

        // choose input modeAi
        int file_input = strcasecmp(argv[3], "-f");
        int cmd_input = strcasecmp(argv[3], "-c");

        // 文件输入
        if(!file_input){
            string file_path = argv[4];
            int function_choice = stoi(argv[5]);
            function_execute_query_file(mq, dataset, file_path, function_choice);  // todo: do this
        }

        // 命令行直接输入
        else if(!cmd_input){
            string params_str = argv[4];
            string params_str2 = argv[5];
            int function_choice = stoi(argv[6]);
            function_execute_query_cmd(mq, dataset, params_str, params_str2, function_choice);  // todo: do this
        }
    }
}

void original_main(){
//    // 各个数据集的预处理
//
//    HT_Sensor_sample_average("./input/HT_Sensor/HT_Sensor_dataset", 10, 2000);
//    wilt_transfer_csv_to_txt("./input/wilt/training.csv");
    Air_preprocess("./input/AirQuality/CrawledData.txt");
//
//    // 简单验证 HT_Sensor 数据集，渐进计算是否有效
//
//	 DecisionTree dt;
//	 dt.constructDecisionTree("./input/HT_Sensor/tree.dot");
//	 dt.readCostMessage("./input/HT_Sensor/attribute_cost.txt");
//	 dt.averageDepthOfLeaves();
//	 dt.traditionalDecision("./input/HT_Sensor/test_data_200");
//	 dt.incrementalDecision_no_prior("./input/HT_Sensor/test_data_200");
//	 dt.incrementalDecision_with_known_attribute("./input/HT_Sensor/test_data_200", {0,1,2,3}, true, 4);
//
//    // 简单验证 wilt 数据集，渐进计算是否有效
//
//	 DecisionTree dt;
//	 dt.constructDecisionTree("./input/wilt/tree.dot");
//	 dt.readCostMessage("./input/wilt/attribute_cost.txt");
//	 dt.averageDepthOfLeaves();
//	 dt.traditionalDecision("./input/wilt/test_data");
//	 dt.incrementalDecision_no_prior("./input/wilt/test_data");
//	 dt.incrementalDecision_with_known_attribute("./input/wilt/test_data", {}, true, 2);
//
//    // 简单验证 Healthy 数据集，渐进计算是否有效
//
//	DecisionTree dt;
//	dt.constructDecisionTree("./input/Healthy/network_attribute_5-8/decision_tree.dot");
//	dt.readCostMessage("./input/Healthy/attribute_cost.txt");
//	dt.averageDepthOfLeaves();
//	dt.traditionalDecision("./input/Healthy/network_attribute_5-8/test_data.txt");
//	dt.incrementalDecision_no_prior("./input/Healthy/network_attribute_5-8/test_data.txt");
//	dt.incrementalDecision_with_known_attribute("./input/Healthy/network_attribute_5-8/test_data.txt", {}, true, 2);
//
//    // 在异构网络中验证 HT_Sensor 数据集，渐进计算是否有效
//
////	map<int, int> cost_trans = { {0,4}, {1,4}, {2,4}, {3,4}, {4,4}, {5,4}, {6,4}, {7,4}, {8,4}, {9,4} };
////	SimpleNetwork network = SimpleNetwork("./input/HT_Sensor/network1/");
////	DecisionTree dt;
////	dt.constructDecisionTree("./input/HT_Sensor/tree.dot");
////	dt.averageDepthOfLeaves();
////	dt.setCostTrans(cost_trans);
//	dt.traditionalDecision_Battery("./input/HT_Sensor/test_data_200", network);
//	dt.incrementalDecision_no_prior_self_control_Battery("./input/HT_Sensor/test_data_200", network);
//	dt.incrementalDecision_known_attribute_self_control_Battery("./input/HT_Sensor/test_data_200", network, { 0,1,2,3 });
//	dt.incrementalDecision_no_prior_RF_control_Battery("./input/HT_Sensor/test_data_200", network);
//	dt.incrementalDecision_known_attribute_RF_control_Battery("./input/HT_Sensor/test_data_200", network, { 0,1,2,3 });
//	cout << dt.RF_charging_tradition_1("./input/HT_Sensor/test_data_200", network, K) << endl;
//	cout << dt.RF_charging_incremental("./input/HT_Sensor/test_data_200", network, K) << endl;
//	cout << dt.RF_charging_incremental_with_known_attribute("./input/HT_Sensor/test_data_200", network, { 0,1,2,3 }, K) << endl;
//	cout << dt.RF_charging_known_attribute_new("./input/HT_Sensor/test_data_200", network, K) << endl;
//	cout << dt.energy_harvest_tradition("./input/HT_Sensor/test_data_200", network, 1, 50, K, 2) << endl;
//	cout << dt.energy_harvest_incremental("./input/HT_Sensor/test_data_200", network, 1, 50, K, 2) << endl;
//	cout << dt.energy_harvest_incremental_with_known_attribute("./input/HT_Sensor/test_data_200", network, { 0,1,2,3 }, 1, 50, K, 2) << endl;
//	cout << dt.energy_harvest_known_attribute_new("./input/HT_Sensor/test_data_200", network, 1, 50, K, 2) << endl;

    // 在异构网络中验证 Healthy 数据集，渐进计算是否有效

//	string attribute = "2-4";
//	SimpleNetwork network = SimpleNetwork("./input/Healthy/network_attribute_" + attribute + "/");
//	DecisionTree dt;
//	dt.constructDecisionTree("./input/Healthy/network_attribute_" + attribute + "/decision_tree.dot");
//	dt.averageDepthOfLeaves();
//	dt.traditionalDecision_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network);
//	dt.incrementalDecision_no_prior_self_control_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network);
//	dt.incrementalDecision_known_attribute_self_control_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { 0,1 });
//	dt.incrementalDecision_no_prior_RF_control_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network);
//	dt.incrementalDecision_known_attribute_RF_control_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { 0,1 });
//	dt.RF_charging_tradition_1("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, true);
//	dt.RF_charging_incremental("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, true);
//	dt.RF_charging_incremental_with_known_attribute("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { 0,1 }, K);
//	dt.energy_harvest_tradition("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, 10, 50, K, 2);
//	dt.energy_harvest_incremental("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, 10, 50, K, 2);
//	dt.energy_harvest_incremental_with_known_attribute("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { 0,1 }, 10, 50, K, 2);

    // 在异构网络中验证 wilt 数据集，渐进计算是否有效

    //SimpleNetwork network = SimpleNetwork("./input/wilt/network/");
    //DecisionTree dt;
    //dt.constructDecisionTree("./input/wilt/tree.dot");
    //dt.averageDepthOfLeaves();
    //dt.traditionalDecision_Battery("./input/wilt/test_data", network);
    //dt.incrementalDecision_no_prior_self_control_Battery("./input/wilt/test_data", network);
    //dt.incrementalDecision_known_attribute_self_control_Battery("./input/wilt/test_data", network, { 0 });
    //dt.incrementalDecision_no_prior_RF_control_Battery("./input/wilt/test_data", network);
    //dt.incrementalDecision_known_attribute_RF_control_Battery("./input/wilt/test_data", network, { 0 });
    //dt.RF_charging_tradition_1("./input/wilt/test_data", network, K);
    //dt.RF_charging_incremental("./input/wilt/test_data", network, K);
    //dt.RF_charging_incremental_with_known_attribute("./input/wilt/test_data", network, { 0 }, K);
    //dt.energy_harvest_tradition("./input/wilt/test_data", network, 10, 50, K, 2);
    //dt.energy_harvest_incremental("./input/wilt/test_data", network, 10, 50, K, 2);
    //dt.energy_harvest_incremental_with_known_attribute("./input/wilt/test_data", network, { 0 }, 10, 50, K, 2);


    // 简单验证北京空气质量数据集，多目标查询渐进计算是否有效

//    MultipleQuery mq;
//    mq.readAttributeCosts("./input/AirQuality/");
//    mq.air_quality_readRecord("./input/AirQuality/CrawledData.txt");
//    mq.air_quality_traditionalQuery("./input/AirQuality/statistics.txt");
//    mq.air_quality_sequentialProgressiveQuery("./input/AirQuality/statistics.txt", { 0, 1, 2, 3, 5 }, true);
//    mq.air_quality_guidedProgressiveQuery("./input/AirQuality/statistics.txt", { 0, 1, 2, 3, 5 }, true);
//    mq.air_quality_extraProgressiveQuery("./input/AirQuality/statistics.txt", { 0, 1, 2, 3, 5 }, true);
//    mq.air_quality_sequentialProgressiveQuery("./input/AirQuality/statistics.txt", { 0, 1, 2, 3, 5 }, false);
//    mq.air_quality_guidedProgressiveQuery("./input/AirQuality/statistics.txt", { 0, 1, 2, 3, 5 }, false);
//    mq.air_quality_extraProgressiveQuery("./input/AirQuality/statistics.txt", { 0, 1, 2, 3, 5 }, false);


    // 实验二

    //map<int, int> cost_24 = { {0,16}, {1,16}, {2,16} };
    //map<int, int> cost_58 = { {0,256}, {1,256}, {2,256}, {3,256} };
    //map<int, int> cost_28 = { {0,16}, {1,16}, {2,16}, {3,256}, {4,256}, {5,256}, {6,256} };
    ////
    //string attribute = "2-8";
    //SimpleNetwork network = SimpleNetwork("./input/Healthy/network_attribute_" + attribute + "/");
    ////
    //DecisionTree dt;
    //dt.constructDecisionTree("./input/Healthy/network_attribute_" + attribute + "/decision_tree.dot");
    //dt.setCostTrans(cost_28);
    //cout << dt.traditionalDecision_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
    //cout << dt.incrementalDecision_no_prior_RF_control_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;

    // 实验三

    //map<int, int> cost_28 = { {0,16}, {1,16}, {2,16}, {3,256}, {4,256}, {5,256}, {6,256} };
    ////
    //string attribute = "2-8";
    //SimpleNetwork network = SimpleNetwork("./input/Healthy/network_attribute_" + attribute + "/");
    ////
    //DecisionTree dt;
    //dt.constructDecisionTree("./input/Healthy/network_attribute_" + attribute + "/decision_tree.dot");
    //dt.setCostTrans(cost_28);
    //cout << dt.traditionalDecision_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
    //cout << dt.battery_power_known_attribute_new_RF_control("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, {  }) << endl;
    //cout << dt.battery_power_known_attribute_new_RF_control("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { {0,1,2} }) << endl;
    //cout << dt.battery_power_known_attribute_new_RF_control("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { {3,4,5,6} }) << endl;
    //cout << dt.battery_power_known_attribute_new_RF_control("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { { 0,1,2 }, {3,4,5,6} }) << endl;

    // 实验五

    // SimpleNetwork network = SimpleNetwork("./input/HT_Sensor/network1/");
    // network.printSimpleNetwork();
    // DecisionTree dt;
    // //
    // ofstream outfile;
    // outfile.open("./output/test.txt");
    // outfile.setf(ios::fixed, ios::floatfield);
    // outfile.precision(6);
    // outfile << "属性的保质期 10s	RF-tradition	RF-渐进无已知属性	RF-渐进有已知属性	室内光-tradition	室内光-渐进无已知属性	室内光-渐进有已知属性	室外光-tradition	室外光-渐进无已知属性	室外光-渐进有已知属性" << endl;
    // dt.constructDecisionTree("./input/HT_Sensor/tree.dot");
    // int temp = 13107; // 0.1M
    // for (int i = 1; i <= 50; i++) {
    // 	int k = i * temp;
    // 	vector<double> tradition_1, incremental_1, known_1;
    // 	vector<double> tradition_2, incremental_2, known_2;
    // 	for (int seed = 5; seed <= 5; seed++) { // 2 3
    // 		tradition_1.push_back(dt.energy_harvest_tradition("./input/HT_Sensor/test_data_200", network, 1, 50, k, seed));
    // 		incremental_1.push_back(dt.energy_harvest_incremental("./input/HT_Sensor/test_data_200", network, 1, 50, k, seed));
    // 		known_1.push_back(dt.energy_harvest_known_attribute_new("./input/HT_Sensor/test_data_200", network, 1, 50, k, seed));
    // 		tradition_2.push_back(dt.energy_harvest_tradition("./input/HT_Sensor/test_data_200", network, 100, 500, k, seed));
    // 		incremental_2.push_back(dt.energy_harvest_incremental("./input/HT_Sensor/test_data_200", network, 100, 500, k, seed));
    // 		known_2.push_back(dt.energy_harvest_known_attribute_new("./input/HT_Sensor/test_data_200", network, 100, 500, k, seed));
    // 	}
    // 	outfile << to_string(0.1 * i) << "MB\t" << dt.RF_charging_tradition_1("./input/HT_Sensor/test_data_200", network, k) << "\t" <<
    // 		dt.RF_charging_incremental("./input/HT_Sensor/test_data_200", network, k) << "\t" <<
    // 		dt.RF_charging_known_attribute_new("./input/HT_Sensor/test_data_200", network, k) << "\t" <<
    // 		vector_average(tradition_1) << "\t" << vector_average(incremental_1) << "\t" << vector_average(known_1) << "\t" <<
    // 		vector_average(tradition_2) << "\t" << vector_average(incremental_2) << "\t" << vector_average(known_2) << endl;
    // }

    // 实验六

    // MultipleQuery mq;
    // mq.readAttributeCosts("./input/AirQuality/");
    // mq.air_quality_readRecord("./input/AirQuality/CrawledData.txt");
    // cout << "RF tradition: " << mq.RF_tradition("./input/AirQuality/statistics.txt") << "s" << endl;
    // cout << "RF sequential: " << mq.RF_sequential("./input/AirQuality/statistics.txt") << "s" << endl;
    // cout << "RF guided: " << mq.RF_guided("./input/AirQuality/statistics.txt") << "s" << endl;
    // cout << "RF extra: " << mq.RF_extra("./input/AirQuality/statistics.txt") << "s" << endl << endl;
    // // {10,26,37,49,53,63,75,76,78,81,82,87,131,143,147,154,160,183,185,189,194,198}
    // int seed = 147;
    // cout << "室内光 tradition: " << mq.energy_harvest_tradition("./input/AirQuality/statistics.txt", 1, 50, seed) << "s" << endl;
    // cout << "室内光 sequential: " << mq.energy_harvest_sequential("./input/AirQuality/statistics.txt", 1, 50, seed) << "s" << endl;
    // cout << "室内光 guided: " << mq.energy_harvest_guided("./input/AirQuality/statistics.txt", 1, 50, seed) << "s" << endl;
    // cout << "室内光 extra: " << mq.energy_harvest_extra("./input/AirQuality/statistics.txt", 1, 50, seed) << "s" << endl << endl;
    // cout << "室外光 tradition: " << mq.energy_harvest_tradition("./input/AirQuality/statistics.txt", 100, 500, seed) << "s" << endl;
    // cout << "室外光 sequential: " << mq.energy_harvest_sequential("./input/AirQuality/statistics.txt", 100, 500, seed) << "s" << endl;
    // cout << "室外光 guided: " << mq.energy_harvest_guided("./input/AirQuality/statistics.txt", 100, 500, seed) << "s" << endl;
    // cout << "室外光 extra: " << mq.energy_harvest_extra("./input/AirQuality/statistics.txt", 100, 500, seed) << "s" << endl;

    // 8.11新实验 6.1

    // MultipleQuery mq;
    // mq.readAttributeCosts("./input/AirQuality/");
    // mq.air_quality_readRecord("./input/AirQuality/CrawledData.txt");
    // for (int k = 2; k <= 7; k++) {
    // 	cout << "attribute number: " << k << endl;
    // 	cout << "RF tradition: " << mq.RF_tradition("./input/AirQuality/statistics.txt", k) << "s" << endl;
    // 	cout << "RF sequential: " << mq.RF_sequential("./input/AirQuality/statistics.txt", k) << "s" << endl;
    // 	cout << "RF sel: " << mq.RF_sel("./input/AirQuality/statistics.txt", k) << "s" << endl;
    // 	// {10,26,37,49,53,63,75,76,78,81,82,87,131,143,147,154,160,183,185,189,194,198}
    // 	int seed = 147;
    // 	cout << "室内光 tradition: " << mq.energy_harvest_tradition("./input/AirQuality/statistics.txt", 1, 50, seed, k) << "s" << endl;
    // 	cout << "室内光 sequential: " << mq.energy_harvest_sequential("./input/AirQuality/statistics.txt", 1, 50, seed, k) << "s" << endl;
    // 	cout << "室内光 sel: " << mq.energy_harvest_sel("./input/AirQuality/statistics.txt", 1, 50, seed, k) << "s" << endl;
    // 	cout << "室外光 tradition: " << mq.energy_harvest_tradition("./input/AirQuality/statistics.txt", 100, 500, seed, k) << "s" << endl;
    // 	cout << "室外光 sequential: " << mq.energy_harvest_sequential("./input/AirQuality/statistics.txt", 100, 500, seed, k) << "s" << endl;
    // 	cout << "室外光 sel: " << mq.energy_harvest_sel("./input/AirQuality/statistics.txt", 100, 500, seed, k) << "s" << endl << endl;
    // }

    // 8.11 新实验 6.2

     MultipleQuery mq;
     mq.readAttributeCosts("./input/AirQuality/");
     mq.air_quality_readRecord("./input/AirQuality/CrawledData.txt");
     int seed = 147;
     for (int k = 1; k <= 8; k++) {
     	cout << "k = " << k << endl;
     	mq.RF("./input/AirQuality/statistics.txt", 4, k);
     	mq.energy_harvest("./input/AirQuality/statistics.txt", 1, 50, seed, 4, k);
     	mq.energy_harvest("./input/AirQuality/statistics.txt", 100, 500, seed, 4, k);
     }

    // 8.12 新实验 1，注意这里改成了每个record的能量消耗，如果想重现之前实验一二三的结果的话，代码需要改

    // 葡萄酒数据集

    // map<int, int> cost_trans = { {0,16}, {1,16}, {2,16}, {3,16}, {4,16}, {5,16}, {6,16}, {7,16}, {8,16}, {9,16} };
    // SimpleNetwork network = SimpleNetwork("./input/HT_Sensor/network1/");
    // DecisionTree dt;
    // dt.constructDecisionTree("./input/HT_Sensor/tree.dot");
    // dt.averageDepthOfLeaves();
    // dt.setCostTrans(cost_trans);
    // cout << dt.traditionalDecision_Battery("./input/HT_Sensor/test_data_200", network) << endl;
    // cout << dt.incrementalDecision_no_prior_RF_control_Battery("./input/HT_Sensor/test_data_200", network) << endl;
    // cout << dt.battery_power_known_attribute_new_RF_control("./input/HT_Sensor/test_data_200", network, {}) << endl;

    // 动作识别数据集

    // map<int, int> cost_28 = { {0,16}, {1,16}, {2,16}, {3,16}, {4,16}, {5,16}, {6,16} };
    // //
    // string attribute = "2-8";
    // SimpleNetwork network = SimpleNetwork("./input/Healthy/network_attribute_" + attribute + "/");
    // //
    // DecisionTree dt;
    // dt.constructDecisionTree("./input/Healthy/network_attribute_" + attribute + "/decision_tree.dot");
    // dt.averageDepthOfLeaves();
    // dt.setCostTrans(cost_28);
    // cout << dt.traditionalDecision_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
    // cout << dt.incrementalDecision_no_prior_RF_control_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
    // cout << dt.battery_power_known_attribute_new_RF_control("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, {}) << endl;

    // 树木生病数据集

    // map<int, int> cost_trans = { {0,16}, {1,16}, {2,16}, {3,16}, {4,16}};
    // SimpleNetwork network = SimpleNetwork("./input/wilt/network/");
    // DecisionTree dt;
    // dt.constructDecisionTree("./input/wilt/tree.dot");
    // dt.averageDepthOfLeaves();
    // dt.setCostTrans(cost_trans);
    // cout << dt.traditionalDecision_Battery("./input/wilt/test_data", network) <<endl;
    // cout << dt.incrementalDecision_no_prior_RF_control_Battery("./input/wilt/test_data", network) <<endl;
    // cout << dt.battery_power_known_attribute_new_RF_control("./input/wilt/test_data", network, {}) << endl;

    // 8.13 新实验 2

    // for (int k = 1; k <= 8; k++) {
    // 	map<int, int> cost_28;
    // 	if (k == 1) {
    // 		cost_28 = { {0,16}, {1,16}, {2,16}, {3,16}, {4,16}, {5,16}, {6,16} };
    // 	}
    // 	else {
    // 		cost_28[0] = 16;
    // 		for (int i = 1; i <= 6; i++) {
    // 			cost_28[i] = 16 * i * k;
    // 		}
    // 	}

    // 	string attribute = "2-8";
    // 	SimpleNetwork network = SimpleNetwork("./input/Healthy/network_attribute_" + attribute + "/");
    // 	//
    // 	DecisionTree dt;
    // 	dt.constructDecisionTree("./input/Healthy/network_attribute_" + attribute + "/decision_tree.dot");
    // 	dt.setCostTrans(cost_28);
    // 	cout << "k = " << k << endl;
    // 	cout << dt.traditionalDecision_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
    // 	cout << dt.incrementalDecision_no_prior_RF_control_Battery("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
    // 	cout << dt.battery_power_known_attribute_new_RF_control("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, {}) << endl;
    // 	cout << dt.battery_power_known_attribute_new_RF_control("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, { {0,1,2} }) << endl;
    // }

    // 8.13 新实验 3

//    for (int k = 1; k <= 8; k++) {
//        map<int, int> cost_28;
//        if (k == 1) {
//            cost_28 = { {0,16}, {1,16}, {2,16}, {3,16}, {4,16}, {5,16}, {6,16} };
//        }
//        else {
//            cost_28[0] = 16;
//            for (int i = 1; i <= 6; i++) {
//                cost_28[i] = 16 * i * k;
//            }
//        }
//
//        string attribute = "2-8";
//        SimpleNetwork network = SimpleNetwork("./input/Healthy/network_attribute_" + attribute + "/");
//        //
//        DecisionTree dt;
//        dt.constructDecisionTree("./input/Healthy/network_attribute_" + attribute + "/decision_tree.dot");
//        dt.setCostTrans(cost_28);
//        cout << "k = " << k << endl;
//        cout << dt.new_3_RF_charging_tradition("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
//        cout << dt.new_3_RF_charging_incremental("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network) << endl;
//        cout << dt.new_3_energy_harvest_tradition("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, 1, 50, 3) << endl;
//        cout << dt.new_3_energy_harvest_incremental("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, 1, 50, 3) << endl;
//        cout << dt.new_3_energy_harvest_tradition("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, 100, 500, 3) << endl;
//        cout << dt.new_3_energy_harvest_incremental("./input/Healthy/network_attribute_" + attribute + "/test_data.txt", network, 100, 500, 3) << endl;
//    }
}

void print_help_informamtion(){
    // todo: output cmd usage
    cout << "hello world" << endl;
}

MultipleQuery dataset_deal_query(const string& dataset){
    MultipleQuery mq;
    if(!strcasecmp(dataset.c_str(), "HT_sensor")){
        HT_Sensor_sample_average("./input/HT_Sensor/HT_Sensor_dataset", 10, 2000);
        // todo: realization not found
    }
    else if(!strcasecmp(dataset.c_str(), "wilt")){
        wilt_transfer_csv_to_txt("./input/wilt/training.csv");
        // todo: realization not found
    }
    else if(!strcasecmp(dataset.c_str(), "AirQuality")){
        Air_preprocess("./input/AirQuality/CrawledData.txt");
        mq.readAttributeCosts("./input/AirQuality/");
        mq.air_quality_readRecord("./input/AirQuality/CrawledData.txt");
    }
    else if(!strcasecmp(dataset.c_str(), "healthy")){
        // todo: realization not found
    }
    return mq;
}

DecisionTree dataset_deal_recognition(const string& dataset, string attribute="2-8"){
    DecisionTree dt;
    if(!strcasecmp(dataset.c_str(), "HT_sensor")){
        HT_Sensor_sample_average("./input/HT_Sensor/HT_Sensor_dataset", 10, 2000);
        dt.constructDecisionTree("./input/HT_Sensor/tree.dot");
        dt.readCostMessage("./input/HT_Sensor/attribute_cost.txt");
        dt.setCostTrans(get_cost_trans("./input/HT_sensor/cost_trans.txt"));
        dt.averageDepthOfLeaves();
    }
    else if(!strcasecmp(dataset.c_str(), "wilt")){
        wilt_transfer_csv_to_txt("./input/wilt/training.csv");
        dt.constructDecisionTree("./input/wilt/tree.dot");
        dt.readCostMessage("./input/wilt/attribute_cost.txt");
        dt.setCostTrans(get_cost_trans("./input/wilt/cost_trans.txt"));
        dt.averageDepthOfLeaves();
    }
    else if(!strcasecmp(dataset.c_str(), "AirQuality")){
        Air_preprocess("./input/AirQuality/CrawledData.txt");
        // todo: no tree.dot
    }
    else if(!strcasecmp(dataset.c_str(), "healthy")){
        dt.constructDecisionTree("./input/Healthy/network_attribute_" + attribute + "/decision_tree.dot");
        dt.readCostMessage("./input/Healthy/attribute_cost.txt");
        dt.setCostTrans(get_cost_trans("./input/Healthy/cost_trans.txt"));
        dt.averageDepthOfLeaves();
    }
    return dt;
}

SimpleNetwork get_network(const string& dataset, string attribute){
    if(!strcasecmp(dataset.c_str(), "HT_sensor")){
        SimpleNetwork network = SimpleNetwork("./input/HT_Sensor/network1/");
        return network;
    }
    else if(!strcasecmp(dataset.c_str(), "wilt")){
        SimpleNetwork network = SimpleNetwork("./input/wilt/network/");
        return network;
    }
    else if(!strcasecmp(dataset.c_str(), "healthy")){
        SimpleNetwork network = SimpleNetwork("./input/Healthy/network_attribute_" + attribute + "/");
        return network;
    }
//    else if(!strcasecmp(dataset.c_str(), "AirQuality")){
//        // todo: realization not found
//    }
}

map<int, int> get_cost_trans(string file_path){
    map<int, int> cost_trans;
    ifstream ifs(file_path, ios::in);
    int a, b;
    while(ifs >> a){
        ifs >> b;
        cost_trans.insert(make_pair(a, b));
    }
    ifs.close();
    return cost_trans;
}

vector<string> split(string s, char a) {
    vector<string> res;
    string tmp;
    for(char i: s){
        if(i == a){
            if(!tmp.empty()) { res.emplace_back(tmp); tmp.clear();}
            continue;
        }
        tmp.push_back(i);
    }
    if(!tmp.empty()) res.emplace_back(tmp);
    return res;
}

void function_execute_recognition_file(DecisionTree dt, string dataset, string file_path, int function_choice, SimpleNetwork network){
    switch(function_choice){
        case 1: dt.traditionalDecision(file_path); break;
        case 2: dt.incrementalDecision_no_prior(file_path); break;
        case 3: dt.incrementalDecision_with_known_attribute(file_path, {0,1,2,3}, true, 4); break;
        case 4: dt.traditionalDecision_Battery(file_path, network); break;
        case 5: dt.incrementalDecision_no_prior_self_control_Battery(file_path, network); break;
        case 6: dt.incrementalDecision_random_known_attribute_self_control_Battery(file_path, network, { 0,1,2,3 }); break;
        case 7: dt.incrementalDecision_no_prior_RF_control_Battery(file_path, network); break;
        case 8: dt.incrementalDecision_random_known_attribute_RF_control_Battery(file_path, network, { 0,1,2,3 }); break;
        case 9: cout << dt.RF_charging_tradition_1(file_path, network, K) << endl; break;
        case 10: cout << dt.RF_charging_incremental(file_path, network, K) << endl; break;
        case 11: cout << dt.RF_charging_incremental_with_known_attribute(file_path, network, { 0,1,2,3 }, K) << endl; break;
        case 12: cout << dt.RF_charging_known_attribute_new(file_path, network, K) << endl; break;
        case 13: cout << dt.energy_harvest_tradition(file_path, network, 1, 50, K, 2) << endl; break;
        case 14: cout << dt.energy_harvest_incremental(file_path, network, 1, 50, K, 2) << endl; break;
        case 15: cout << dt.energy_harvest_incremental_with_known_attribute(file_path, network, { 0,1,2,3 }, 1, 50, K, 2) << endl; break;
        case 16: cout << dt.energy_harvest_known_attribute_new(file_path, network, 1, 50, K, 2) << endl; break;
    }
}

void function_execute_recognition_cmd(DecisionTree dt, string dataset, string query_data, int function_choice, SimpleNetwork network){
    ofstream ofs("./tmp.txt", ios::out);
    ofs << replace(query_data, '_', '\t');
    ofs.close();
    function_execute_recognition_file(dt, dataset, "./tmp.txt", function_choice, network);
}

string replace(string s, char from, char to){
    string res = s;
    for(int i = 0; i < res.size(); ++i){
        if(res[i] == from) res[i] = to;
    }
    return res;
}

void function_execute_query_file(MultipleQuery mq, string dataset, string file_path, int function_choice){
    int seed = 147;  // useless in "file" mode
    int k = 3;  // todo: input by user?
    string data_file = "./input/" + dataset + "/statistics.txt";

    int split_line_length = 100;
    for(int i = 0; i < split_line_length; ++i) cout << '-'; cout << endl;
    cout << "Dataset: " << dataset << endl;
    cout << "Query file: " << file_path << endl;
    cout << "Detail below:" << endl;

    switch(function_choice){
//        case 1: mq.air_quality_sequentialProgressiveQuery(data_file, { 0, 1, 2, 3, 5 }, true); break;
//        case 2: mq.air_quality_guidedProgressiveQuery(data_file, { 0, 1, 2, 3, 5 }, true); break;
//        case 3: mq.air_quality_extraProgressiveQuery(data_file, { 0, 1, 2, 3, 5 }, true); break;
//        case 4: mq.air_quality_sequentialProgressiveQuery(data_file, { 0, 1, 2, 3, 5 }, false); break;
//        case 5: mq.air_quality_guidedProgressiveQuery(data_file, { 0, 1, 2, 3, 5 }, false); break;
//        case 6: mq.air_quality_extraProgressiveQuery(data_file, { 0, 1, 2, 3, 5 }, false); break;
        case 7: cout << "RF tradition: " << mq.RF_tradition_file(data_file, file_path) << "s" << endl; break;
        case 8: cout << "RF sequential: " << mq.RF_sequential_file(data_file, file_path) << "s" << endl; break;
        case 9: cout << "RF sel: " << mq.RF_sel_file(data_file, file_path) << "s" << endl; break;
//        case 10: cout << "RF extra: " << mq.RF_extra(data_file) << "s" << endl; break;
        case 11: cout << "室内光 tradition: " << mq.energy_harvest_tradition_file(data_file, 1, 50, seed, file_path) << "s" << endl; break;
        case 12: cout << "室内光 sequential: " << mq.energy_harvest_sequential_file(data_file, 1, 50, seed, file_path) << "s" << endl; break;
        case 13: cout << "室内光 sel: " << mq.energy_harvest_sel_file(data_file, 1, 50, seed, file_path) << "s" << endl; break;
//        case 14: cout << "室内光 extra: " << mq.energy_harvest_extra(data_file, 1, 50, seed) << "s" << endl << endl; break;
        case 15: mq.RF_file(data_file, file_path, k); break;
        case 16: mq.energy_harvest_file(data_file, 1, 50, seed, file_path, k); break;
    }

    for(int i = 0; i < split_line_length; ++i) cout << '-'; cout << "\n" << endl;
}

void function_execute_query_cmd(MultipleQuery mq, string dataset, string query_data, string query_limits, int function_choice){
    ofstream ofs("./tmp.txt", ios::out);
    ofs << replace(query_data, '_', '\t') << "\n";
    ofs << replace(query_limits, '_', '\t');
    ofs.close();
    function_execute_query_file(mq, dataset, "./tmp.txt", function_choice);
}
