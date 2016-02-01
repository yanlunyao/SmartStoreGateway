/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#ifndef __MQTT_CLIENT_C_
#define __MQTT_CLIENT_C_

#include "MQTTPacket.h"
#include "stdio.h"
#include "MQTTLinux.h" //Platform specific implementation header file
#include "common.h"

#define MAX_PACKET_ID 65535
#define MAX_MESSAGE_HANDLERS 5

enum QoS { QOS0, QOS1, QOS2 };

// all failure return codes must be negative
enum returnCode { SOCKET_CLOSED=-3, BUFFER_OVERFLOW = -2, FAILURE = -1, SUCCESS = 0 };
enum subscribeOrder {suscribe_none=0, ser2cli_res_order=0, ser2cli_noti_order, app2dev_order, subscribe_over};

//by yanly
//send type
//enum sendingTypeCode {NORMAL_SENDING = 0, PING_SENDING = 1};

void NewTimer(Timer*);

typedef struct MQTTMessage MQTTMessage;

typedef struct MessageData MessageData;

struct MQTTMessage
{
    enum QoS qos;
    char retained;
    char dup;
    unsigned short id;
    void *payload;
    size_t payloadlen;
};

struct MessageData
{
    MQTTMessage* message;
    MQTTString* topicName;
};
typedef struct Client Client;

typedef void (*messageHandler)(MessageData*, Client*);

int MQTTConnect (Client*, MQTTPacket_connectData*);
int MQTTPublish (Client*, const char*, MQTTMessage*);
int MQTTSubscribe (Client*, const char*, enum QoS, messageHandler);
int MQTTUnsubscribe (Client*, const char*);
int MQTTDisconnect (Client*);
int MQTTYield (Client*, int);

void setDefaultMessageHandler(Client*, messageHandler);

void MQTTClient(Client*, Network*, unsigned int, unsigned char*, size_t, unsigned char*, size_t);

struct Client {
    unsigned int next_packetid;
    unsigned int command_timeout_ms;
    size_t buf_size, readbuf_size;
    unsigned char *buf;
    unsigned char *readbuf;
    unsigned int keepAliveInterval;
    char ping_outstanding;
    int isconnected;

/////////////////////////////////        add by yanly
    size_t send_len; //add by yanly, mqtt actual send length
//    char send_type; //add by yanly, send type in order to distinguish is a ping or a normal send
    int suborder;  //add by yanly, subscribe topic order,需要完成所有订阅才能进行正常业务通信。mqtt通信重连后，该标记需重置！
//    char mqtt_server_index;
    char mqtt_server[SERVER_MAX_LEN];
    int mqtt_port;
    void *send_queue;   //发送队列   //important: 不用需free
    int reconnect_count;
////////////////////////////////

    struct MessageHandlers
    {
        const char* topicFilter;
        void (*fp) (MessageData*, Client *);
    } messageHandlers[MAX_MESSAGE_HANDLERS];      // Message handlers are indexed by subscription topic
    
    void (*defaultMessageHandler) (MessageData*);
    
    Network* ipstack;
    Timer ping_timer;
};

#define DefaultClient {0, 0, 0, 0, NULL, NULL, 0, 0, 0}


//add by yanly
int sendPacket_ev(Client* c, int length);
int keepalive(Client* c);
int MQTTReConnect(Client *c);

#endif
