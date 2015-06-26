#ifndef ENN_DEVICE_ATTR_H
#define ENN_DEVICE_ATTR_H


//用水量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_WATERMETER_VALUE 2001

//用电量
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_POWERMETER_VALUE 2002

//三路开关状态控制
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_ON_OFF_THREE_STATE 2003
//数据类型： int 32bit

//两路窗帘控制 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_OPEN   2005
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_CLOSE   2006
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_STOP   2007


//可调亮度灯属性 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_BRIGHTNESS_LAMP_STATE 2009
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_BRIGHTNESS_LAMP_BRIGHTNESS_VALUE 2010

//温湿度 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_TEMP_VALUE 2011
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_HUM_VALUE  2012

//燃气
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_GAS_VALUE  2022


#endif