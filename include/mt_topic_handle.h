/*
 *	File name   : mt_topic_handle.h
 *  Created on  : Jan 13, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_MT_TOPIC_HANDLE_H_
#define INCLUDE_MT_TOPIC_HANDLE_H_

#include "ev_wrap.h"

void mh_none(MessageData* md, Client *c);
void mh_messageArrived(MessageData* md, Client *c);
void ser2cli_res_process(MessageData* md, Client *c);
void ser2cli_noti_process(MessageData* md, Client *c);
void app2dev_process(MessageData* md, Client *c);

#endif /* INCLUDE_MT_TOPIC_HANDLE_H_ */
