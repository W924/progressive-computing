#pragma once

#define SENSOR_INITIAL_ENERGY 0 // 网络中Sensor初始化时的能量
#define COST_TRANS_INITIAL 105.6 // sensor 唤醒，等ACK及传输报文头部需要消耗的能量
#define COST_TRANS_CONTROL 55.2 // sensor 控制包需消耗的能量
#define COST_TRANS_UNIT_8 12.9  // sensor 每传输 8 字节需要消耗的能量

#define K 16 // sensor传输包的大小，表示K*8为传输的字节数
#define RANDOM_SEED 1

#define PRINT_COST_MESSAGE true

#define RANDOM_OBJECT true // 在网络中选择object是否随机
#define OBJECT_ID_FIXED "o0"  // 若固定object，固定的object

#define RANDOM_KNOWN_ATTRIBUTE true  // 已知属性是否随机
#define RANDOM_KNOWN_ATTRIBUTE_NUMBER 4 // 若已知属性随机的话，已知属性的个数

#define LIFE 5 // 不随机已知属性的情况下，已知属性的保质期，record
#define LIFE_TIME 1000 // 秒

#define BATTERY_POWER_SAMPLE_NUMBER 50
#define RF_CHARGING_SAMPLE_NUMBER 50
#define ENERGY_HARVEST_SAMPLE_NUMBER 50

#define ENERGY_HARVEST_RECORD_RANDOM_INTERVAL false // 两条记录之间处理的时间间隔是否随机
#define ENERGY_HARVEST_RECORD_TIME_INTERVAL 0 // 若不随机，则手动设置
#define ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MIN 0.0 // 若时间间隔随机，间隔的上下界
#define ENERGY_HARVEST_RECORD_RANDOM_INTERVAL_MAX 1.0

#define AIR_RANDOM_SEED 0
#define AIR_SAMPLE_NUMBER 50
#define AIR_PRINT_MESSAGE true