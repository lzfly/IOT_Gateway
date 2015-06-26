#ifndef ENN_DEVICE_ATTR_H
#define ENN_DEVICE_ATTR_H


//用水量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_WATERMETER_VALUE 1013

//用电量
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_POWERMETER_VALUE 1011

//开关状态控制
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_ON_OFF_THREE_STATE 1019
//数据类型： int 32bit

//窗帘控制 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_VALUE   2005


//可调亮度灯属性 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_BRIGHTNESS_LAMP_STATE 1221
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_BRIGHTNESS_LAMP_BRIGHTNESS_VALUE 1006

//可调亮度色温灯属性 
//数据类型： int 32bit 
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE 2009
//数据类型： int 32bit  min:0 max:255
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE 2010
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE 2010

//温湿度 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_TEMP_VALUE 2011
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_HUM_VALUE  2012

//燃气报警
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_GAS_VALUE  2022


#endif