#include "Object.h"

using namespace std;


Object::Object() {
	location_x = 0;
	location_y = 0;
}

Object::Object(double x_val, double y_val) {
	location_x = x_val;
	location_y = y_val;
}

Object::~Object() {

}