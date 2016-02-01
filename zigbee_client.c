/*
 *	File name   : zigbee_client.c
 *  Created on  : Jan 14, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "zigbee_client.h"
#include "zigbee_protocol.h"
#include "ev_socket.h"
#include "common.h"
#include "log_printf.h"
#include "ev_wrap.h"
#include "linked_list_queue.h"
#include "pbdata_handle.h"
#include "alarm_control.h"
#include "msgtype.h"
#include "port.h"
//发送给zigbee中间件的固定二进制命令
const unsigned char get_dev_list[8] = {0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x81};
const unsigned char get_group[8] = {0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x8E};
const unsigned char set_permit_join_on[8] = {0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x9F};
const unsigned char reset_gateway_filesystem[12] = {0x0C, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xA1, 0x03, 0x55, 0xAA, 0x0A };
const unsigned char reset_all_data[12] = {0x0C, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xA1, 0x03, 0x55, 0xAA, 0x50};
char *del_dev_data = "1500FFFFFFFFFE950C02";
char ep[3] = {0};
char *ieee_low = "004B1200";
char ieee_high[5] = {0};
char ieee[17] = {0};
char zone_type[5] = {0};
char nwk_addr[5] = {0};
char online_flag[3] = {0};
char online[2] = {0};
char profile_id[5] = {0};
char device_id[5] = {0};
char ep_name[20] = {0};
int on_off;
void zigbee_socket_close(zigbee_client_st *zc)
{
	ev_fd_disconnect(zc->my_socket);
	zc->my_socket = -1;
	zc->isconnected = socket_notconnect;
	llqueue_free(zc->send_queue);  //release send queue
}
void zibee_client_para_init(zigbee_client_st *zc)
{
	zc->reconnect_count = zc->reconnect_count+1;

	if(zc->reconnect_count >RECONNECT_MAX_COUNT)
	{
		log_printf(LOG_ERROR, "[ZigbeeReConnectExceedMax]\n");
		exit(1); //超过重连次数退出程序
	}
}
void zigbee_reconnect()
{
	zigbee_socket_close(&global_ev_all.zigbee_client);
	stop_watchers("zigbee");
	zibee_client_para_init(&global_ev_all.zigbee_client);
	zigbee_init();
}
/*
 * description:处理zigbee中间件发送过来的数据
 * input: cev-ev结构体指针，buf-，
 * */
void zigbee_msg_handle(void *cev_void, const unsigned char *buf, int buflen)
{
	int i;
	uint8_t *ucharbuf;
	uint8_t str[1000]={0};
	ucharbuf =(uint8_t*) buf;
	time_t tm;
	char date_time[11];
	log_printf(LOG_NOTICE, "[RecvFromZW]....\n", buf);
	for(i=0; i<buflen; i++)
	{
		printf("[0x%x]",  ucharbuf[i]);
	}
	printf("\n");
	//EV_ALL *cev = (EV_ALL *)cev_void;
	HexToStr(str,ucharbuf, buflen);
	tm = time(NULL);						//get time
	snprintf(date_time,11,"%ld",tm);

/////////////////////////////////////////////////
	//TODO:
	//解析数据
	switch(ucharbuf[0])
	{
		case DEVICE_STATE_RES_TYPE:
			//"返回码类型"+"后续长度+"+"指令"+"发送的指令"
			//更改设备名称的指令时0x03，且没有包含"发送的指令"。其它一般指令是0x04
			printf("i am 29\n");
			switch(ucharbuf[3])
			{
				case SET_PERMIT_JOIN_ON:
					break;
				case DEL_DEVICE_SUCCESS:
					break;
				default:break;
			}
				//todo : 后面再做
			break;
		case DEV_INFO_RES_TYPE://获取新设备
			if((ucharbuf[1] == 0x2A) || (ucharbuf[1] == 0x2B))
			{
				snprintf(nwk_addr,5,"%c%c%c%c",str[4],str[5],str[6],str[7]);
				//check nwk_addr existing?
				if(dev_exist(nwk_addr))
				{
					printf("nwk_addr: %s\n",nwk_addr);
					snprintf(ep,3,"%c%c",str[8],str[9]);
					printf("%s\n",ep);
					snprintf(ieee,17,"%c%c%c%c%c%c%c%c%s",str[24],str[25],str[26],str[27],str[28],str[29],str[30],str[31],ieee_low);
					printf("ieee: %s\n",ieee);
					snprintf(profile_id,5,"%c%c%c%c",str[10],str[11],str[12],str[13]);
					printf("profile_id: %s\n",profile_id);
					snprintf(device_id,5,"%c%c%c%c",str[16],str[17],str[14],str[15]);
					printf("device_id: %s\n",device_id);
					snprintf(online_flag,3,"%c%c",str[22],str[23]);
					printf("online_flag: %s\n",online_flag);
					if(!strcmp(online_flag, "03"))
					{
						online[0] = '1';
						printf("%s\n i am online\n",online);
					}
					else
					{
						online[0] = '0';
						printf("%s\n i am un_online\n",online);
					}
					//to save
					if(ucharbuf[1] == 0x2A)
					{
						snprintf(zone_type,5,"%c%c%c%c",str[74],str[75],str[72],str[73]);
						printf("zone_type: %s\n",zone_type);
					}
					if(ucharbuf[1] == 0x2B)
					{
						snprintf(zone_type,5,"%c%c%c%c",str[76],str[77],str[74],str[75]);
						printf("zone_type: %s\n",zone_type);
					}
					//判断zone_type
					sensor_type(zone_type);
					char *list[] = {ieee,nwk_addr,"",online,"","1",profile_id,device_id,zone_type,ep,ep_name,"","1"};
					uint8_t buffer[500];
					int len = ss_pb_encode(buffer,500,300,list);
					publish_msg_2_mqtts_qos0(&pubtopics[0][0], protobuf_header, 300, len, buffer);
				}
			}
			else	//设备没有正确操作入网
			{
				printf("设备重新入网\n");
			}
			break;
		case GET_GATEWAY_INFO_UPLOAD_TYPE:
			break;
		case REPORT_UPLOAD_TYPE:
			snprintf(nwk_addr,5,"%c%c%c%c",str[4],str[5],str[6],str[7]);
			//check nwk_addr which device ->return zonetype?
			//to pb do
			if(!dev_exist(nwk_addr))
			{
				unsigned short int ucharbuf_word;
				ucharbuf_word = (ucharbuf[6] << 8) + ucharbuf[5];
				if(ucharbuf_word == SECURITY_DEVICE)
				{
					if(!get_deviceid_zonetype(device_id,zone_type,nwk_addr))
					{
						ucharbuf_word = (ucharbuf[8] << 8) + ucharbuf[9];
						if(ucharbuf_word == ALARM_STATE)
						{
							union_byte ucharbuf_byte;
							uint8_t buffer[50];
							int len;
							printf("%s\n",zone_type);
							snprintf(ep,3,"%c%c",str[8],str[9]);
							ucharbuf_byte.int8u = ucharbuf[11];
							w_mode_st w_mode = {0};
							uint8_t alarm_flag = 0;
							if(!strcmp(zone_type,"0015"))		//门磁
							{
								//if(ucharbuf_byte.ByteBit8.b4)		//非周期
								{
									if(ucharbuf_byte.ByteBit8.b0)
									{
										w_mode.burglar = 1;	//on
										alarm_control(burglar_alarm,gateway_info.duration);
										printf("door_open\n");
									}
									if(ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;	//low_voltage
										alarm_control(lowbattery_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b0 && ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;
										w_mode.burglar = 1;
										alarm_flag = 1;
									}
								}
							}
							if(!strcmp(zone_type,"000D"))		//红外
							{
//								if(ucharbuf_byte.ByteBit8.b4)		//非周期
								{
									if(ucharbuf_byte.ByteBit8.b0)
									{
										w_mode.burglar = 1;	//on
										alarm_control(burglar_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;	//low_voltage
										alarm_control(lowbattery_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b0 && ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;
										w_mode.burglar = 1;
										alarm_flag = 1;
									}
								}
							}
							if(!strcmp(zone_type,"0028"))		//烟雾
							{
//								if(ucharbuf_byte.ByteBit8.b4)		//非周期
								{
									if(ucharbuf_byte.ByteBit8.b0)
									{
										w_mode.fire_or_water = 1;	//on
										alarm_control(fire_or_water_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;	//low_voltage
										alarm_control(lowbattery_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b0 && ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;
										w_mode.fire_or_water = 1;
										alarm_flag = 1;
									}
								}
							}
							if(!strcmp(zone_type,"002A"))		//水浸
							{
//								if(ucharbuf_byte.ByteBit8.b4)		//非周期
								{
									if(ucharbuf_byte.ByteBit8.b0)
									{
										w_mode.fire_or_water = 1;	//on
										alarm_control(fire_or_water_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;	//low_voltage
										alarm_control(lowbattery_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b0 && ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;
										w_mode.fire_or_water = 1;
										alarm_flag = 1;
									}
								}

							}
							if(!strcmp(zone_type,"0115"))		//安防遥控器
							{
//								if(ucharbuf_byte.ByteBit8.b4)		//非周期
								{
									if(ucharbuf_byte.ByteBit8.b1)
									{
										//w_mode.emergency = 1;	//on
										alarm_control(0,0);
									}
								}
							}
							if(!strcmp(zone_type,"002C"))		//紧急按钮
							{
//								if(ucharbuf_byte.ByteBit8.b4)		//非周期
								{
									if(ucharbuf_byte.ByteBit8.b1)
									{
										w_mode.emergency = 1;	//on
										alarm_control(emergency_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;	//low_voltage
										alarm_control(lowbattery_alarm,gateway_info.duration);
									}
									if(ucharbuf_byte.ByteBit8.b1 && ucharbuf_byte.ByteBit8.b3)
									{
										w_mode.lowbattery = 1;
										w_mode.emergency = 1;
										alarm_flag = 1;
									}
								}
							}
							if(w_mode.burglar)
							{
								alarm_control(burglar_alarm,gateway_info.duration);
								char *list[] = {nwk_addr,ep,"1",date_time,device_id,zone_type};
								len = ss_pb_encode(buffer,50,305,list);
								publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 305 , len,buffer);
							}
							if(w_mode.fire_or_water)
							{
								alarm_control(fire_or_water_alarm,gateway_info.duration);
								char *list[] = {nwk_addr,ep,"2",date_time,device_id,zone_type};
								len = ss_pb_encode(buffer,50,305,list);
								publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 305 , len,buffer);
							}
							if(w_mode.emergency)
							{
								alarm_control(emergency_alarm,gateway_info.duration);
								char *list[] = {nwk_addr,ep,"3",date_time,device_id,zone_type};
								len = ss_pb_encode(buffer,50,305,list);
								publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 305 , len,buffer);
							}
							if(w_mode.lowbattery)
							{
								char *list[] = {nwk_addr,ep,"5",date_time,device_id,zone_type};
								len = ss_pb_encode(buffer,50,305,list);
								publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 305 , len,buffer);
								if(!alarm_flag)
								{
									alarm_control(burglar_alarm,gateway_info.duration);
								}
							}

						}
					}
				}
			}
			break;
		default:break;
	}
}

//void sensor_type(char *type)
//{
//	snprintf(ieee_high,5,"%s",ieee);
//	if(!strcmp(zone_type,"0015"))
//	{
//		snprintf(ep_name,20,"%s-%s","DOOR",ieee_high);
//		printf("ep_name: %s\n",ep_name);
//	}
//	if(!strcmp(zone_type,"000D"))
//	{
//		snprintf(ep_name,20,"%s-%s","IR",ieee_high);
//		printf("ep_name: %s\n",ep_name);
//	}
//	if(!strcmp(zone_type,"0028"))
//	{
//		snprintf(ep_name,20,"%s-%s","SMOKE",ieee_high);
//		printf("ep_name: %s\n",ep_name);
//	}
//	if(!strcmp(zone_type,"002A"))
//	{
//		snprintf(ep_name,20,"%s-%s","WATER",ieee_high);
//		printf("ep_name: %s\n",ep_name);
//	}
//	if(!strcmp(zone_type,"0115"))
//	{
//		snprintf(ep_name,20,"%s-%s","CONTROL",ieee_high);
//		printf("ep_name: %s\n",ep_name);
//	}
//	if(!strcmp(zone_type,"002C"))
//	{
//		snprintf(ep_name,20,"%s-%s","SOS",ieee_high);
//		printf("ep_name: %s\n",ep_name);
//	}
//
//} //modify by yanly
void sensor_type(char *type)
{
	snprintf(ieee_high,5,"%s",ieee);
	if(!strcmp(zone_type,"0015"))
	{
		snprintf(ep_name,EP_NAME_MAX_LEN,"%s-%s","门磁探测器",ieee_high);  //ep name max size is 30
		printf("ep_name: %s\n",ep_name);
	}
	if(!strcmp(zone_type,"000D"))
	{
		snprintf(ep_name,EP_NAME_MAX_LEN,"%s-%s","红外探测器",ieee_high);
		printf("ep_name: %s\n",ep_name);
	}
	if(!strcmp(zone_type,"0028"))
	{
		snprintf(ep_name,EP_NAME_MAX_LEN,"%s-%s","烟雾报警器",ieee_high);
		printf("ep_name: %s\n",ep_name);
	}
	if(!strcmp(zone_type,"002A"))
	{
		snprintf(ep_name,EP_NAME_MAX_LEN,"%s-%s","水浸探测器",ieee_high);
		printf("ep_name: %s\n",ep_name);
	}
	if(!strcmp(zone_type,"0115"))
	{
		snprintf(ep_name,EP_NAME_MAX_LEN,"%s-%s","遥控器",ieee_high);
		printf("ep_name: %s\n",ep_name);
	}
	if(!strcmp(zone_type,"002C"))
	{
		snprintf(ep_name,EP_NAME_MAX_LEN,"%s-%s","紧急按钮",ieee_high);
		printf("ep_name: %s\n",ep_name);
	}

}
void HexToStr(uint8_t *pbDest,uint8_t *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;
	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}
	pbDest[nLen*2] = '\0';
}

void StrToHex(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
 char h1,h2;
 uint8_t s1,s2;
 int i;

	for (i=0; i<nLen; i++)
 {
 h1 = pbSrc[2*i];
 h2 = pbSrc[2*i+1];

 s1 = toupper(h1) - 0x30;
 if (s1 > 9)
		s1 -= 7;

 s2 = toupper(h2) - 0x30;
 if (s2 > 9)
		s2 -= 7;

 pbDest[i] = s1*16 + s2;
 }
}
//pb接口
//ev_send_msg_2_zigbees(&global_ev_all, temp, strlen(temp)+1);
void access_netword()				//允许入网
{
	ev_send_msg_2_zigbees(&global_ev_all, set_permit_join_on, sizeof(set_permit_join_on));

	///////////////  modify by yanly
//	char *args[] = {"0"};
//	uint8_t buffer[50];
//	int len = ss_pb_encode(buffer,50,3,args);
//	publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 3 , len,buffer);
	///////////////
}
//
int del_dev(char *nwk_addr_data,char *ieee_data,char *ep)		//删除设备
{

	uint8_t pbDest[21];
	char src[50];
	int len;
	uint8_t buffer[50];
	snprintf(src,50,"%s%s%s%s",del_dev_data,nwk_addr_data,ieee_data,ep);
	StrToHex(pbDest, (uint8_t *)src, 21);
	ev_send_msg_2_zigbees(&global_ev_all, pbDest, sizeof(pbDest));
	//成功删除信息
	///////////////  modify by yanly
//	char *args[] = {"0"};
//	len = ss_pb_encode(buffer,50,4,args);
//	publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 4 , len,buffer);
	///////////////
	//成功删除设备所对应的信息（短地址）
	char nwk_addr_temp[5];
	strcpy(nwk_addr_temp ,nwk_addr_data);
	char *list[] = {nwk_addr_temp};
	len = ss_pb_encode(buffer,50,301,list);
	publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 301, len,buffer);
	return 0;
}

void stop_alarm()								//停止报警
{
	alarm_control(0,0);
	///////////////  modify by yanly
//	char *args[] = {"0"};
//	uint8_t buffer[50];
//	int len = ss_pb_encode(buffer,50,7,args);
//	publish_msg_2_mqtts_qos0(&pubtopics[0][0],protobuf_header, 7 , len,buffer);
	///////////////
}

