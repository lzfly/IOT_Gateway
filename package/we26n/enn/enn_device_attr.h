#ifndef ENN_DEVICE_ATTR_H
#define ENN_DEVICE_ATTR_H


//水表-用水量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_WATERMETER_VALUE 1013

//电表-用电量
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_POWERMETER_VALUE 1012

//开关状态控制
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_ON_OFF_THREE_STATE 1016
//数据类型： int 32bit

//窗帘控制 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_VALUE   1004
#define ENN_DEVICE_ATTR_WINDOWS_RANGE   1005


//可调亮度色温灯属性 
//数据类型： int 32bit 
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE  1001
//数据类型： int 32bit  min:1 max:100
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE 1002
//数据类型： int 32bit min:1 max:100
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE 1003

//温湿度 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_TEMP_VALUE 1014
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_HUM_VALUE  1015

//PM2.5
//数据类型： int 32bit 
#define ENN_DEVICE_ATTR_PM25_VALUE 1006

//甲醛报警 
#define ENN_DEVICE_ATTR_FORMAL 1007

//燃气报警
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_GAS_ALERT 1009 
//CONCENTRATION
#define ENN_DEVICE_ATTR_GAS_CONCENTRATION 1008 

//烟雾
#define ENN_DEVICE_ATTR_SMOKE_CONCENTRATION 1010
//CONCENTRATION
#define ENN_DEVICE_ATTR_SMOKE_ALERT 1011 

#endif
