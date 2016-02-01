/*
 *	File name   : pbdata_handle.c
 *  Created on  : Jan 22, 2016
 *  Author      : pengjf
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "pb.h"
#include "SmartStore.pb.h"
#include "cJSON.h"
#include "MQTTClient.h"
#include "common.h"
#include "mt_topic_handle.h"
#include "zigbee_client.h"
#include "log_printf.h"
#include "msgtype.h"
#include "pbdata_handle.h"


/**
 * encode the gateway message and write the encoded buf to GATEWAY_MSG_PATH
 * @param ssmsg
 */
void gwmsg_write(SSMsg *ssmsg)
{
	uint8_t buffer[1024];
	int msg_len;
	FILE *fd;
	int status;

	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	status=pb_encode(&stream, SSMsg_fields, ssmsg);
	msg_len=stream.bytes_written;

	if (!status)
	{
		log_printf(LOG_ERROR,"GateWay Msg Encoding failed: %s\n", PB_GET_ERROR(&stream));
		return ;
	}

	fd=fopen(GATEWAY_MSG_PATH,"wb");
	if(fd==NULL)
	{
		log_printf(LOG_ERROR,"Can not open %s error!\n",GATEWAY_MSG_PATH);
	}
	fwrite(buffer,msg_len,1,fd);

	fclose(fd);
}



/**
 * judge whether nwk_addr(dev) exists or not
 * @param nwk_addr
 * @return 0 means exist, 1 means doesn't exist
 */
int dev_exist(char *nwk_addr)
{

	if(nwk_addr==NULL)return 1;

	char path[50]={'\0'};

	strcat(path,DEVINFO_DIR);
	strcat(path,nwk_addr);

	if (access(path, F_OK) == 0)
	{
		return 0;
	}else
	{
		return 1;
	}
}


/**
 * get device_id, zone_type form /gl/etc/devlist/"nwk_addr"
 * @param args
 * @return
 */
int get_deviceid_zonetype(char *device_id,char *zone_type,char *nwk_addr)
{
	char path[50]={'\0'};
	char temp[30];
	FILE *fd;
	int i,len;

	if(nwk_addr==NULL)return -1;
	strcat(path,DEVINFO_DIR);
	strcat(path,nwk_addr);


	if (access(path, F_OK) == 0)
	{
		fd = fopen(path, "r");
		if(fd==NULL)
		{
			log_printf(LOG_ERROR,"Can't not open %s files!\n",path);
		}

		for (i = 0; i < 8; i++)
		{
			fgets(temp, sizeof(temp), fd);
		}

		len = strlen(temp);
		for (i = 0; i <= len; i++)
		{
			if (temp[i] == '\n')
				temp[i] = 0;
		}

		strcpy(device_id,temp);

		fgets(temp, sizeof(temp), fd);
		len = strlen(temp);
		for (i = 0; i <= len; i++)
		{
			if (temp[i] == '\n')
				temp[i] = 0;
		}
		strcpy(zone_type,temp);

	}else
	{
		log_printf(LOG_VERBOSE,"Device Info File (%s) doesn't exist!\n ",path);
		return -1;
	}

	return 0;
}



/**
 * update/write device info file
 * @param args
 * @return
 */
int devinfo_file_update(char **args)
{
	FILE *fd;
	int len,i;

	char path[30] = DEVINFO_DIR;
	strcat(path,args[1]);
	log_printf(LOG_VERBOSE,"Update Device Info File:%s!\n",path);

	len=atoi(args[5])*7+6;
	fd=fopen(path,"w");
	if(fd==NULL)log_printf(LOG_ERROR,"Can't not open %s file!\n",path);

	for(i=0;i<len;i++)
	{
		fputs(args[i],fd);
		fputs("\n",fd);
	}
	//fwrite(args,1,len,fd);
	fclose(fd);
	return 1;
}


/**
 * Message EPS decode callback function
 * @param stream
 * @param field
 * @param arg
 * @return
 */
bool ss_eps_decode(pb_istream_t *stream, const pb_field_t *field, void **arg)
{

	EPS eps=EPS_init_zero;
	if (!pb_decode(stream, EPS_fields, &eps))
		   return false;

	return true;

}


/**
 * Message EPS encode callback function
 * @param stream
 * @param field
 * @param arg
 * @return
 */
bool ss_eps_encode(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	EPS eps=EPS_init_zero;

	log_printf(LOG_VERBOSE,"Encoding EPS SubMessage!\n");
	char** arg_eps = (char **)arg[0];

	int i;
	int count=atoi(arg_eps[0]);
	arg_eps=arg_eps+1;

	for(i=0;i<count;i++)
	{
		arg_eps=arg_eps+i*7;

		strcpy(eps.profile_id,arg_eps[0]);
		eps.has_profile_id=true;
		strcpy(eps.device_id,arg_eps[1]);
		eps.has_device_id=true;
		strcpy(eps.zone_type,arg_eps[2]);
		eps.has_zone_type=true;
		strcpy(eps.ep,arg_eps[3]);
		eps.has_ep=true;
		strcpy(eps.ep_name,arg_eps[4]);
		eps.has_ep_name=true;
		eps.rid=atoi(arg_eps[5]);
		eps.has_rid=true;
		eps.arm=atoi(arg_eps[6]);
		eps.has_arm=true;

		if (!pb_encode_tag_for_field(stream, field))
			return false;

		//This encodes the data for the field, based on our FileInfo structure.
		if (!pb_encode_submessage(stream, EPS_fields, &eps))
			return false;
	}



	return true;
}


/**
 * Message Dev encode callback function
 * @param stream
 * @param field
 * @param arg
 * @return
 */
bool ss_pb_encode_devlist(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{

	int i,j;
	FILE *fd;

	log_printf(LOG_VERBOSE,"Encoding Devlist SubMessage!\n");
	char *args[100];
	char temp_str[100][40];
	char temp[40];
	Dev devlist=Dev_init_zero;

	struct dirent *ptr=NULL;
	DIR *dir;
	dir=opendir("/gl/etc/devlist");

	while((ptr=readdir(dir))!=NULL)
	{

		//跳过’.'和’..’两个目录
		if (ptr->d_name[0] == '.')
			continue;

		//printf("%s is ready...\n",ptr->d_name);
		char path[35]={'\0'};
		strcat(path,DEVINFO_DIR);
		strcat(path, ptr->d_name);
		fd = fopen(path, "r");
		if(fd==NULL)log_printf(LOG_ERROR,"Can't open path file!\n",path);

		j=0;
		while (!feof(fd))
		{

			fgets(temp, sizeof(temp), fd);
			int L;
			L = strlen(temp);
			for (i = 0; i <= L; i++)
				if (temp[i] == '\n')
				{
					temp[i] = 0;
				}
			strcpy(temp_str[j], temp);
			args[j]=temp_str[j];
			//printf("read test:%s\n",args[j]);
			j++;
		}

		strcpy(devlist.ieee, args[0]);
		devlist.has_ieee = true;
		strcpy(devlist.nwk_addr, args[1]);
		devlist.has_nwk_addr = true;
		strcpy(devlist.node_name, args[2]);
		devlist.has_node_name = true;
		devlist.online = atoi(args[3]);
		devlist.has_online = true;
		devlist.node_status = atoi(args[4]);
		devlist.has_node_status = true;
		devlist.eps.funcs.encode = &ss_eps_encode;
		devlist.eps.arg = args + 5;

		if (!pb_encode_tag_for_field(stream, field))
			return false;

		//This encodes the data for the field, based on our FileInfo structure.
		if (!pb_encode_submessage(stream, Dev_fields, &devlist))
			return false;

		fclose(fd);
	}

	return true;
}


/**
 * Message SSMsg encode
 * @param buffer
 * @param buf_len
 * @param msg_type
 * @param args
 * @return
 */
int ss_pb_encode(uint8_t *buffer,int buf_len,int msg_type,char **args)
{
	SSMsg ssmsg=SSMsg_init_zero;
	ssmsg.msgtype=msg_type;

	switch(msg_type)
	{
		case get_gateway_msg_reqtype:
			//printf("arg[0]:%s",args[0]);
			ssmsg.result=atoi(args[0]);
			ssmsg.has_result=true;
			printf("msgtype :%d execution\n",ssmsg.result);
			if(ssmsg.result!=0)break;
			ssmsg.has_devinfo=true;
			strcpy(ssmsg.devinfo.manufacture,args[1]);
			ssmsg.devinfo.has_manufacture=true;
			strcpy(ssmsg.devinfo.model,args[2]);
			ssmsg.devinfo.has_model=true;
			strcpy(ssmsg.devinfo.sn,args[3]);
			ssmsg.devinfo.has_sn=true;
			strcpy(ssmsg.devinfo.mac,args[4]);
			ssmsg.devinfo.has_mac=true;
			strcpy(ssmsg.devinfo.sw_ver,args[5]);
			ssmsg.devinfo.has_sw_ver=true;
			strcpy(ssmsg.devinfo.hw_ver,args[6]);
			ssmsg.devinfo.has_hw_ver=true;
			strcpy(ssmsg.name,args[7]);
			ssmsg.has_name=true;
			ssmsg.status=atoi(args[8]);
			ssmsg.has_status=true;
			break;
		case get_devlist_reqtype:
			ssmsg.result=0;
			ssmsg.has_result=true;

			ssmsg.devlist.funcs.encode=&ss_pb_encode_devlist;

			break;
		case set_allow_jonin_net_reqtype:
			ssmsg.result=atoi(args[0]);
			ssmsg.has_result=true;
			break;
		case del_dev_reqtype:
			ssmsg.result=atoi(args[0]);
			ssmsg.has_result=true;
			break;
		case stop_alarm_reqtype:
			ssmsg.result=atoi(args[0]);
			ssmsg.has_result=true;
			break;

		case new_dev_added_updtype:
			devinfo_file_update(args);
			ssmsg.has_dev=true;
			strcpy(ssmsg.dev.ieee,args[0]);
			ssmsg.dev.has_ieee=true;
			strcpy(ssmsg.dev.nwk_addr,args[1]);
			ssmsg.dev.has_nwk_addr=true;
			strcpy(ssmsg.dev.node_name,args[2]);
			ssmsg.dev.has_node_name=true;
			ssmsg.dev.online=atoi(args[3]);
			ssmsg.dev.has_online=true;
			ssmsg.dev.node_status=atoi(args[4]);
			ssmsg.dev.has_node_status=true;
			ssmsg.dev.eps.funcs.encode=&ss_eps_encode;
			ssmsg.dev.eps.arg=args+5;
			break;
		case del_dev_updtype:
			strcpy(ssmsg.nwk_addr,args[0]);
			ssmsg.has_nwk_addr=true;
			break;
		case dev_alarm_udptype:
			strcpy(ssmsg.nwk_addr,args[0]);
			ssmsg.has_nwk_addr=true;
			strcpy(ssmsg.ep,args[1]);
			ssmsg.has_ep=true;
			ssmsg.w_mode=atoi(args[2]);
			ssmsg.has_w_mode=true;
			ssmsg.date_time=atoi(args[3]);
			ssmsg.has_date_time=true;
			strcpy(ssmsg.device_id,args[4]);
			ssmsg.has_device_id=true;
			strcpy(ssmsg.zone_type,args[5]);
			ssmsg.has_zone_type=true;
			break;
		default:
			break;

	}

	pb_ostream_t stream = pb_ostream_from_buffer(buffer, buf_len);
	pb_encode(&stream, SSMsg_fields, &ssmsg);

	return stream.bytes_written;
}


/**
 * Message EPS decode callback function
 * @param stream
 * @param field
 * @param arg
 * @return
 */
bool ss_decode_eps(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
	printf("eps_decode_callback executing\n");

	EPS eps=EPS_init_zero;
	if (!pb_decode(stream, EPS_fields, &eps))
		   return false;

	printf("in eps decode function, profile_id:%s\n",eps.profile_id);

	return true;

}


/**
 * Message devlist decode callback funciton
 * @param stream
 * @param field
 * @param arg
 * @return
 */
bool ss_decode_devlist(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
	  Dev devlist=Dev_init_zero;
	  log_printf(LOG_VERBOSE,"Devlist SubMessage Decoding!\n");
	  devlist.eps.funcs.decode=&ss_decode_eps;

	 if (!pb_decode(stream, Dev_fields, &devlist))
	 {
		 //printf("devlist_decode_callback failed!\n");
		 return false;
	 }

	 //printf("ieee:%s\n",devlist.nwk_addr);

	 return true;
}


/**
 * Message SSMsg decode function
 * @param buffer
 * @param buf_len
 * @param ssmsg
 * @return
 */
int ss_pb_decode(uint8_t *buffer,int buf_len,SSMsg *ssmsg)
{
	bool status;

	pb_istream_t stream = pb_istream_from_buffer(buffer, buf_len);
	//ssmsg->devlist.funcs.decode=&ss_decode_devlist;

	log_printf(LOG_VERBOSE,"SSMsg Decoding! \n");
	status=pb_decode(&stream, SSMsg_fields, ssmsg);

	if (!status)
	{
	       log_printf(LOG_ERROR,"Decoding failed: %s\n", PB_GET_ERROR(&stream));
	       return -1;
	}

	return 1;
}


/**
 * SSMsg paresed form mqtt message handle function
 * @param ssmsg
 */
void ssmsg_handle(SSMsg *ssmsg,const char* topicname)
{
	uint8_t buffer[MQTT_BUFFER_LEN];
	int msg_len;

	switch(ssmsg->msgtype)
	{
		case get_gateway_msg_reqtype: //todo: 换成宏或者enum
		{
			FILE *fd;
			fd=fopen(GATEWAY_MSG_PATH,"rb");
			if (fd == NULL)
			{
				log_printf(LOG_ERROR, "Can't open %s file!\n",
						GATEWAY_MSG_PATH);
				return;
			}
			fseek(fd,0,SEEK_END);
			msg_len = ftell(fd);
			rewind(fd);
			fread(buffer,1,msg_len,fd);

			publish_msg_2_mqtts_qos0(topicname, protobuf_header, 0x0091, msg_len, buffer);
			fclose(fd);
		}
			break;
		case get_devlist_reqtype:
			msg_len=ss_pb_encode(buffer,sizeof(buffer),get_devlist_reqtype,NULL);
			publish_msg_2_mqtts_qos0(topicname, protobuf_header, 0x0091, msg_len, buffer);
			break;
		case set_allow_jonin_net_reqtype:
		{
			access_netword();
			char *args[] = {"0"};
			msg_len = ss_pb_encode(buffer,sizeof(buffer),set_allow_jonin_net_reqtype,args);
			publish_msg_2_mqtts_qos0(topicname,protobuf_header,3,msg_len,buffer);
		}
			break;
		case del_dev_reqtype:
		{
			char path[50]={'\0'};
			strcat(path,DEVINFO_DIR);
			strcat(path,ssmsg->nwk_addr);

			if(access(path,F_OK)==0)
			{

				if (!del_dev(ssmsg->nwk_addr, ssmsg->ieee,ssmsg->ep))
				{
					remove(path);

					char *args[] = {"0"};
					msg_len = ss_pb_encode(buffer,sizeof(buffer),del_dev_reqtype,args);
					publish_msg_2_mqtts_qos0(topicname,protobuf_header, 4 , msg_len,buffer);
				}

			} else
			{
				// dev doesn't exist, return failure code
				char* args[] = { "-1" };
				msg_len = ss_pb_encode(buffer, sizeof(buffer), del_dev_reqtype, args);
				publish_msg_2_mqtts_qos0(topicname, protobuf_header, 0x0091, msg_len,buffer);
			}

		}
			break;
		case stop_alarm_reqtype:
		{
			stop_alarm();
			char *args[] = {"0"};
			msg_len = ss_pb_encode(buffer,sizeof(buffer),stop_alarm_reqtype,args);
			publish_msg_2_mqtts_qos0(topicname,protobuf_header,7,msg_len,buffer);
		}
			break;
		default:
			break;
	}

}


