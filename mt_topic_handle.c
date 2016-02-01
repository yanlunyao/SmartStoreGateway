/*
 *	File name   : mt_topic_handle.c
 *  Created on  : Jan 13, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include "mt_topic_handle.h"
#include "common.h"
#include "zigbee_client.h"
#include "zigbee_protocol.h"

#include "SmartStore.pb.h"
#include "pbdata_handle.h"

#include "alarm_control.h"  //for test

void mh_none(MessageData* md, Client *c)
{}

//处理接收到的不同主题的mqtt数据
void mh_messageArrived(MessageData* md, Client *c)
{
	MQTTMessage* message = md->message;

	printf("%.*s\t\n", md->topicName->lenstring.len, md->topicName->lenstring.data);
	printf("%.*s\n", (int)message->payloadlen, (char*)message->payload);
}
void ser2cli_res_process(MessageData* md, Client *c)
{
	MQTTMessage* message = md->message;

	printf("%.*s\t\n", md->topicName->lenstring.len, md->topicName->lenstring.data);
	printf("%.*s\n", (int)message->payloadlen, (char*)message->payload);
}
void ser2cli_noti_process(MessageData* md, Client *c)
{
	MQTTMessage* message = md->message;

	printf("%.*s\t\n", md->topicName->lenstring.len, md->topicName->lenstring.data);
	printf("%.*s\n", (int)message->payloadlen, (char*)message->payload);

}
void app2dev_process(MessageData* md, Client *c)
{
	MQTTMessage* message = md->message;

	printf("%.*s\t\n", md->topicName->lenstring.len, md->topicName->lenstring.data);
	printf("%.*s\n", (int)message->payloadlen, (char*)message->payload);

	//MQTT message用连续内存保存topicname和payload，字符串没有截止符，先复制一份添加截止符
	char req_topicname[TOPIC_MAX_LEN]={0};
	memcpy(req_topicname,md->topicName->lenstring.data,md->topicName->lenstring.len);
	//replace "app2dev" with "dev2app" in string req_topicname
	char topicname[TOPIC_MAX_LEN]={0};
	strcat(topicname,"dev2app");
	strcat(topicname,strstr(req_topicname,"/"));

	//protobuf message type
	int buf_len;
	int mqtt_msg_len;
	int datatype;
	uint8_t *buffer=(uint8_t *)message->payload;

	memcpy(&datatype,buffer,4);
	memcpy(&mqtt_msg_len,buffer+4,4);

	/////////////////add by yanly
	printf("====================\n");
	int i;
	for(i=0; i<message->payloadlen; i++)
		printf("0x%x ", buffer[i]);
	printf("================\n");
	/////////////////////////

	if(datatype==protobuf_header)
	{
		SSMsg ssmsg=SSMsg_init_zero;
		buf_len=mqtt_msg_len-2;

		//printf("protobuf msg len=%d\n", buf_len);
		ss_pb_decode(buffer+10, buf_len, &ssmsg);  //modify by yanly
		ssmsg_handle(&ssmsg,topicname);
	}
}
