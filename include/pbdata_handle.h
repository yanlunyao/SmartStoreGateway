/*
 *	File name   : pbdata_handle.h
 *  Created on  : Jan 23, 2016
 *  Author      : pengjf
 *  Description : protobuf data handling releated declaration
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_PBDATA_HANDLE_H_
#define INCLUDE_PBDATA_HANDLE_H_

#include "SmartStore.pb.h"
#include "cJSON.h"

//int ss_decode_pb(uint8_t *buffer,int buf_len,SSMsg *ssmsg);
//int ss_encode_pb(uint8_t *buffer,int *buf_len,cJSON *json);

int ss_pb_decode(uint8_t *buffer,int buf_len,SSMsg *ssmsg);
int ss_pb_encode(uint8_t *buffer,int buf_len,int msg_type,char **args);

void ssmsg_handle(SSMsg *ssmsg, const char * topicname);
int get_deviceid_zonetype(char *device_id,char *zone_type,char *nwk_addr);
int dev_exist(char *nwk_addr);
void gwmsg_write(SSMsg *ssmsg); //add by yanly
#endif /* INCLUDE_PBDATA_HANDLE_H_ */
