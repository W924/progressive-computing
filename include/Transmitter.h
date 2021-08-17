#pragma once
#include <vector>

class Transmitter
{
public:
	double transmission_power = 3; // 传输功率，单位为 W
	double frequency = 915; // 频率，单位为 MHZ
	double K_const = 4725; // 计算sensor接收功率时的参数，单位为 uJ，sensor的接收功率为 P = K / d^2，其中 d 为sensor到transmitter的距离

	double location_x, location_y;

	Transmitter();
	Transmitter(double x_val, double y_val);
	virtual ~Transmitter();
	
};
