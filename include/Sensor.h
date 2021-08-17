#pragma once

class Sensor
{
public:
	int transmission_rate = 250; // 传输速率，单位 kbps

	double voltage = 1.8; // 工作电压，单位 V
	double working_current = 28e-3; // 工作电流，单位 A
	double sleep_current = 1e-6; //休眠电流，单位 A
	double transmission_power = 50.4e-3; // 传输功率，单位 W
	double sleep_power = 1.8e-6; // 休眠功率，单位 W

	// 第一次实验的变量（传统使用电池，渐进使用RF充电）
	int unit_packet_size = 8; // 一次传输数据量，设为 8 bytes
	double unit_cost_trans = 66.9; // 发送 8 bytes数据包所需要的总能量，单位为 uJ
	double now_energy; // sensor当前存储的电量，单位为 uJ，在后来也都会用到该变量

	// 第二次实验的变量（都用电池）
	double initial_cost_trans = 105.6; // 唤醒，等ACK，传输报文表头等能量总和，uJ
	double control_cost_trans = 55.2; // 接收一个控制包需要的能量，uJ
	double unit_8_cost_trans = 12.9; // 传输8字节需要的能量，uJ

	double location_x, location_y; // sensor在网络中的位置

	int life_count; // 保质期
	double life_time;
	
	Sensor();
	Sensor(double x_val, double y_val);
	virtual ~Sensor();
	
	// battery-power 用到的函数
	double energyTransTradition(int k, double probability);
	double energyTransProg_NoControl(int k);
	double energyTransProg_WithControl(int k);

};
