#ifndef ENN_DEVICE_ATTR_H
#define ENN_DEVICE_ATTR_H


//水表-用水量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_WATERMETER_VALUE 1007

//电表-用电量
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_POWERMETER_VALUE 1006

//开关状态控制
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_ON_OFF_THREE_STATE 1013
//数据类型： int 32bit

//窗帘控制 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_VALUE   1001


//可调亮度色温灯属性 
//数据类型： int 32bit 
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE  1010
//数据类型： int 32bit  min:1 max:100
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE 1011
//数据类型： int 32bit min:1 max:100
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE 1012

//温湿度 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_TEMP_VALUE 1008
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_HUM_VALUE  1009

//PM2.5
//数据类型： int 32bit 
#define ENN_DEVICE_ATTR_PM25_VALUE 1002

//甲醛报警 
#define ENN_DEVICE_ATTR_FORMAL_VALUE 1003

//燃气报警
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_GAS_ALERT 1004

//门磁
#define ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT 1015

//人体红外
#define ENN_DEVICE_ATTR_BODY_INFRARED 1016

//开关插座i
#define ENN_DEVICE_ATTR_POWER_OUTLET 1014

//烟雾
#define ENN_DEVICE_ATTR_SMOKE_ALERT 1005 

//气表-用气量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_GASMETER_VALUE 1017

//热表-用热量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_HEATMETER_VALUE 1018


#endif
