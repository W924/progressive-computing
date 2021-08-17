#include "Sensor.h"
#include "ConstDefine.h"

using namespace std;

Sensor::Sensor() {
	location_x = 0;
	location_y = 0;
	now_energy = SENSOR_INITIAL_ENERGY;
	life_count = 0;
	life_time = -1;
}
Sensor::Sensor(double x_val, double y_val) {
	location_x = x_val;
	location_y = y_val;
	now_energy = SENSOR_INITIAL_ENERGY;
	life_count = 0;
	life_time = -1;
}
Sensor::~Sensor() {

}


// battery-power用到的函数
double Sensor::energyTransTradition(int k, double probability) {
	return (k * unit_8_cost_trans + initial_cost_trans) * (1 / probability);
}
double Sensor::energyTransProg_NoControl(int k) {
	return k * unit_8_cost_trans + initial_cost_trans;
}
double Sensor::energyTransProg_WithControl(int k) {
	return k * unit_8_cost_trans + initial_cost_trans + control_cost_trans;
}
