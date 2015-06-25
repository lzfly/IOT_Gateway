#ifndef ENN_DEVICE_ATTR_H
#define ENN_DEVICE_ATTR_H


//用水量 
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_WATERMETER_VALUE 2001

//用电量
//数据类型： double 64bit
#define ENN_DEVICE_ATTR_POWERMETER_VALUE 2002

//两路开关状态控制
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_ON_OFF_A_STATE 2003
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_ON_OFF_B_STATE 2004

//两路窗帘控制 
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_A_OPEN   2005
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_A_CLOSE   2006
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_B_OPEN   2007
//数据类型： int 32bit
#define ENN_DEVICE_ATTR_WINDOWS_B_CLOSE   2008

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


#endif