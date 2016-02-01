/*
 *	File name   : common.h
 *  Created on  : Jan 11, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef COMMON_H_
#define COMMON_H_

//开关宏
#define EV_WRAP
#define USE_IN_ARM
//#define USE_IN_PC


//files path
#define GATEWAY_INFO_PATH				"/gl/etc/gateway_setting.json"
#define GATEWAY_MSG_PATH				"/gl/etc/gateway_msg.proto"
#define GATEWAY_SN						"/gl/etc/sn"
//protobuf releated macro
#define DEVINFO_DIR						"/gl/etc/devlist/"

#define GATEWAY_INFO_FILE_SIZE			1024+1

//网卡名
#ifndef USE_IN_PC
#define IF_NAME			 "eth0"
#else
#define IF_NAME			 "eth0"
#endif
//socket 连接标记
enum socket_connect_flag {socket_notconnect =0, socket_connected = 1};


/////////////////////////////////////////////////length macro////////////////////////////////
//+1 包括截止符
#define	PASSCODE_LEN		8+1
#define MAC_LEN				12+1
#define DID_LEN				100+1
#define PRODUCTKEY_LEN		100+1
#define SN_NUM_LEN			16+1
#define PRODUCT_KEY			"abcdefghijklmnopqrstuvwxyz"
#define NAME_LEN			30+1
///////////////////////////////////////////////////////////////////////////////////////////

//gateway
#define GATEWAY_RESERVED    ""
#define MANUFACTURE			"GLEXER"
#define	MODEL				"HGZB02A"
#define	SW_VER				"1.0.0"
#define HW_VER				"0.1.0"
#define GATEWAY_NWKADDR		"GATE"
#define GATEWAY_NODE_NAME	GATEWAY_RESERVED
#define GATEWAY_ONLINE		"1"
#define GATEWAY_NODE_STATUS	GATEWAY_RESERVED
#define	GATEWAY_DEVICE_ID	"GATE"
#define	GATEWAY_PROFILE_ID	"GATE"
#define	GATEWAY_ZONETYPE	"GATE"
#define	GATEWAY_EP			"0A"
#define	GATEWAY_EP_NAME		"智能网关"
#define	GATEWAY_RID			GATEWAY_RESERVED    //todo: 下一版本放在/gl/etc/gateway_setting.json
#define	GATEWAY_ARM			"1"	  //下一版本放在/gl/etc/gateway_setting.json



//zigbee
#define	ZIGBEE_PORT			8001
#ifndef USE_IN_PC

#define ZIGBEE_SERVER		"127.0.0.1"
#else

#define ZIGBEE_SERVER		"192.168.1.29"
#endif

//HTTP
#ifndef USE_IN_PC
#define HTTP_SERVER_HOST	"192.168.1.177"
#define HTTP_SERVER_PORT	8080
#else
#define HTTP_SERVER_HOST	"192.168.1.177"
#define HTTP_SERVER_PORT	8080
#endif
#define DEV_REGISTER_URL	"/GCloudAPI/api/device/register/"
#define GET_MQTT_INFO_URL	"/GCloudAPI/api/metadata/getvalue/mqttserver/"

//MQTT连接参数
enum mqtt_buf_header { json_header =1, protobuf_header, binary_header };

#define SERVER_HOST			"iot.eclipse.org"
#define SERVER_PORT			1883
#define SERVER_MAX_LEN		100+1
#define WILL_FLAG			0
#define MQTT_VERSION		3
//#define CLIENT_ID			"did"
//#define USERNAME			"mac"
//#define PASSWORD			"passcode"
#define KEEPALIVE_INTERVAL	120   //Unit:s  //modify by yanly
#define PING_INTERVAL		KEEPALIVE_INTERVAL-10    //-10
#define CLEANSESSIN			1
#define	MQTT_BUFFER_LEN		4096
#define MQTT_PLAYLOADLEN	MQTT_BUFFER_LEN +10
#define MQTT_RECV_LEN
#define MQTT_SEND_LEN
#define TOPIC_MAX_LEN		150+1
#define RECONNECT_MAX_COUNT	30      //重连最大次数，超过重启程序

#define SUBTOPIC_PRE1				"ser2cli_res/"
#define SUBTOPIC_PRE2				"ser2cli_noti/"
#define SUBTOPIC_PRE3				"app2dev/"
#define PUBTOPIC_PRE1				"dev2app/"
#define PUBTOPIC_PRE2				"dev2ser/"
#define PUBTOPIC_PRE3				"ser2cli_req/"

//根据结构体成员变量获取结构体指针
#define container_in(ptr,TYPE,member) \
    (TYPE*)((unsigned char*)(ptr) - (size_t)&(((TYPE*)0)->member))


typedef struct {
	char http_server[SERVER_MAX_LEN];
	int http_port;
	char mt_server1[SERVER_MAX_LEN];
	int mt_port1;
	char mt_server2[SERVER_MAX_LEN];
	int mt_port2;
	char passcode[PASSCODE_LEN];
	char mac[MAC_LEN];
	char did[DID_LEN];
	char productkey[PRODUCTKEY_LEN];
	char sn[SN_NUM_LEN];
	int duration;  //add by yanly0126
} gateway_info_st;  //结构体后缀_st

typedef struct {
	char host[100];
	int port;
}server_st;

//发送队列item结构体
typedef struct{
	void *sendbuf;
	int buflen;
}send_queue_item_st;

extern gateway_info_st gateway_info;
extern struct ev_loop *global_loop;
char subtopics[3][TOPIC_MAX_LEN];
char pubtopics[3][TOPIC_MAX_LEN];

#endif /* COMMON_H_ */
