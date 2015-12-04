#ifndef ENN_DEVICE_ATTR_H
#define ENN_DEVICE_ATTR_H


//水表-用水量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_WATERMETER_VALUE 1004

//电表-用电量
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_POWERMETER_VALUE 1004

//开关状态控制
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_ON_OFF_THREE_STATE 1001
//数据类型： int 32bit

//窗帘控制 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_VALUE   1010


//可调亮度色温灯属性 
//数据类型： int 32bit 
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE  1001
//数据类型： int 32bit  min:1 max:100
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE 1005
//数据类型： int 32bit min:1 max:100
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE 1007

//温湿度 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_TEMP_VALUE 1008
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_HUM_VALUE  1009

//PM2.5
//数据类型： int 32bit 
#define ENN_DEVICE_ATTR_PM25_VALUE 1003

//甲醛报警 
#define ENN_DEVICE_ATTR_FORMAL_VALUE 1003

//燃气报警
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_GAS_ALERT 1011

//门磁
#define ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT 1011

//人体红外
#define ENN_DEVICE_ATTR_BODY_INFRARED 1011

//开关插座i
#define ENN_DEVICE_ATTR_POWER_OUTLET 1001

//烟雾
#define ENN_DEVICE_ATTR_SMOKE_ALERT 1011 

//气表-用气量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_GASMETER_VALUE 1004

//热表-用热量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_HEATMETER_VALUE 1004


#endif
