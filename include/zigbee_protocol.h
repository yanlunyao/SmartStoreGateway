/*
 *	File name   : zigbee_protocol.h
 *  Created on  : Jan 19, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_ZIGBEE_PROTOCOL_H_
#define INCLUDE_ZIGBEE_PROTOCOL_H_
#include <stdbool.h>

//zigbee模块上传码类型
#define DEVICE_STATE_RES_TYPE					0x29		//控制命令的一般返回码类型
#define DEV_INFO_RES_TYPE						0x01		//传感器设备信息返回码
#define GET_GATEWAY_INFO_UPLOAD_TYPE			0x15		//网关信息返回码
#define REPORT_UPLOAD_TYPE						0x70		//主动上报信息码
//
#define DEL_DEVICE_SUCCESS						0x95		//删除设备成功返回码
#define SET_PERMIT_JOIN_ON						0x9F		//允许入网成功返回码

//簇ID
#define SECURITY_DEVICE 		0x0500		//安防设备

//设备上报状态
#define ALARM_STATE			0x8000

//zigbee模块控制命令类型码
enum zigbee_api_ctrl_cmd {
	delete_dev_name_zcmd = 0x94,
	delete_dev_zigbee_zcmd = 0x95,
	get_gateway_info_zcmd = 0x9D,
	allow_join_on_net_zcmd = 0x9F,
	reset_gateway = 0xA1
};


//
typedef struct {
	bool burglar;
	bool fire_or_water;
	bool emergency;
	bool devicetrouble;
	bool lowbattery;
	bool doorbell;

} w_mode_st;

//报警类型
enum alarm_mode{
	none_alarm =0,
	burglar_alarm,
	fire_or_water_alarm,
	emergency_alarm,
	devicetrouble_alarm,
	lowbattery_alarm,
	doorbell_alarm,
	max_alarm
};

//默认报警时长
#define DEFAULT_ALARM_TIME		120

////////////////// length macro////////////////////////////
#define EP_NAME_MAX_LEN				30



extern const unsigned char get_dev_list[8];
extern const unsigned char get_group[8];
extern const unsigned char set_permit_join_on[8];
extern const unsigned char reset_gateway_filesystem[12];
extern const unsigned char reset_all_data[12];


#endif /* INCLUDE_ZIGBEE_PROTOCOL_H_ */
