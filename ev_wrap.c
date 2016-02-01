/*
 *	File name   : ev_wrap.c
 *  Created on  : Jan 11, 2016
 *  Author      : yanly
 *  Description : libev事件处理
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#include <stddef.h>
#include <string.h>
#include "common.h"
#include "ev_wrap.h"
#include "log_printf.h"
#include "ev_socket.h"
#include "unistd.h"
#include "linked_list_queue.h"
#include "gatewayHwControl.h"
#include "gateway_init.h"

//global
gateway_info_st gateway_info;
struct ev_loop *global_loop;
EV_ALL	global_ev_all;

int mqtt_client_initw(EV_ALL *cev);
int http_client_initw(http_cev *cev);
static void mqtt_ping_cb(EV_P_ ev_timer *w, int revents);
static void mqtt_read_cb(EV_P_ ev_io *w, int revents);
static void mqtt_write_cb(EV_P_ ev_io *w, int revents);
static void zg_ping_cb(EV_P_ ev_timer *w, int revents);
int zigbee_client_initw(EV_ALL *cev);
/*-----------------------------------------------------------------------------------------------*/
//ev callback handle
//mqtt callback
static void mqtt_read_cb(EV_P_ ev_io *w, int revents)
{
	int rc;
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, mt_readw));
//	EV_ALL* cev = (EV_ALL*) w;

	rc = MQTTYield(&cev->client, 1000);
	if (rc != SUCCESS)
	{
		//TODO：错误处理
	}
}
static void mqtt_connect_cb(EV_P_ ev_io *w, int revents)
{
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, mt_connectw));

	ev_io_stop(cev->mainloop, &cev->mt_connectw);
	ev_io_start(cev->mainloop, &cev->mt_readw);

	if(cev->client.keepAliveInterval >0)
	{
		ev_timer_init(&cev->mt_pingw, mqtt_ping_cb, 0, PING_INTERVAL);//发ping包时间要小于keepAliveInterval
		log_printf(LOG_NOTICE, "init ping timer,timer interval is %d\n", PING_INTERVAL);
	}
	//重连次数重置
	cev->client.reconnect_count =0;

	//发connect包
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = WILL_FLAG;
	data.MQTTVersion = MQTT_VERSION;
	char client_id[DID_LEN];
	char username[MAC_LEN];
	char password[PASSCODE_LEN];
	snprintf(client_id, sizeof(client_id), "%s", gateway_info.did);
	snprintf(username, sizeof(username), "%s", gateway_info.mac);
	snprintf(password, sizeof(password), "%s", gateway_info.passcode);
	data.clientID.cstring = client_id;
	data.username.cstring = username;
	data.password.cstring = password;
	data.keepAliveInterval = KEEPALIVE_INTERVAL;
	data.cleansession = CLEANSESSIN;


	log_printf(LOG_NOTICE, "[MqttSocketConnected]: %s, %d\n", cev->client.mqtt_server, cev->client.mqtt_port);

	MQTTConnect(&cev->client, &data);
	log_printf(LOG_NOTICE, "willFlag: %d, clientID: %s, username: %s, password: %s, keepAliveInterval: %d, cleansession: %d\n",
			data.willFlag, data.clientID.cstring, data.username.cstring, data.password.cstring, data.keepAliveInterval, data.cleansession);

}
static void mqtt_write_cb(EV_P_ ev_io *w, int revents)
{
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, mt_writew));
	int rc = FAILURE,
        sent = 0;

	unsigned char *buf_ptr_item;
	send_queue_item_st *item;

	item = llqueue_poll(cev->client.send_queue);
	buf_ptr_item = item->sendbuf;
	int length = item->buflen;

    while (sent < length)
    {
        rc = linux_write_ev(cev->client.ipstack, &buf_ptr_item[sent], length);
        if (rc < 0)  // there was an error writing the data
            break;
        sent += rc;
    }
    if (sent == length)
    {
    	//record the fact that we have successfully sent the packet
    	if(cev->client.keepAliveInterval >0) {
//    		cev->mt_pingw.repeat = cev->client.keepAliveInterval-1; //可以重新设超时时间
    		ev_timer_again(cev->mainloop, &cev->mt_pingw); //重新计时发ping包
    		log_printf(LOG_NOTICE, "reset mqtt ping timer,interval is %d\n", PING_INTERVAL);
    	}
    	log_printf(LOG_NOTICE, "[Send2Mqtt]: %s, %d\n", buf_ptr_item, length);
        rc = SUCCESS;
    }
    else
    {
    	log_printf(LOG_ERROR, "mqtt_write_cb failed\n");
        rc = FAILURE;
    }

    //队列数据取出发送之后，需要释放
	free(buf_ptr_item); //must free
	free(item); //must free
	int q_count = llqueue_count(cev->client.send_queue);
	log_printf(LOG_NOTICE, "[MqttSendQueueCount]=%d\n", q_count);
	if(q_count <=0)
	{
		log_printf(LOG_NOTICE, "[StopMqttSendWatcher]send queue empty!\n");
		ev_io_stop(cev->mainloop, &cev->mt_writew);
	}
	//

    if(rc != SUCCESS)
    {
    	log_printf(LOG_ERROR, "mqtt_write_cb failed,[ReConnectMqtt]\n");
		MQTTReConnect(&cev->client);
    }
}
static void mqtt_ping_cb(EV_P_ ev_timer *w, int revents)
{
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, mt_pingw));
	keepalive(&cev->client);
}
//http callback
static void hp_connect_cb(EV_P_ ev_io *w, int revents)
{
	http_cev *hcv = (http_cev *)(((char *)w) - offsetof (http_cev, hp_connectw));
	log_printf(LOG_NOTICE, "[HTTPConnected]: %s %d\n", hcv->http_client.httpserver, hcv->http_client.httpport);
	ev_io_stop(hcv->cev->mainloop, &hcv->hp_connectw);
	ev_io_start(hcv->cev->mainloop, &hcv->hp_readw);
	ev_io_start(hcv->cev->mainloop, &hcv->hp_writew);
}
static void hp_read_cb(EV_P_ ev_io *w, int revents)
{
	//TODO: 后续HTTP协议解析放到这里做
	http_cev *hcv = (http_cev *)(((char *)w) - offsetof (http_cev, hp_readw));
	int recvlen;
	char tempbuf[HTTP_BUF_MAXLEN];

	recvlen = ev_read(hcv->http_client.my_socket, tempbuf, HTTP_BUF_MAXLEN);

	if(recvlen)
	{

		char * end = strrchr(tempbuf, '}');
		if(end)
		{
			ev_io_stop(hcv->cev->mainloop, &hcv->hp_readw);
			http_socket_close(&hcv->http_client); //http数据接收完成就可以关闭该连接了 	//关闭http socket后才可以进行新的http请求
			strncat(hcv->http_client.readbuf, tempbuf, recvlen);
			hcv->http_client.recv_len = hcv->http_client.recv_len+recvlen;
		}
		else
		{
			strncat(hcv->http_client.readbuf, tempbuf, recvlen);
			hcv->http_client.recv_len = recvlen;
			return ;
		}
		log_printf(LOG_NOTICE, "[RecvHttp]: %s,%d\n", hcv->http_client.readbuf, hcv->http_client.recv_len);
		int handlerc = hcv->http_client.fp((void *)hcv, hcv->http_client.readbuf, hcv->http_client.recv_len, hcv->http_client.callback_para);
		free(hcv); //must free
		if(handlerc <0)
		{
			log_printf(LOG_ERROR, "http recv parse failed\n");
		}
	}
	else
	{
		//TODO: ERROR HANDLE
		log_printf(LOG_ERROR, "hp_read_cb recv failed\n");
	}
}
static void hp_write_cb(EV_P_ ev_io *w, int revents)
{
	int rc=-1;
	http_cev *hcv = (http_cev *)(((char *)w) - offsetof (http_cev, hp_writew));

	rc = ev_write(hcv->http_client.my_socket, hcv->http_client.sendbuf, hcv->http_client.send_len);
	ev_io_stop(hcv->cev->mainloop, &hcv->hp_writew);

	if (rc <0)
	{
		//TODO: ERROR HANDLE
	}
	else
		log_printf(LOG_NOTICE, "[SEND2HTTP]: %s\n", hcv->http_client.sendbuf);
	//开始计时，规定时间内没返回当错误处理
	//TODO
}
//static void hp_timeout_cb(EV_P_ ev_timer *w, int revents)
//{
//	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, mt_pingw));
//}
//zigbee callback
static void zg_connect_cb(EV_P_ ev_io *w, int revents)
{
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, zg_connectw));

	log_printf(LOG_NOTICE, "[ZigbeeConneted]\n");
	ev_io_stop(cev->mainloop, &cev->zg_connectw);
	ev_io_start(cev->mainloop, &cev->zg_readw);

	cev->zigbee_client.isconnected = socket_connected;
	cev->zigbee_client.reconnect_count = 0; //重置重连次数

	if(cev->zigbee_client.keepAliveInterval >0)
	{
		ev_timer_init(&cev->zg_pingw, zg_ping_cb, 0, cev->zigbee_client.keepAliveInterval);
		ev_timer_again(cev->mainloop, &cev->zg_pingw);
	}
//	dev_init();  //todo :
}
static void zg_ping_cb(EV_P_ ev_timer *w, int revents)
{
	char *temp = "pingtest";
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, zg_pingw));

	ev_send_msg_2_zigbees(cev, (unsigned char *)temp, strlen(temp)+1);
	log_printf(LOG_NOTICE, "[SendPing2ZW] %d\n", cev->zigbee_client.keepAliveInterval);

	if(cev->zigbee_client.keepAliveInterval >0)
		ev_timer_again(cev->mainloop, &cev->zg_pingw); //重新计时发ping包
}
static void zg_read_cb(EV_P_ ev_io *w, int revents)
{
//	printf("zg_read_cb start\n");
//	int rc;
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, zg_readw));

	int recvlen;
	unsigned char tempbuf[ZIGBEE_BUF_MAXLEN];

	recvlen = ev_read(cev->zigbee_client.my_socket, tempbuf, ZIGBEE_BUF_MAXLEN);

	if(recvlen>0)
	{
//		memset(cev->zigbee_client.readbuf, 0, sizeof(cev->zigbee_client.readbuf));
		snprintf((char *)cev->zigbee_client.readbuf, ZIGBEE_BUF_MAXLEN, "%s", (char *)tempbuf);
		zigbee_msg_handle((void *)cev, tempbuf, recvlen);
	}
	else
	{
		//TODO: ERROR HANDLE
		log_printf(LOG_ERROR, "zg_read_cb failed\n");
		zigbee_reconnect();
	}
//	printf("zg_read_cb over\n");
}
static void zg_write_cb(EV_P_ ev_io *w, int revents)  //send by queue
{
	int rc=-1;
	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, zg_writew));

	unsigned char *buf_ptr_item;
	send_queue_item_st *item;

	item = llqueue_poll(cev->zigbee_client.send_queue);
	buf_ptr_item = item->sendbuf;
	int sendlen = item->buflen;
	rc = ev_write(cev->zigbee_client.my_socket, buf_ptr_item, sendlen);

	if (rc <0)
	{
		log_printf(LOG_ERROR, "zg_write_cb,send msg to zigbee middleware failed\n");
		//TODO: ERROR HANDLE
	}
	else
	{
		//send success
		log_printf(LOG_NOTICE, "[Send2ZW]: %s, %d\n", buf_ptr_item, sendlen);
	}
	free(buf_ptr_item); //must free
	free(item); //must free
	int q_count = llqueue_count(cev->zigbee_client.send_queue);
	log_printf(LOG_NOTICE, "[ZigbeeSendQueueCount]=%d\n", q_count);
	if(q_count <=0)
	{
		log_printf(LOG_NOTICE, "[StopZigbeeSendWatcher]send queue empty!\n");
		ev_io_stop(cev->mainloop, &cev->zg_writew);
	}
}
/*
 * description: send msg to zigbee middleware
 * sendbuf: the msg , need contain '\0' in the tail
 * sendbuflen: msg length
 * */
void ev_send_msg_2_zigbees(EV_ALL* cev, const unsigned char *sendbuf, int sendbuflen) //add send queue
{
	if(cev->zigbee_client.isconnected ==socket_notconnect)
	{
		log_printf(LOG_ERROR, "[ZigbeeClientIsClosed]\n");
		return;
	}

	send_queue_item_st *item;
	item = malloc(sizeof(send_queue_item_st));//important, must be free , 队列发送完成后需要释放
	unsigned char *buf_ptr_item = malloc(sendbuflen); //important, must be free , 队列发送完成后需要释放
	//snprintf((char *)buf_ptr_item, sendbuflen, "%s", (char *)sendbuf);
//	char* buf_ptr_item = (char *)sendbuf;
	memcpy(buf_ptr_item, sendbuf,sendbuflen );
	item->sendbuf = buf_ptr_item;
	item->buflen = sendbuflen;

	llqueue_offer(cev->zigbee_client.send_queue, item);  //入队列

	int q_count = llqueue_count(cev->zigbee_client.send_queue);

	log_printf(LOG_NOTICE, "[ZigbeeSendQueueCount]=%d\n", q_count);

	if(llqueue_count(cev->zigbee_client.send_queue)==1)
	{
//		ev_io_set(&cev->zg_writew, cev->zigbee_client.my_socket, EV_WRITE);
		ev_io_start(cev->mainloop, &cev->zg_writew);
	}
}
void ev_send_msg_2_mqtts(EV_ALL* cev, const unsigned char *sendbuf, int sendbuflen) //new queue send
{
	send_queue_item_st *item;

	item = malloc(sizeof(send_queue_item_st));//important, must be free , 队列发送完成后需要释放
	unsigned char *buf_ptr_item = malloc(sendbuflen); //important, must be free , 队列发送完成后需要释放
	memcpy(buf_ptr_item, sendbuf, sendbuflen);

	item->sendbuf = buf_ptr_item;
	item->buflen = sendbuflen;

	llqueue_offer(cev->client.send_queue, item);  //入队列

	int q_count = llqueue_count(cev->client.send_queue);

	log_printf(LOG_NOTICE, "[MqttSendQueueCount]=%d\n", q_count);

	if(llqueue_count(cev->client.send_queue)==1)
	{
		ev_io_start(cev->mainloop, &cev->mt_writew);
	}
}
//void ev_send_msg_2_mqtts(EV_ALL* cev, const char *sendbuf, int sendbuflen) //old
//{
//	memcpy(cev->client.buf, sendbuf, sendbuflen);
//    cev->client.send_len = sendbuflen;
//
//	ev_io_stop(cev->mainloop, &cev->mt_writew);
//	ev_io_set(&cev->mt_writew, cev->client.ipstack->my_socket, EV_WRITE);
//	ev_io_start(cev->mainloop, &cev->mt_writew);
//}
//sendbuf必须包含截止符
void mqtt_client_send_msg_2_zigbees(Client *c, const unsigned char *sendbuf, int sendbuflen)
{
	EV_ALL *cev = container_in(c,EV_ALL, client);
	ev_send_msg_2_zigbees(cev, sendbuf, sendbuflen);
}
/*
 * description: mqtt publish
 * */
//void publish_msg_2_mqtts(void *cev, const char *topicName, MQTTMessage* message)
//{
//	int rc;
//
//	rc = MQTTPublish(&global_ev_all.client, topicName, message);
//	if(rc!=0)
//		log_printf(LOG_NOTICE, "publish_msg_2_mqtts failed\n");
//	else
//		log_printf(LOG_NOTICE, "[PUBLISH]: %s, %s\n", topicName, (char *)message->payload);
//}

/*
 * description: publish qos0 msg to mqtt
 *   topicName--主题名
 *   cmd--命令字，msgytype
 *   header_type-- enum value, json_header or protobuf_header or binary_header
 *   buf--buffer
 *   buflen--buffer length
 * */
void publish_msg_2_mqtts_qos0(const char *topicName, enum mqtt_buf_header header_type, int cmd, int buflen, const void *buf)
{
	int rc;

	//组包头
	char playload[MQTT_PLAYLOADLEN] = {0};
	int totallen = buflen+2;
	int payloadlen = buflen+10;
	memcpy(playload, &header_type, 4);
	memcpy(playload+4, &totallen, 4);
	memcpy(playload+8, &cmd, 2);
	memcpy(playload+10, buf, buflen);

	//构造mqtt publish数据
	MQTTMessage msg;
	msg.dup=0;  //必须初始化。
	msg.id=0;
	msg.qos=0;
	msg.retained=0;
	msg.payloadlen = payloadlen;
	msg.payload = playload;

	rc = MQTTPublish(&global_ev_all.client, topicName, &msg);
	if(rc!=0)
		log_printf(LOG_NOTICE, "publish_msg_2_mqtts failed\n");
	else
	{
		log_printf(LOG_NOTICE, "[PUBLISH]: %s, %s\n", topicName, (char *)msg.payload);
		int i;
		for(i=0; i<buflen+10; i++)
		{
			printf("0x%x ", playload[i]);
		}
		printf("\n");
	}

}
//static void zg_ping_cb(EV_P_ ev_timer *w, int revents)
//{
//	EV_ALL *cev = (EV_ALL *)(((char *)w) - offsetof (EV_ALL, zg_pingw));
//}
/*-----------------------------------------------------------------------------------------------*/
//start write cb
void start_mqtt_write_cb(Client* c)
{
	EV_ALL *cev = container_in(c,EV_ALL, client);

	ev_io_stop(cev->mainloop, &cev->mt_writew);
	ev_io_set(&cev->mt_writew, cev->client.ipstack->my_socket, EV_WRITE);
	ev_io_start(cev->mainloop, &cev->mt_writew);
}

/*-----------------------------------------------------------------------------------------------*/
//ev init
void mqtt_client_init(EV_ALL *cev, char *host, int port)
{
	int rc;
	if(cev->client.send_queue)
		llqueue_free(cev->client.send_queue);
	cev->client.send_queue = llqueue_new();
	rc = ConnectNetwork(cev->client.ipstack, host, port);
//	if(rc <0)
//	{
//		printf("ConnectNetwork failed\n");
//		MQTTReConnect(&cev->client);
//	}
	snprintf(cev->client.mqtt_server, SERVER_MAX_LEN-1, "%s", host);
	cev->client.mqtt_port = port;
	mqtt_client_initw(cev);  //watchers init
}
/*
 * 判读是否需要请求获取did和mqtt info
 * */
void mqtt_init()
{
	system(SET_DARK_CLOUD);//add by yanly
	char tembuf[HTTP_BUF_MAXLEN];
	int sendlen;
	if(gateway_info.did[0] ==0) //request did
	{
		sendlen = http_req_did(tembuf, HTTP_BUF_MAXLEN, gateway_info.http_server, DEV_REGISTER_URL,
						 gateway_info.passcode, gateway_info.sn, gateway_info.productkey);
		if(sendlen)
		{
			http_req_start(tembuf, sendlen, gateway_info.http_server, gateway_info.http_port,
					NULL, http_res_did);
		}
		else
			log_printf(LOG_ERROR, "http_req_did error\n");
	}
	else
	{
		if((gateway_info.mt_server1[0] !=0)&&(gateway_info.mt_port1)!=0)
		{
			mqtt_client_init(&global_ev_all, gateway_info.mt_server1, gateway_info.mt_port1);
		}
		else if((gateway_info.mt_server2[0] !=0)&&(gateway_info.mt_port2)!=0)
		{
			mqtt_client_init(&global_ev_all, gateway_info.mt_server2, gateway_info.mt_port2);
		}
		else
		{
			 //request mqtt info
			sendlen = http_req_mtinfo(tembuf, HTTP_BUF_MAXLEN, gateway_info.http_server, GET_MQTT_INFO_URL,
							 gateway_info.did);
			if(sendlen)
			{
				http_req_start(tembuf, sendlen, gateway_info.http_server, gateway_info.http_port,
						NULL, http_res_mtinfo);
			}
			else
				log_printf(LOG_ERROR, "http_req_did error\n");
		}
	}
}
void zigbee_client_init(EV_ALL *cev, char *zw_server, int zw_port)
{
	int rc;
	rc = ev_connect(&cev->zigbee_client.my_socket, zw_server, zw_port);
	zigbee_client_initw(cev);
}
void zigbee_init()
{
	global_ev_all.zigbee_client.send_queue = llqueue_new();
	global_ev_all.zigbee_client.keepAliveInterval = ZIGBEE_KEEPALIVE;
	zigbee_client_init(&global_ev_all, ZIGBEE_SERVER, ZIGBEE_PORT);
}

void http_req_start(const char *sendbuf,int sendbuf_len, const char *host, int port,  void *callback_para, http_callback_hander http_hander)
{
	int rc;
	http_cev *hcv;

	hcv = malloc(sizeof(http_cev));
	hcv->cev = &global_ev_all;
	hcv->http_client.httpport= port;
	sprintf(hcv->http_client.httpserver, "%s", host);
	sprintf(hcv->http_client.sendbuf, "%s", sendbuf);
	hcv->http_client.send_len = sendbuf_len;
	hcv->http_client.fp = http_hander;

	if(callback_para !=NULL)
		hcv->http_client.callback_para = callback_para;

	rc = ev_connect(&hcv->http_client.my_socket, hcv->http_client.httpserver, hcv->http_client.httpport);

	http_client_initw(hcv);
}
//watchers init
int mqtt_client_initw(EV_ALL *cev)
{
	ev_io_init(&cev->mt_connectw, mqtt_connect_cb, cev->client.ipstack->my_socket, EV_WRITE);
	ev_io_init(&cev->mt_readw, mqtt_read_cb, cev->client.ipstack->my_socket, EV_READ);
	ev_io_init(&cev->mt_writew, mqtt_write_cb, cev->client.ipstack->my_socket, EV_WRITE);

	ev_io_start(cev->mainloop, &cev->mt_connectw);
	return 0;
}
int http_client_initw(http_cev *hcv)
{
	ev_io_init(&hcv->hp_connectw, hp_connect_cb, hcv->http_client.my_socket, EV_WRITE);
	ev_io_init(&hcv->hp_readw, hp_read_cb, hcv->http_client.my_socket, EV_READ);
	ev_io_init(&hcv->hp_writew, hp_write_cb, hcv->http_client.my_socket, EV_WRITE);

	ev_io_start(hcv->cev->mainloop, &hcv->hp_connectw);
	return 0;
}
int zigbee_client_initw(EV_ALL *cev)
{
	ev_io_init(&cev->zg_connectw, zg_connect_cb, cev->zigbee_client.my_socket, EV_WRITE);
	ev_io_init(&cev->zg_readw, zg_read_cb, cev->zigbee_client.my_socket, EV_READ);
	ev_io_init(&cev->zg_writew, zg_write_cb, cev->zigbee_client.my_socket, EV_WRITE);

	ev_io_start(cev->mainloop, &cev->zg_connectw);
	return 0;
}
//watchers stop
void stop_watchers(const char *para)
{
	if(memcmp(para, "mqtt", strlen(para))==0)
	{
		ev_io_stop(global_ev_all.mainloop, &global_ev_all.mt_writew);
		ev_io_stop(global_ev_all.mainloop, &global_ev_all.mt_readw);
		ev_timer_stop(global_ev_all.mainloop, &global_ev_all.mt_pingw);
//		ev_io_stop(global_ev_all.mainloop, &global_ev_all.mt_connectw);
	}
	else if(memcmp(para, "zigbee", strlen(para))==0)
	{
		ev_io_stop(global_ev_all.mainloop, &global_ev_all.zg_writew);
		ev_io_stop(global_ev_all.mainloop, &global_ev_all.zg_readw);
		ev_timer_stop(global_ev_all.mainloop, &global_ev_all.zg_pingw);
//		ev_io_stop(global_ev_all.mainloop, &global_ev_all.zg_connectw);
	}
	else
	{

	}
}
