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

#include "MQTTClient.h"
#include "ev_wrap.h"
#include "common.h"
#include "mt_topic_handle.h"
#include "log_printf.h"
#include "linked_list_queue.h"
#include "gatewayHwControl.h"

char subtopics[3][TOPIC_MAX_LEN] = {SUBTOPIC_PRE1, SUBTOPIC_PRE2, SUBTOPIC_PRE3};
char pubtopics[3][TOPIC_MAX_LEN] = {PUBTOPIC_PRE1, PUBTOPIC_PRE2, PUBTOPIC_PRE3};
//ser2cli_res/ <DID>: 接收服务器的消息响应
//ser2cli_noti/<ProductKey>: 接收服务器的消息推送
//app2dev/<DID>/#，接收远程控制指令。(DID指代设备的DID，#是通配符)

//dev2app/<DID>，设备向所有绑定的app推送数据。
//dev2ser/<DID>，设备向云端推送数据。
//ser2cli_req/<DID>


const messageHandler mh[]=
{
//	mh_messageArrived,
	ser2cli_res_process,
	ser2cli_noti_process,
	app2dev_process,
	mh_none
};

void NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessgage) {
    md->topicName = aTopicName;
    md->message = aMessgage;
}


int getNextPacketId(Client *c) {
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}

#ifdef EV_WRAP
//int sendPacket_ev(Client* c, int length) //old
//{
//	int rc;
//    c->send_len = length; //add by yanly
//    start_mqtt_write_cb(c);
//    rc = SUCCESS;
//    return rc;
//}
int sendPacket_ev(Client* c, int length)
{
	int rc;
    c->send_len = length;
    ev_send_msg_2_mqtts(&global_ev_all, c->buf, c->send_len);
    rc = SUCCESS;
    return rc;
}
int sendPacket(Client* c, int length, Timer* timer)
{
    int rc = FAILURE,
        sent = 0;
    c->send_len = length; //add by yanly
    while (sent < length && !expired(timer))
    {
        rc = c->ipstack->mqttwrite(c->ipstack, &c->buf[sent], length, left_ms(timer));
        if (rc < 0)  // there was an error writing the data
            break;
        sent += rc;
    }
    if (sent == length)
    {
        countdown(&c->ping_timer, c->keepAliveInterval); // record the fact that we have successfully sent the packet
        rc = SUCCESS;
    }
    else
        rc = FAILURE;
    c->send_len = 0; //add by yanly
    return rc;
}
#else
int sendPacket(Client* c, int length, Timer* timer)
{
    int rc = FAILURE, 
        sent = 0;
    
    while (sent < length && !expired(timer))
    {
        rc = c->ipstack->mqttwrite(c->ipstack, &c->buf[sent], length, left_ms(timer));
        if (rc < 0)  // there was an error writing the data
            break;
        sent += rc;
    }
    if (sent == length)
    {
        countdown(&c->ping_timer, c->keepAliveInterval); // record the fact that we have successfully sent the packet    
        rc = SUCCESS;
    }
    else
        rc = FAILURE;
    return rc;
}
#endif

#ifdef EV_WRAP
void MQTTClient(Client* c, Network* network, unsigned int _keepAliveInterval, unsigned char* buf, size_t buf_size, unsigned char* readbuf, size_t readbuf_size)
{
    int i;

    c->ipstack = network;
    
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = 0;
    c->keepAliveInterval = _keepAliveInterval;   //modify by yanly
    c->buf = buf;
    c->buf_size = buf_size;
    c->readbuf = readbuf;
    c->readbuf_size = readbuf_size;
    c->isconnected = 0;
    c->ping_outstanding = 0;
    c->defaultMessageHandler = NULL;
    InitTimer(&c->ping_timer);

/////////    add by yanly
    c->send_queue = NULL;
    c->send_len = 0;
    c->suborder = suscribe_none;
    c->reconnect_count = 0;
//  c->mqtt_server_index = 0;
//  c->send_type = NORMAL_SENDING;
////////
}
#else
void MQTTClient(Client* c, Network* network, unsigned int command_timeout_ms, unsigned char* buf, size_t buf_size, unsigned char* readbuf, size_t readbuf_size)
{
    int i;

    c->ipstack = network;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = 0;
    c->command_timeout_ms = command_timeout_ms;
    c->buf = buf;
    c->buf_size = buf_size;
    c->readbuf = readbuf;
    c->readbuf_size = readbuf_size;
    c->isconnected = 0;
    c->ping_outstanding = 0;
    c->defaultMessageHandler = NULL;
    InitTimer(&c->ping_timer);
}
#endif

int decodePacket(Client* c, int* value, int timeout)
{
    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }
        rc = c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
        if (rc != 1)
            goto exit;
        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);
exit:
    return len;
}

#ifdef EV_WRAP
int readPacket(Client* c, Timer* timer)
{
    int rc = FAILURE;
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if (c->ipstack->mqttread(c->ipstack, c->readbuf, 1, left_ms(timer)) != 1)
    {
    	rc = SOCKET_CLOSED;
        goto exit;
    }

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, left_ms(timer));

    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (rem_len > 0 && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, left_ms(timer)) != rem_len))
        goto exit;

    header.byte = c->readbuf[0];
    rc = header.bits.type;
exit:
    return rc;
}
#else
int readPacket(Client* c, Timer* timer) 
{
    int rc = FAILURE;
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if (c->ipstack->mqttread(c->ipstack, c->readbuf, 1, left_ms(timer)) != 1)
        goto exit;

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, left_ms(timer));
    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (rem_len > 0 && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, left_ms(timer)) != rem_len))
        goto exit;

    header.byte = c->readbuf[0];
    rc = header.bits.type;
exit:
    return rc;
}
#endif

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;
    
    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };
    
    return (curn == curn_end) && (*curf == '\0');
}

#ifdef EV_WRAP
int deliverMessage(Client* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp(&md, c);
                rc = SUCCESS;
            }
        }
    }

    if (rc == FAILURE && c->defaultMessageHandler != NULL)
    {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }

    return rc;
}
#else
int deliverMessage(Client* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp(&md);
                rc = SUCCESS;
            }
        }
    }
    
    if (rc == FAILURE && c->defaultMessageHandler != NULL) 
    {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }   
    
    return rc;
}
#endif
#ifdef EV_WRAP
int keepalive(Client* c)
{
    int rc = FAILURE;
	if (!c->ping_outstanding)
	{
//		c->send_type = PING_SENDING;
		int len = MQTTSerialize_pingreq(c->buf, c->buf_size);
		log_printf(LOG_NOTICE, "[MqttTimeReach]send ping\n");
		if (len > 0 && (rc = sendPacket_ev(c, len)) == SUCCESS) // send the ping packet
			c->ping_outstanding = 1;
	}
	else
	{
		//规定时间内，服务器没有回复ping包
		//TODO：error handle
		log_printf(LOG_ERROR, "timeout, mqtt server not ping respond\n [ReConnectMqtt]");
		MQTTReConnect(c);
	}
//
//    //for test
//    MQTTMessage msg;
//    log_printf(LOG_NOTICE, "ping send\n");
////    MQTTPublish(c, "ping", &msg);
    return rc;
}
#else
int keepalive(Client* c)
{
    int rc = FAILURE;

    if (c->keepAliveInterval == 0)
    {
        rc = SUCCESS;
        goto exit;
    }

    if (expired(&c->ping_timer))
    {
        if (!c->ping_outstanding)
        {
            Timer timer;
            InitTimer(&timer);
            countdown_ms(&timer, 1000);
            int len = MQTTSerialize_pingreq(c->buf, c->buf_size);
            if (len > 0 && (rc = sendPacket(c, len, &timer)) == SUCCESS) // send the ping packet
                c->ping_outstanding = 1;
        }
    }

exit:
    return rc;
}
#endif

#ifdef EV_WRAP
int cycle(Client* c, Timer* timer)
{
    // read the socket, see what work is due
    int read_status;
    unsigned short packet_type;
    read_status = readPacket(c, timer);
    packet_type = (unsigned short )read_status;

    if(read_status == SOCKET_CLOSED)
    {
    	MQTTReConnect(c);
    	goto exit;
    }
    int len = 0,
        rc = SUCCESS;
    switch (packet_type)
    {
        case CONNACK:
        {
			{
				unsigned char connack_rc = 255;
				char sessionPresent = 0;
				if (MQTTDeserialize_connack((unsigned char*)&sessionPresent, &connack_rc, c->readbuf, c->readbuf_size) == 1)
				{
					c->isconnected = 1;
					//开cloud灯
					system(SET_LIGHT_CLOUD);

					log_printf(LOG_NOTICE, "[MqttConnected]: recv connack\n");
					//subtopics and pubtopics init
					sprintf(&subtopics[0][0], "%s%s", SUBTOPIC_PRE1, gateway_info.did);
					sprintf(&subtopics[1][0], "%s%s", SUBTOPIC_PRE2, gateway_info.productkey);
					sprintf(&subtopics[2][0], "%s%s/#", SUBTOPIC_PRE3, gateway_info.did);

					sprintf(&pubtopics[0][0], "%s%s", PUBTOPIC_PRE1, gateway_info.did);
					sprintf(&pubtopics[1][0], "%s%s", PUBTOPIC_PRE2, gateway_info.did);
					sprintf(&pubtopics[2][0], "%s%s/#", PUBTOPIC_PRE3, gateway_info.did);

					rc = MQTTSubscribe(c, subtopics[c->suborder], 0, mh[c->suborder]); //for test
					log_printf(LOG_NOTICE, "Subscribing to %s\n", subtopics[c->suborder]);
					c->suborder++;
				}
			}
        }
        	break;
        case PUBACK:
        	break;
        case SUBACK:
        {
			int count = 0, grantedQoS = -1;
			unsigned short mypacketid;
			if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, c->readbuf, c->readbuf_size) == 1)
				rc = grantedQoS; // 0, 1, 2 or 0x80
			if (rc != 0x80)
			{
				if(c->suborder <subscribe_over)
				{
					rc = MQTTSubscribe(c, subtopics[c->suborder], 0, mh[c->suborder]);
					log_printf(LOG_NOTICE, "Subscribing to %s\n",subtopics[c->suborder]);
					c->suborder++;
				}

//				int i;
//				for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
//				{
//					if (c->messageHandlers[i].topicFilter == 0)
//					{
//						c->messageHandlers[i].topicFilter = mytopics[i];
//						c->messageHandlers[i].fp = mh[i];
//						rc = 0;
//						break;
//					}
//				}
			}
			else
			{
				log_printf(LOG_ERROR, "SUCACK FAILED\n");
				//TODO: error handle
			}
        }
        	break;
        case PUBLISH:
        {
            if(c->suborder != subscribe_over)
            {
            	log_printf(LOG_ERROR, "[REC_MSG] rece publish msg but subcribe not over\n");
            	//TODO: error
            }
            MQTTString topicName = MQTTString_initializer;
            MQTTMessage msg;
            if (MQTTDeserialize_publish((unsigned char*)&msg.dup, (int*)&msg.qos, (unsigned char*)&msg.retained, (unsigned short*)&msg.id, &topicName,
               (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                goto exit;
            deliverMessage(c, &topicName, &msg);
//            MQTTPublish(c, "applerespond", &msg); //add by yaly for test
            if (msg.qos != QOS0)
            {
                if (msg.qos == QOS1)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                else if (msg.qos == QOS2)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                if (len <= 0)
                    rc = FAILURE;
                   else
                       rc = sendPacket_ev(c, len);
                if (rc == FAILURE)
                    goto exit;
            }
        }
        	break;
        case PUBREC:
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
            else if ((len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREL, 0, mypacketid)) <= 0)
                rc = FAILURE;
            else if ((rc = sendPacket_ev(c, len)) != SUCCESS) // send the PUBREL packet
                rc = FAILURE;
            if (rc == FAILURE)
                goto exit;
            break;
        }
        break;
        case PUBCOMP:
            break;
        case PINGRESP:
        	log_printf(LOG_NOTICE, "[MqttPingResp]\n");
            c->ping_outstanding = 0;
            break;
    }
//    keepalive(c); //modify by yanly
exit:
    if (rc == SUCCESS)
        rc = packet_type;
    return rc;
}
#else
int cycle(Client* c, Timer* timer)
{
    // read the socket, see what work is due
    unsigned short packet_type = readPacket(c, timer);

    int len = 0,
        rc = SUCCESS;

    switch (packet_type)
    {
        case CONNACK:
        case PUBACK:
        case SUBACK:
            break;
        case PUBLISH:
        {
            MQTTString topicName;
            MQTTMessage msg;
            if (MQTTDeserialize_publish((unsigned char*)&msg.dup, (int*)&msg.qos, (unsigned char*)&msg.retained, (unsigned short*)&msg.id, &topicName,
               (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                goto exit;
            deliverMessage(c, &topicName, &msg);
            if (msg.qos != QOS0)
            {
                if (msg.qos == QOS1)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                else if (msg.qos == QOS2)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                if (len <= 0)
                    rc = FAILURE;
                   else
                       rc = sendPacket(c, len, timer);
                if (rc == FAILURE)
                    goto exit; // there was a problem
            }
            break;
        }
        case PUBREC:
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
            else if ((len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREL, 0, mypacketid)) <= 0)
                rc = FAILURE;
            else if ((rc = sendPacket(c, len, timer)) != SUCCESS) // send the PUBREL packet
                rc = FAILURE; // there was a problem
            if (rc == FAILURE)
                goto exit; // there was a problem
            break;
        }
        case PUBCOMP:
            break;
        case PINGRESP:
            c->ping_outstanding = 0;
            break;
    }
    keepalive(c);
exit:
    if (rc == SUCCESS)
        rc = packet_type;
    return rc;
}
#endif
#ifdef EV_WRAP
int MQTTYield(Client* c, int timeout_ms)
{
    int rc = SUCCESS;
    Timer timer;
//
//    InitTimer(&timer);
//    countdown_ms(&timer, timeout_ms);
//    while (!expired(&timer))
//    {
        if (cycle(c, &timer) == FAILURE)   //timer：这个参数没有意义，只是为了复用原来的接口才留下来
        {
            rc = FAILURE;
            log_printf(LOG_NOTICE, "MQTTYield failed\n");
//            break;
        }
//    }

    return rc;
}
#else
int MQTTYield(Client* c, int timeout_ms)
{
    int rc = SUCCESS;
    Timer timer;

    InitTimer(&timer);    
    countdown_ms(&timer, timeout_ms);
    while (!expired(&timer))
    {
        if (cycle(c, &timer) == FAILURE)
        {
            rc = FAILURE;
            break;
        }
    }
        
    return rc;
}
#endif

// only used in single-threaded mode where one command at a time is in process
int waitfor(Client* c, int packet_type, Timer* timer)
{
    int rc = FAILURE;
    
    do
    {
        if (expired(timer)) 
            break; // we timed out
    }
    while ((rc = cycle(c, timer)) != packet_type);  
    
    return rc;
}

#ifdef EV_WRAP
int MQTTConnect(Client* c, MQTTPacket_connectData* options)
{
//    Timer connect_timer;
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;
    
//    InitTimer(&connect_timer);
//    countdown_ms(&connect_timer, c->command_timeout_ms);

    if (c->isconnected) // don't send connect packet again if we are already connected
        goto exit;

    if (options == 0)
        options = &default_options; // set default options if none were supplied
    
    c->keepAliveInterval = options->keepAliveInterval;

//    countdown(&c->ping_timer, c->keepAliveInterval);

    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
        goto exit;
    if ((rc = sendPacket_ev(c, len)) != SUCCESS)  // send the connect packet
        goto exit; // there was a problem
    rc = SUCCESS;
exit:
    return rc;
}
#else
int MQTTConnect(Client* c, MQTTPacket_connectData* options)
{
    Timer connect_timer;
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;

    InitTimer(&connect_timer);
    countdown_ms(&connect_timer, c->command_timeout_ms);

    if (c->isconnected) // don't send connect packet again if we are already connected
        goto exit;

    if (options == 0)
        options = &default_options; // set default options if none were supplied

    c->keepAliveInterval = options->keepAliveInterval;
    countdown(&c->ping_timer, c->keepAliveInterval);
    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &connect_timer)) != SUCCESS)  // send the connect packet
        goto exit; // there was a problem

    // this will be a blocking call, wait for the connack
    if (waitfor(c, CONNACK, &connect_timer) == CONNACK)
    {
        unsigned char connack_rc = 255;
        char sessionPresent = 0;
        if (MQTTDeserialize_connack((unsigned char*)&sessionPresent, &connack_rc, c->readbuf, c->readbuf_size) == 1)
            rc = connack_rc;
        else
            rc = FAILURE;
    }
    else
        rc = FAILURE;

exit:
    if (rc == SUCCESS)
        c->isconnected = 1;
    return rc;
}
#endif

#ifdef EV_WRAP
int MQTTSubscribe(Client* c, const char* topicFilter, enum QoS qos, messageHandler messageHandler)
{ 
    int rc = FAILURE;  
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;


    if (!c->isconnected)
        goto exit;
    
    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);
    if (len <= 0)
    {
    	log_printf(LOG_ERROR, "MQTTSerialize_subscribe failed\n");
    	goto exit;
    }
    if ((rc = sendPacket_ev(c, len)) != SUCCESS) // send the subscribe packet use libev
        goto exit;             // there was a problem

	int i;
	for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
	{
		if (c->messageHandlers[i].topicFilter == 0)
		{
			c->messageHandlers[i].topicFilter = topicFilter;
			c->messageHandlers[i].fp = messageHandler;
			rc = 0;
			break;
		}
	}
exit:
    return rc;
}
//int MQTTSubscribe(Client* c, const char* topicFilter, enum QoS qos)
//{
//    int rc = FAILURE;
////   Timer timer;
//    int len = 0;
//    MQTTString topic = MQTTString_initializer;
//    topic.cstring = (char *)topicFilter;
//
////    InitTimer(&timer);
////    countdown_ms(&timer, c->command_timeout_ms);
//
//    if (!c->isconnected)
//        goto exit;
//
//    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);
//    if (len <= 0)
//    {
//    	log_printf(LOG_ERROR, "MQTTSerialize_subscribe failed\n");
//    	goto exit;
//    }
//    if ((rc = sendPacket_ev(c, len)) != SUCCESS) // send the subscribe packet use libev
//        goto exit;             // there was a problem
//exit:
//    return rc;
//}
#else
int MQTTSubscribe(Client* c, const char* topicFilter, enum QoS qos, messageHandler messageHandler)
{
    int rc = FAILURE;
    Timer timer;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;

    InitTimer(&timer);
    countdown_ms(&timer, c->command_timeout_ms);

    if (!c->isconnected)
        goto exit;

    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit;             // there was a problem

    if (waitfor(c, SUBACK, &timer) == SUBACK)      // wait for suback
    {
        int count = 0, grantedQoS = -1;
        unsigned short mypacketid;
        if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, c->readbuf, c->readbuf_size) == 1)
            rc = grantedQoS; // 0, 1, 2 or 0x80
        if (rc != 0x80)
        {
            int i;
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if (c->messageHandlers[i].topicFilter == 0)
                {
                    c->messageHandlers[i].topicFilter = topicFilter;
                    c->messageHandlers[i].fp = messageHandler;
                    rc = 0;
                    break;
                }
            }
        }
    }
    else
        rc = FAILURE;

exit:
    return rc;
}
#endif
#ifdef EV_WRAP
int MQTTUnsubscribe(Client* c, const char* topicFilter)
{
    int rc = FAILURE;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

    if (!c->isconnected)
        goto exit;

    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket_ev(c, len)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem
exit:
    return rc;
}
#else
int MQTTUnsubscribe(Client* c, const char* topicFilter)
{   
    int rc = FAILURE;
    Timer timer;    
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

    InitTimer(&timer);
    countdown_ms(&timer, c->command_timeout_ms);
    
    if (!c->isconnected)
        goto exit;
    
    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem
    
    if (waitfor(c, UNSUBACK, &timer) == UNSUBACK)
    {
        unsigned short mypacketid;  // should be the same as the packetid above
        if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
            rc = 0; 
    }
    else
        rc = FAILURE;
    
exit:
    return rc;
}
#endif

#ifdef EV_WRAP
int MQTTPublish(Client* c, const char* topicName, MQTTMessage* message)
{
    int rc = FAILURE;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;

    if(c->suborder != subscribe_over)
    {
    	log_printf(LOG_ERROR, "[MQTTPublish] publish but subcribe not over\n");
    	goto exit;
    	//TODO: error
    }

    int len = 0;
    if (!c->isconnected)
        goto exit;

    if (message->qos == QOS1 || message->qos == QOS2)
        message->id = getNextPacketId(c);
    
    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id, 
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket_ev(c, len)) != SUCCESS) // send the subscribe packet
        goto exit;
exit:
    return rc;
}
#else
int MQTTPublish(Client* c, const char* topicName, MQTTMessage* message)
{
    int rc = FAILURE;
    Timer timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

    InitTimer(&timer);
    countdown_ms(&timer, c->command_timeout_ms);

    if (!c->isconnected)
        goto exit;

    if (message->qos == QOS1 || message->qos == QOS2)
        message->id = getNextPacketId(c);

    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

    if (message->qos == QOS1)
    {
        if (waitfor(c, PUBACK, &timer) == PUBACK)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }
    else if (message->qos == QOS2)
    {
        if (waitfor(c, PUBCOMP, &timer) == PUBCOMP)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }

exit:
    return rc;
}
#endif
#ifdef EV_WRAP
int MQTTDisconnect(Client* c)
{
    int rc = FAILURE;
    int len = MQTTSerialize_disconnect(c->buf, c->buf_size);
    if (len > 0)
        rc = sendPacket_ev(c, len);            // send the disconnect packet

    c->isconnected = 0;
    return rc;
}
#else
int MQTTDisconnect(Client* c)
{  
    int rc = FAILURE;
    Timer timer;     // we might wait for incomplete incoming publishes to complete
    int len = MQTTSerialize_disconnect(c->buf, c->buf_size);

    InitTimer(&timer);
    countdown_ms(&timer, c->command_timeout_ms);

    if (len > 0)
        rc = sendPacket(c, len, &timer);            // send the disconnect packet
        
    c->isconnected = 0;
    return rc;
}
#endif

void client_para_reset(Client *c)
{
	c->ipstack->disconnect(c->ipstack);
	c->ipstack->my_socket = -1;
	c->isconnected =0;
	c->suborder = suscribe_none;
	c->reconnect_count = c->reconnect_count +1;
}
/*
 * mqtt重连情况：服务器没有响应ping包，socket断开连接。
 * */
int MQTTReConnect(Client *c)
{
	sleep(1);
	int rc = FAILURE;
	client_para_reset(c);
	log_printf(LOG_NOTICE, "reconnect count=%d\n", c->reconnect_count);
	if(c->reconnect_count > RECONNECT_MAX_COUNT)
	{
		system(SET_DARK_CLOUD);
		log_printf(LOG_ERROR, "[ReConnectExceedMax], exit\n");
		exit(1);
	} //超过重连次数退出程序
	stop_watchers("mqtt");
	mqtt_init();
	rc =SUCCESS;
	log_printf(LOG_NOTICE, "[ReConnectMqtt]\n");
	return rc;
}
