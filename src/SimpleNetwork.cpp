#include "SimpleNetwork.h"
#include "ConstDefine.h"
#include "Util.h"
#include <iostream>
#include <fstream>
#include <set>

using namespace std;


SimpleNetwork::SimpleNetwork(string inFileFolder) {
	ifstream infile;
	string line;
	
	// 获取每个属性的计算代价
	infile.open(inFileFolder + "attribute_cost_proc");
	if (!infile.is_open()) {
		cout << "Open attribute_cost_proc fail!" << endl;
	}
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ",");
		attributes_cost_proc[atoi(temp_vector[0].c_str())] = atof(temp_vector[1].c_str());

	}
	infile.close();

	// 获取sensor和transmitter的信息，并记录距离每个sensor最近的transmitter及距离
	infile.open(inFileFolder + "sensor_transmitter");
	if (!infile.is_open()) {
		cout << "Open sensor_transmitter fail!" << endl;
	}
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ":");
		vector<string> sensor_vector = split(temp_vector[0], ",");
		vector<string> transmitter_vector = split(temp_vector[1], ",");

		string sensor_name = sensor_vector[0];
		double sensor_x = atof(sensor_vector[1].c_str());
		double sensor_y = atof(sensor_vector[2].c_str());
		if (sensors.count(sensor_name) == 0) {
			Sensor new_sensor = Sensor(sensor_x, sensor_y);
			sensors[sensor_name] = new_sensor; // 这里相当于是拷贝！也就是创建一个新的相同的new_sensor对象给map
		}

		string transmitter_name = transmitter_vector[0];
		double transmitter_x = atof(transmitter_vector[1].c_str());
		double transmitter_y = atof(transmitter_vector[2].c_str());
		if (transmitters.count(transmitter_name) == 0) {
			Transmitter new_transmitter = Transmitter(transmitter_x, transmitter_y);
			transmitters[transmitter_name] = new_transmitter;
		}
		
		double distacne = atof(temp_vector[2].c_str());
		if (sensor_transmitter_distance.count(sensor_name) == 0) {
			//sensor_transmitter_distance[sensor_name] = make_pair(transmitter_name, norm({ sensor_x - transmitter_x, sensor_y - transmitter_y }));
			sensor_transmitter_distance[sensor_name] = make_pair(transmitter_name, distacne);
		}
	}
	infile.close();

	// 获取objects的信息，并记录监测每个object的所有sensor
	infile.open(inFileFolder + "sensor_object");
	if (!infile.is_open()) {
		cout << "Open sensor_object fail" << endl;
	}
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ":");
		vector<string> sensor_vector = split(temp_vector[0], ",");
		vector<string> object_vector = split(temp_vector[1], ",");

		string sensor_name = sensor_vector[0];
		double sensor_x = atof(sensor_vector[1].c_str());
		double sensor_y = atof(sensor_vector[2].c_str());

		string object_name = object_vector[0];
		double object_x = atof(object_vector[1].c_str());
		double object_y = atof(object_vector[2].c_str());
		if (objects.count(object_name) == 0) {
			Object new_object = Object(object_x, object_y);
			objects[object_name] = new_object;
		}

		if (object_sensor_distance.count(object_name) == 0) {
			map<string, double> new_map;
			new_map[sensor_name] = norm({sensor_x - object_x, sensor_y - object_y});
			object_sensor_distance[object_name] = new_map;
		}
		else {
			object_sensor_distance[object_name][sensor_name] = norm({ sensor_x - object_x, sensor_y - object_y });
		}
	}
	infile.close();

	// 获取监测每个object的每个属性的sensor集合，
	infile.open(inFileFolder + "sensor_attribute");
	while (getline(infile, line)) {
		vector<string> temp_vector = split(line, ":");
		sensor_attribute[temp_vector[0]] = atoi(temp_vector[1].c_str());
	}
	map<string, map<string, double>>::iterator iter1;
	map<string, double>::iterator iter2;
	for (iter1 = object_sensor_distance.begin(); iter1 != object_sensor_distance.end(); iter1++) {
		string object_name = iter1->first;
		map<string, double> sensor_distance = iter1->second;
		
		map<int, vector<string>> attribute_sensors;
		for (iter2 = sensor_distance.begin(); iter2 != sensor_distance.end(); iter2++) {
			string sensor_name = iter2->first;
			int attribute = sensor_attribute[sensor_name];

			if (attribute_sensors.count(attribute) == 0) {
				attribute_sensors[attribute] = {sensor_name};
			}
			else {
				attribute_sensors[attribute].push_back(sensor_name);
			}
		}
		object_attribute_sensor[object_name] = attribute_sensors;
	}
	infile.close();
}


void SimpleNetwork::printSimpleNetwork() {
	map<string, Sensor>::iterator iter1;
	map<string, Transmitter>::iterator iter2;
	map<string, pair<string, double>>::iterator iter3;
	map<string, Object>::iterator iter4;
	map<string, map<string, double>>::iterator iter5;
	map<string, double>::iterator iter6;
	map<string, map<int, vector<string>>>::iterator iter7;
	map<int, vector<string>>::iterator iter8;

	cout << "sensors: " << endl;
	for (iter1 = sensors.begin(); iter1 != sensors.end(); iter1++) {
		cout << iter1->first << ": (" << iter1->second.location_x << "," << iter1->second.location_y << ") energy: " << iter1->second.now_energy << endl;
	}

	cout << "transmitters: " << endl;
	for (iter2 = transmitters.begin(); iter2 != transmitters.end(); iter2++) {
		cout << iter2->first << ": (" << iter2->second.location_x << "," << iter2->second.location_y << ")" << endl;
	}

	cout << "objects: " << endl;
	for (iter4 = objects.begin(); iter4 != objects.end(); iter4++) {
		cout << iter4->first << ": (" << iter4->second.location_x << ", " << iter4->second.location_y << ")" << endl;
	}

	cout << "sensor -> transmitter: " << endl;
	for (iter3 = sensor_transmitter_distance.begin(); iter3 != sensor_transmitter_distance.end(); iter3++) {
		string sensor_name = iter3->first;
		string transmitter_name = iter3->second.first;
		double distance = iter3->second.second;

		cout << sensor_name << ": (" << sensors[sensor_name].location_x << "," << sensors[sensor_name].location_y << ") -> " <<
			transmitter_name << ": (" << transmitters[transmitter_name].location_x << "," << transmitters[transmitter_name].location_y <<
			") distance: " << distance << endl;
	}

	cout << "object -> sensor: " << endl;
	for (iter5 = object_sensor_distance.begin(); iter5 != object_sensor_distance.end(); iter5++) {
		string object_name = iter5->first;
		map<string, double> sensor_distance = iter5->second;
		for (iter6 = sensor_distance.begin(); iter6 != sensor_distance.end(); iter6++) {
			cout << iter6->first << ": (" << sensors[iter6->first].location_x << "," << sensors[iter6->first].location_y << ") -> " <<
				object_name << ": (" << objects[object_name].location_x << "," << objects[object_name].location_y <<
				") distance: " << iter6->second << endl;
		}
	}

	cout << "object attribute sensor: " << endl;
	for (iter7 = object_attribute_sensor.begin(); iter7 != object_attribute_sensor.end(); iter7++) {
		string object_name = iter7->first;
		map<int, vector<string>> attribute_sensors = iter7->second;

		for (iter8 = attribute_sensors.begin(); iter8 != attribute_sensors.end(); iter8++) {
			int attribute = iter8->first;
			vector<string> sensor = iter8->second;

			cout << object_name << ": " << attribute << " [";
			for (int i = 0; i < sensor.size(); i++) {
				cout << sensor[i] << ",";
			}
			cout << "]" << endl;
		}
	}
}


double SimpleNetwork::cost_control_time(vector<string> sensor_id) {
	double max_charging_time = 0;
	for (int i = 0; i < sensor_id.size(); i++) {
		string temp_sensor = sensor_id[i];
		string transimitter_nearest = sensor_transmitter_distance[temp_sensor].first; // 距离该sensor最近的transmitter
		double distance = sensor_transmitter_distance[temp_sensor].second; // sensor和transmitter之间的距离

		double power_received = transmitters[transimitter_nearest].K_const / (distance * distance); // sensor的接收功率，单位是 uW
		double charging_time = sensors[temp_sensor].control_cost_trans / power_received; // 充该sensor需要的时间，单位是 s

		if (charging_time > max_charging_time) {
			max_charging_time = charging_time;
		}
	}
	return max_charging_time;
}


pair<double, double> SimpleNetwork::rf_charging_cost(vector<pair<string, double>> sensor_id_energyRequired) {
	double cost_trans = 0; // 传输代价，这里为时间
	double cost_proc = 0; // 计算代价

	// 首先计算这一整个 sensor 集合所传输数据的计算代价，这里要根据所能检测 attribute 的个数来计算
	set<int> attributes;
	for (int i = 0; i < sensor_id_energyRequired.size(); i++) {
		attributes.insert(sensor_attribute[sensor_id_energyRequired[i].first]);
	}
	for (set<int>::iterator iter = attributes.begin(); iter != attributes.end(); iter++) {
		cost_proc += attributes_cost_proc[*iter];
	}

	// 然后计算这一整个 sensor 集合的传输代价，即 transmitter 充电时间
	while (!sensor_id_energyRequired.empty()) {
		// 找到需要充电时间最长的 sensor, 和对应要工作的transmitter
		string max_charging_time_sensor, charging_transmitter;
		double max_charging_time = 0;

		for (int i = 0; i < sensor_id_energyRequired.size(); i++) {
			string temp_sensor = sensor_id_energyRequired[i].first; // sensor的ID
			double temp_energy = sensor_id_energyRequired[i].second; // sensor传输数据所需的能量

			// sensor所需充！的能量，单位是 uJ
			double energy_required = sensors[temp_sensor].now_energy >= temp_energy ? 0 : temp_energy - sensors[temp_sensor].now_energy;
			
			string transimitter_nearest = sensor_transmitter_distance[temp_sensor].first; // 距离该sensor最近的transmitter
			double distance = sensor_transmitter_distance[temp_sensor].second; // sensor和transmitter之间的距离

			double power_received = transmitters[transimitter_nearest].K_const / (distance * distance); // sensor的接收功率，单位是 uW
			double charging_time = energy_required / power_received; // 充该sensor需要的时间，单位是 s
			if (charging_time > max_charging_time) {
				max_charging_time = charging_time;
				max_charging_time_sensor = temp_sensor;
				charging_transmitter = transimitter_nearest;
			}
		}
		//cout << max_charging_time_sensor << ", " << max_charging_time << endl;
		Transmitter transmitter = transmitters[charging_transmitter]; // 要充电的充电桩
		cost_trans += max_charging_time; // 充电桩消耗的电量，单位是 J

		// 接下来是充电过程，给距离该transmitter 10米内的所有sensor充电，充电时间是确定的，是max_charging_time
		for (map<string, Sensor>::iterator iter = sensors.begin(); iter != sensors.end(); iter++) {
			string temp_sensor = iter->first;
			double distance = norm({ transmitter.location_x - sensors[temp_sensor].location_x, transmitter.location_y - sensors[temp_sensor].location_y });
			if (distance <= 10) {
				double power_received = transmitter.K_const / (distance * distance); // 单位是 uW
				sensors[temp_sensor].now_energy += power_received * max_charging_time; // 充电，单位是 uJ
			}
		}

		// 然后是电量消耗的过程，只有需要传输数据的sensor需要消耗电量，首先需要判断电量是否足够
		for (vector<pair<string, double>>::iterator iter = sensor_id_energyRequired.begin(); iter != sensor_id_energyRequired.end(); ) {
			string temp_sensor = (*iter).first;
			double temp_energy = (*iter).second; // sensor传输数据需要的能量

			if (sensors[temp_sensor].now_energy >= temp_energy) {  // 如果电量足够，消耗电量，并从需要传输数据的sensor集合中删掉该sensor
				sensors[temp_sensor].now_energy -= temp_energy;
				iter = sensor_id_energyRequired.erase(iter);
			}
			else {
				iter++;
			}
		}
	}

	return make_pair(cost_trans, cost_proc);
}


pair<double, double> SimpleNetwork::energy_harvest_cost(vector<pair<string, double>> sensor_id_energyRequired, map<string, double> power_received, double power_min, double power_max) {
	double cost_trans = 0; // 传输代价，这里为时间
	double cost_proc = 0; // 计算代价

	// 首先计算这一整个 sensor 集合所传输数据的计算代价，这里要根据所能检测 attribute 的个数来计算
	set<int> attributes;
	for (int i = 0; i < sensor_id_energyRequired.size(); i++) {
		attributes.insert(sensor_attribute[sensor_id_energyRequired[i].first]);
	}
	for (set<int>::iterator iter = attributes.begin(); iter != attributes.end(); iter++) {
		cost_proc += attributes_cost_proc[*iter];
	}

	// 在需要工作的sensor中，找到需要光照充电时间最长的sensor的充电时间
	double max_charging_time = 0;
	for (int i = 0; i < sensor_id_energyRequired.size(); i++) {
		string temp_sensor = sensor_id_energyRequired[i].first;
		double temp_energy = sensor_id_energyRequired[i].second; // 所需能量
		
		// 所需充电时间，如果电量足够的话，则不用充电
		double charging_time = sensors[temp_sensor].now_energy >= temp_energy ? 0 : (temp_energy - sensors[temp_sensor].now_energy) / power_received[temp_sensor];
		
		if (charging_time > max_charging_time) {
			max_charging_time = charging_time;
		}
	}

	cost_trans = max_charging_time;

	// 给网络中所有的sensor充电
	for (map<string, Sensor>::iterator iter = sensors.begin(); iter != sensors.end(); iter++) {
		string temp_sensor = iter->first;
		sensors[temp_sensor].now_energy += max_charging_time * power_received[temp_sensor];
	}

	// 需要工作的sensor消耗电量
	for (int i = 0; i < sensor_id_energyRequired.size(); i++) {
		string temp_sensor = sensor_id_energyRequired[i].first;
		double temp_energy = sensor_id_energyRequired[i].second;
		sensors[temp_sensor].now_energy -= temp_energy;
	}

	return make_pair(cost_trans, cost_proc);
}


void SimpleNetwork::energy_harvest_receive_energy(double charging_time, map<string, double> power_received, double power_min, double power_0max) {
	if (charging_time > 0) {
		// 给网络中所有的sensor充电
		for (map<string, Sensor>::iterator iter = sensors.begin(); iter != sensors.end(); iter++) {
			string temp_sensor = iter->first;
			sensors[temp_sensor].now_energy += charging_time * power_received[temp_sensor];
		}
	}
}


vector<int> SimpleNetwork::generate_known_attributes(string object_name) {
	vector<int> result;
	// 只需要调度监测该object的sensor工作，那么已知属性最多不会超过监测该object的sensor的数量
	map<string, double> sensors_work = object_sensor_distance[object_name];
	for (map<string, Sensor>::iterator iter = sensors.begin(); iter != sensors.end(); iter++) {
		string temp_sensor = iter->first;
		if (sensors_work.count(temp_sensor) == 0) { // 如果是不需要工作的Sensor，需要看当前其是否已经有数据
			if (sensors[temp_sensor].life_count == 0) { // 如果没有数据
				continue;
			}
			else if (sensors[temp_sensor].life_count > 0) { // 如果传输过数据，修改保质期，并判断是否过期
				sensors[temp_sensor].life_count++;
				if (sensors[temp_sensor].life_count >= LIFE) {
					sensors[temp_sensor].life_count = 0;
				}
			}
		}
		else {
			if (sensors[temp_sensor].life_count == 0) { // 如果没传输过数据
				sensors[temp_sensor].life_count = 1;
				if (sensors[temp_sensor].life_count >= LIFE) { // 如果保质期为0或者1的话，就不会保留数据
					sensors[temp_sensor].life_count = 0;
				}
			}
			else {
				result.push_back(sensor_attribute[temp_sensor]);
				sensors[temp_sensor].life_count++;
				if (sensors[temp_sensor].life_count >= LIFE) {
					sensors[temp_sensor].life_count = 0;
				}
			}
		}
	}
	return result;
}


void SimpleNetwork::update_life_time(vector<string> sensors_id, double cost_time) {
	// 这次工作的sensor的保质期都设为 cost_time
	string work_sensor = sensors_id[0]; // 因为在渐进时，只有一个sensor需要传输数据
	for (map<string, Sensor>::iterator iter = sensors.begin(); iter != sensors.end(); iter++) {
		string temp_sensor = iter->first;
		
		if (temp_sensor == work_sensor) { // 工作的sensor
			if (sensors[temp_sensor].life_time >= 0) { 
				sensors[temp_sensor].life_time += cost_time;
				if (sensors[temp_sensor].life_time >= LIFE_TIME) {
					sensors[temp_sensor].life_time = -1;
				}
			}
			else {
				sensors[temp_sensor].life_time = 0; // 工作的sensor传输的数据是最新的
				if (sensors[temp_sensor].life_time >= LIFE_TIME) { // 可能直接就过期了，当life_time设为0的时候就有可能
					sensors[temp_sensor].life_time = -1;
				}
			}
		}
		else { // 其他的sensor
			if (sensors[temp_sensor].life_time == -1) { // 如果没传输过数据或者已经过期，不用管
				
			}
			else if(sensors[temp_sensor].life_time >= 0) { // 如果还在保质期之内，更新保质期，并判断是否过期
				sensors[temp_sensor].life_time += cost_time;
				if (sensors[temp_sensor].life_time >= LIFE_TIME) {
					sensors[temp_sensor].life_time = -1;
				}
			}
		}
	}
}