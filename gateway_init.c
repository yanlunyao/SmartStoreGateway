/*
 *	File name   : gateway_init.c
 *  Created on  : Jan 21, 2016
 *  Author      : pengzhou
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "cJSON.h"
#include "file_operation.h"
#include "common.h"
#include "log_printf.h"
#include "gateway_init.h"
#include "ev_socket.h"
#include "zigbee_protocol.h"
#include "SmartStore.pb.h"
#include "pbdata_handle.h"
#include "msgtype.h"
#include "ev_wrap.h"


//get rand_passcode
void generate(int len,char* buffer)
{
	/*产生密码用的字符串*/
	static const char string[]= "0123456789abcdefghiljklnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i = 0;
	srand(time(0));
	for(; i < len; i++)
	{
		buffer[i] = string[rand()%strlen(string)]; /*产生随机数*/
	}
}

int gateway_setting_init()
{
	int res=0;
	char read_data[2048]={0};
	char* tmp;

	char need_2_rewrite =0; //为1时表示需要重写

	gateway_info_st gateway_init;
	cJSON* json_all;

	res = read_file_data(GATEWAY_INFO_PATH, read_data);
	if(res == 0)
	{
		json_all = cJSON_Parse(read_data);
		if(!json_all)
		{
			res = FILE_DATA_NOT_JSON;
			return res;
		}
	}
	else
	{
		log_printf(LOG_ERROR, "GATEWAY_INFO_PATH open failed\n");
	}
	cJSON* json_http_server = cJSON_GetObjectItem(json_all, "http_server");
	cJSON* json_http_port = cJSON_GetObjectItem(json_all, "http_port");
	cJSON* json_mt_server1 = cJSON_GetObjectItem(json_all, "mt_server1");
	cJSON* json_mt_server2 = cJSON_GetObjectItem(json_all, "mt_server2");
	cJSON* json_mt_port1 = cJSON_GetObjectItem(json_all, "mt_port1");
	cJSON* json_mt_port2 = cJSON_GetObjectItem(json_all, "mt_port2");
	cJSON* json_passcode = cJSON_GetObjectItem(json_all, "passcode");
	cJSON* json_mac = cJSON_GetObjectItem(json_all, "mac");
	cJSON* json_did = cJSON_GetObjectItem(json_all, "did");
	cJSON* json_productkey = cJSON_GetObjectItem(json_all, "productkey");
	cJSON* json_sn = cJSON_GetObjectItem(json_all, "sn");
	cJSON* json_duration = cJSON_GetObjectItem(json_all, "duration");

	//get duration
	gateway_init.duration = json_duration -> valueint;
	if(gateway_init.duration <0)
	{
		cJSON_ReplaceItemInObject(json_all, "duration", cJSON_CreateNumber(DEFAULT_ALARM_TIME));
		gateway_init.duration = DEFAULT_ALARM_TIME;
		need_2_rewrite =1;
	}

	//get http_server
	snprintf(gateway_init.http_server, sizeof(gateway_init.http_server), "%s", json_http_server -> valuestring);
	if(strlen(gateway_init.http_server) == 0)
	{
		snprintf(gateway_init.http_server, sizeof(gateway_init.http_server), "%s", HTTP_SERVER_HOST);
		cJSON_ReplaceItemInObject(json_all, "http_server", cJSON_CreateString(HTTP_SERVER_HOST));
		need_2_rewrite =1;
	}
	//get http_port
	gateway_init.http_port = json_http_port -> valueint;
	if(gateway_init.http_port == 0)
	{
		cJSON_ReplaceItemInObject(json_all, "http_port", cJSON_CreateNumber(HTTP_SERVER_PORT));
		gateway_init.http_port = HTTP_SERVER_PORT;
		need_2_rewrite =1;
	}
	//get passcode
	snprintf(gateway_init.passcode, sizeof(gateway_init.passcode), "%s", json_passcode -> valuestring);
	if(strlen(gateway_init.passcode) == 0)
	{
		char new_passcode[8];
		generate(8,new_passcode);
		cJSON* json_new_passcode = cJSON_CreateString(new_passcode);
		cJSON_ReplaceItemInObject(json_all, "passcode", json_new_passcode);
		need_2_rewrite =1;
		snprintf(gateway_init.passcode, sizeof(gateway_init.passcode), "%s", new_passcode);
	}
	//get mac
	snprintf(gateway_init.mac, sizeof(gateway_init.mac), "%s", json_mac -> valuestring);
	if(strlen(gateway_init.mac) == 0)
	{
		int res;
		char ifname[16];
 		unsigned char mac[6];
		char new_mac[12];
		strcpy(ifname, IF_NAME);
		memset(mac,0,sizeof(mac));
  		res = getmac(ifname,mac);
  		printf("mac: %02X%02X%02X%02X%02X%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		if(res == 0)
		{
			sprintf(new_mac,"%02X%02X%02X%02X%02X%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			cJSON* json_new_mac = cJSON_CreateString(new_mac);
			cJSON_ReplaceItemInObject(json_all, "mac", json_new_mac);
		}
		need_2_rewrite =1;
		snprintf(gateway_init.mac, sizeof(gateway_init.mac), "%s", new_mac);
	}
	//get did
	snprintf(gateway_init.did, sizeof(gateway_init.did), "%s", json_did -> valuestring);
	if(strlen(gateway_init.did) == 0)
	{
//		printf(".....i am did\n");//to http_res
	}
	//get mqtt_sever1
	snprintf(gateway_init.mt_server1, sizeof(gateway_init.mt_server1), "%s", json_mt_server1 -> valuestring);
	if(strlen(gateway_init.mt_server1) == 0)
	{
//		printf(".....i am mt_server1\n");//to http_res
	}
	//get mqtt_sever2
	snprintf(gateway_init.mt_server2, sizeof(gateway_init.mt_server2), "%s", json_mt_server2 -> valuestring);
	if(strlen(gateway_init.mt_server2) == 0)
	{
//		printf(".....i am mt_server2\n");//to http_res
	}
	//get mqtt_port1
	gateway_init.mt_port1 = json_mt_port1 -> valueint;
	if(gateway_init.mt_port1 == 0)
	{
//		printf(".....i am mt_port1\n");//to http_res
	}
	//get mqtt_port2
	gateway_init.mt_port2 = json_mt_port2 -> valueint;
	if(gateway_init.mt_port2 == 0)
	{
//		printf(".....i am mt_port2\n");//to http_res
	}
	//get product key
	snprintf(gateway_init.productkey, sizeof(gateway_init.productkey), "%s", json_productkey -> valuestring);
	if(strlen(gateway_init.productkey) == 0)
	{
		snprintf(gateway_init.productkey, sizeof(gateway_init.productkey), "%s", PRODUCT_KEY);
		cJSON_ReplaceItemInObject(json_all, "productkey", cJSON_CreateString(PRODUCT_KEY));
		need_2_rewrite =1;
	}
	//get sn
	snprintf(gateway_init.sn, sizeof(gateway_init.sn), "%s", json_sn -> valuestring);
	if(gateway_init.sn[0] == 0)
	{
		//todo: 可能从别的文件读
		snprintf(gateway_init.sn, sizeof(gateway_init.sn), "%s", gateway_init.mac);
		cJSON_ReplaceItemInObject(json_all, "sn", cJSON_CreateString(gateway_init.mac));
		need_2_rewrite =1;
	}
	tmp = cJSON_Print(json_all);
	log_printf(LOG_NOTICE, "%s\n",tmp);

	if(need_2_rewrite==1)
	{
		write_file_data(GATEWAY_INFO_PATH, tmp);
		log_printf(LOG_NOTICE, "rewrite GATEWAY_INFO_PATH\n");
	}

	gateway_info = gateway_init;
	cJSON_Delete(json_all);
	printf_gateway_info(&gateway_info);
	return 0;
}
//for test
void printf_gateway_info()
{
	log_printf(LOG_NOTICE, "[GatewayInfo]: ++++++++++++++++++++++\n");
	printf("http server: %s\n", gateway_info.http_server);
	printf("mqtt server1: %s\n", gateway_info.mt_server1);
	printf("mqtt port1: %d\n", gateway_info.mt_port1);
	printf("mqtt server2: %s\n", gateway_info.mt_server2);
	printf("mqtt port2: %d\n", gateway_info.mt_port2);
	printf("mac: %s\n", gateway_info.mac);
	printf("passcode: %s\n", gateway_info.passcode);
	printf("productkey: %s\n", gateway_info.productkey);
	printf("did: %s\n", gateway_info.did);
	printf("sn: %s\n", gateway_info.sn);
	log_printf(LOG_NOTICE, "++++++++++++++++++++++\n");
}

void gwmsg_init()
{
	if(access(GATEWAY_MSG_PATH, F_OK) ==0)
	{
		//file exist
		return;
	}

	SSMsg ssmsg = SSMsg_init_zero;
	DevInfo devinfo=DevInfo_init_zero;
	ssmsg.msgtype=1;
	ssmsg.has_result = true;
	ssmsg.has_devinfo = true;
	ssmsg.has_name = true;
	ssmsg.has_status = true;
	ssmsg.result = 0;
	ssmsg.status = 0;
	strcpy(ssmsg.name, "");

	devinfo.has_hw_ver = true;
	devinfo.has_mac = true;
	devinfo.has_manufacture = true;
	devinfo.has_model = true;
	devinfo.has_sn = true;
	devinfo.has_sw_ver = true;

	strcpy(devinfo.hw_ver, HW_VER);
	strcpy(devinfo.mac, gateway_info.mac);
	strcpy(devinfo.manufacture, MANUFACTURE);
	strcpy(devinfo.model, MODEL);
	strcpy(devinfo.sn, gateway_info.sn);
	strcpy(devinfo.sw_ver, SW_VER);

	ssmsg.devinfo = devinfo;

	gwmsg_write(&ssmsg);
}
/**
 * 设备信息下的网关信息，第一次运行时初始化添加进设备列表。
 * @param
 */
void devlist_gateway_msg_init()
{
	if(dev_exist(GATEWAY_NWKADDR)==0)
	{
		return; //存在退出
	}
	char *ieee = gateway_info.sn;
	char *ep_count = "1";
	char ep_name[NAME_LEN]={0};
	strncat(ep_name, GATEWAY_EP_NAME, sizeof(GATEWAY_EP_NAME));
	strncat(ep_name, "-", 1);
	strncat(ep_name, ieee+(strlen(ieee)-4), 4);

	char *list[] = {ieee, GATEWAY_NWKADDR, GATEWAY_NODE_NAME, GATEWAY_ONLINE, GATEWAY_NODE_STATUS,
			ep_count, GATEWAY_PROFILE_ID, GATEWAY_DEVICE_ID, GATEWAY_ZONETYPE,GATEWAY_EP, ep_name,
			GATEWAY_RID, GATEWAY_ARM};

	uint8_t buffer[500];
	ss_pb_encode(buffer, sizeof(buffer), new_dev_added_updtype, list);
}
/**
 * 网关初始化的时候，需要调用飞比API，获取设备列表
 *
 */
void dev_init()
{
	ev_send_msg_2_zigbees(&global_ev_all, get_dev_list, sizeof(get_dev_list));
}
