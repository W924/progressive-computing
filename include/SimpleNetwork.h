#pragma once
#include <vector>
#include <map>
#include <string>
#include "Sensor.h"
#include "Object.h"
#include "Transmitter.h"


// SimpleNetwork，Simple指的是这里仅考虑一个sensor只能监控一个属性，但考虑了可能有多个sensor同时工作来获取同一属性的情况
class SimpleNetwork
{
public:
	std::map<int, double> attributes_cost_proc; // object每个属性的计算代价

	std::map<std::string, Sensor> sensors;
	std::map<std::string, Object> objects;
	std::map<std::string, Transmitter> transmitters; // 该网络中的节点，包括sensor,object以及transmitter

	std::map<std::string, int> sensor_attribute; // 记录每个sensor负责监控哪个属性，这里仅考虑了一个sensor只能监控计算一个属性的数据
	std::map<std::string, std::map<int, std::vector<std::string>>> object_attribute_sensor; // 记录每个object每个属性的sensor集合（可能有多个sensor共同工作来获取同一个属性）
	std::map<std::string, std::map<std::string, double>> object_sensor_distance; // object到每个监测它的sensor的距离，一对多的关系
	std::map<std::string, std::pair<std::string, double>> sensor_transmitter_distance; // 记录距离每个 sensor 最近的 transmitter及距离，一对一的关系
	
	SimpleNetwork(std::string inFileFolder);
	void printSimpleNetwork();

	// (battery-power情形)，看唤醒需要多长时间，这里没有考虑范围充电
	double cost_control_time(std::vector<std::string> sensor_id);
	
	// 纯RF-charging情形下，传输所需要的时间，计算所需要的代价（范围充电，轮数不超过充电桩的个数）
	std::pair<double, double> rf_charging_cost(std::vector<std::pair<std::string, double>> sensor_id_energyRequired);

	// energy-harvest情形，传输所诉要的时间，计算所需要的代价（如果需要充电的话，所有的sensor都要充电）
	std::pair<double, double> energy_harvest_cost(std::vector<std::pair<std::string, double>> sensor_id_energyRequired, std::map<std::string, double> power_received, double power_min, double power_max);
	void energy_harvest_receive_energy(double charging_time, std::map<std::string, double> power_received, double power_min, double power_max);

	// 根据保质期产生已知属性-record
	std::vector<int> generate_known_attributes(std::string object_name);

	// 更新时间保质期-time
	void update_life_time(std::vector<std::string> sensors_id, double cost_time);
};
