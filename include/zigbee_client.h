/*
 *	File name   : zigbee_client.h
 *  Created on  : Jan 14, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#include <stdint.h>


#ifndef INCLUDE_ZIGBEE_CLIENT_H_
#define INCLUDE_ZIGBEE_CLIENT_H_


#define ZIGBEE_BUF_MAXLEN		2048
#define ZIGBEE_KEEPALIVE		60 //unit:s

typedef struct {

//    int send_len; //actual send length
//    char send_type; //in order to distinguish what type was send
//    char sendbuf[ZIGBEE_BUF_MAXLEN]; //will be send buffer
	unsigned readbuf[ZIGBEE_BUF_MAXLEN];
    unsigned int keepAliveInterval;  //心跳包间隔
    char ping_outstanding;	//心跳报回复标记

    int isconnected;
    int my_socket;
    int reconnect_count;

    //增加发送队列
    void *send_queue;

}zigbee_client_st;

void zigbee_msg_handle(void *cev, const unsigned char *buf, int buflen);
void zigbee_reconnect();
void HexToStr(uint8_t *pbDest,uint8_t *pbSrc, int nLen);
void StrToHex(uint8_t *pbDest, uint8_t *pbSrc, int nLen);
void sensor_type(char *type);
void access_netword();
int del_dev(char *nwk_addr_data,char *ieee_data,char *ep);
void stop_alarm();
//void test();
#endif /* INCLUDE_ZIGBEE_CLIENT_H_ */
