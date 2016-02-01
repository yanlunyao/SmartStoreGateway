/*
 *	File name   : ev_wrap.h
 *  Created on  : Jan 11, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef EV_WRAP_H_
#define EV_WRAP_H_

#include "MQTTClient.h"
#include "ev.h"
#include "http_client.h"
#include "zigbee_client.h"


typedef struct EV_WRAP_EVERYTHING EV_ALL;
typedef struct http_cev http_cev;
struct EV_WRAP_EVERYTHING{

	ev_io mt_connectw;
    ev_io mt_readw; 		//mqtt read watcher
    ev_io mt_writew;
    ev_timer mt_pingw;
    Client client;  		//mqtt client struct

//    ev_io hp_connectw;
//    ev_io hp_readw;
//    ev_io hp_writew;
//    ev_timer hp_timeoutw;
//    httpclient_st http_client;

	ev_io zg_connectw;
    ev_io zg_readw;
    ev_io zg_writew;
    ev_timer zg_pingw;
    zigbee_client_st zigbee_client;

    struct ev_loop *mainloop;	//default ev_loop, need init
};
struct http_cev{
    ev_io hp_connectw;
    ev_io hp_readw;
    ev_io hp_writew;
    ev_timer hp_timeoutw;
    httpclient_st http_client;
    struct ev_loop *mainloop;
    EV_ALL *cev;
};

int mqtt_client_ev_init(EV_ALL *cev);
void start_mqtt_write_cb(Client* c); //
//void mqtt_client_init(EV_ALL *cev, Network *n,unsigned char *buf,unsigned char *readbuf, char *host, int port);
void mqtt_client_init(EV_ALL *cev, char *host, int port);
void zigbee_init();
void mqtt_init();
void ev_send_msg_to_zigbee(EV_ALL* cev, const char *sendbuf, int sendbuflen);
void mqtt_client_send_msg_2_zigbees(Client *c, const unsigned char *sendbuf, int sendbuflen);
void ev_send_msg_2_zigbees(EV_ALL* cev, const unsigned char *sendbuf, int sendbuflen);
void ev_send_msg_2_mqtts(EV_ALL* cev, const unsigned char *sendbuf, int sendbuflen);
//void publish_msg_2_mqtts(void *cev, const char *topicName, MQTTMessage* message);
void publish_msg_2_mqtts_qos0(const char *topicName, enum mqtt_buf_header header_type, int cmd, int buflen, const void *buf);
//ev
int ev_init_mqtt(EV_ALL *e);
void stop_watchers(const char *para);

//zigbee
void zigbee_client_init(EV_ALL *cev, char *zw_server, int zw_port);

//http
//void http_req_start(EV_ALL *cev, char *httpserver, int http_port, const char *sendbuf, int buflen, httphander http_hander);
//void http_req_start(http_cev *hcv, void *callback_para, http_callback_hander http_hander);
void http_req_start(const char *sendbuf,int sendbuf_len, const char *host, int port,  void *callback_para, http_callback_hander http_hander);





//extern
extern EV_ALL	global_ev_all;
#endif /* MQTT_EV_H_ */
