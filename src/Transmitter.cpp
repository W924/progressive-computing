#include "Transmitter.h"

using namespace std;


Transmitter::Transmitter() {
	location_x = 0;
	location_y = 0;
}

Transmitter::Transmitter(double x_val, double y_val) {
	location_x = x_val;
	location_y = y_val;
}

Transmitter:: ~Transmitter() {

}