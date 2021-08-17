#pragma once
#include <vector>

class Object
{
public:
	double location_x, location_y; // Object在网络中的位置
	std::vector<double> attributes_value; // Object每个属性的值
	
	Object();
	Object(double x_val, double y_val);
	virtual ~Object();
};
