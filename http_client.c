/*
 *	File name   : http_client.c
 *  Created on  : Jan 14, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include "http_client.h"
#include "ev_socket.h"
#include "common.h"
#include "ev_wrap.h"
#include "log_printf.h"
#include "cJSON.h"
#include "file_operation.h"
#include "gateway_init.h"

#define kCRLFNewLine     "\r\n"
#define kCRLFLineEnding  "\r\n\r\n"


/*
 * description:解析http返回数据，获取mqtt服务器连接信息
 * input：http_recvbuf-http respond数据
 * output：成功返回0，失败返回-1.
 *
 * */
//int http_res_mtinfo(void *self, const char *recvbuf, int recvbuf_len, void *callback_para)
//{
//
//	printf("http_res_mtinfo is %s, len is %d\n", recvbuf, recvbuf_len);
//
//	mqtt_client_init(&global_ev_all, global_ev_all.client.ipstack, "iot.eclipse.org", 1883);
//	//解析
//	//连接mqtt
//	free(self);
//	return 0;
//}
int http_res_mtinfo(void *self, const char *recvbuf, int recvbuf_len, void *callback_para)
{
/***************************************************************************/
	int status;
	char * begin = strchr(recvbuf, '{');
	char * end = strrchr(recvbuf, '}');
	if(!begin || !end)
	{
		status =-1;
		goto errorhandle;
	}
	int http_recvsize = end - begin + 1;
	char http_recvbuf_body[HTTP_BUF_MAXLEN] = {0};
	strncpy(http_recvbuf_body, begin, http_recvsize);
	cJSON* json_recv ;
	json_recv = cJSON_Parse(http_recvbuf_body); //must delete
	if(json_recv == NULL)
	{
		status =-1;
		goto errorhandle;
	}
	//解析服务器的json
	cJSON* json_mqtt = cJSON_GetObjectItem(json_recv,"mqtt");
	int iSize = cJSON_GetArraySize(json_mqtt);
	int iCnt = 0;
	for(iCnt=0; iCnt < iSize; iCnt++ )
	{
		if(iCnt >2) //暂时只考虑存2个
			break;
		cJSON * pSub = cJSON_GetArrayItem(json_mqtt,iCnt);
		if(NULL == pSub)
		{
			continue;
		}
		cJSON* mt_server = cJSON_GetObjectItem(pSub, "url");
		cJSON* mt_port = cJSON_GetObjectItem(pSub, "port");
		char* tem_server = mt_server -> valuestring;
		int tem_port = mt_port -> valueint;
		if(iCnt == 0)
		{
//			strcpy(gateway_info.mt_server1,tem_server);
			snprintf(gateway_info.mt_server1, SERVER_MAX_LEN-1, "%s", tem_server);
			gateway_info.mt_port1 = tem_port;
		}
		if(iCnt == 1)
		{
//			strcpy(gateway_info.mt_server2,tem_server);
			snprintf(gateway_info.mt_server2, SERVER_MAX_LEN-1, "%s", tem_server);
			gateway_info.mt_port2 = tem_port;
		}
	}
	cJSON_Delete(json_recv);
	//将数据写入文件中
	int res=0;
	char read_data[GATEWAY_INFO_FILE_SIZE]={0};
	cJSON* json_all;
	res = read_file_data(GATEWAY_INFO_PATH, read_data);
	if(res == 0)
	{
		json_all = cJSON_Parse(read_data);
		if(json_all == NULL)
		{
			status =-1;
			goto errorhandle;
		}
	}
	else
	{
		status =-1;
		goto errorhandle;
	}
	cJSON* json_mt_server1 = cJSON_CreateString(gateway_info.mt_server1);
	cJSON* json_mt_server2 = cJSON_CreateString(gateway_info.mt_server2);
	cJSON* json_mt_port1 = cJSON_CreateNumber(gateway_info.mt_port1);
	cJSON* json_mt_port2 = cJSON_CreateNumber(gateway_info.mt_port2);

	cJSON_ReplaceItemInObject(json_all, "mt_server1", json_mt_server1);
	cJSON_ReplaceItemInObject(json_all, "mt_server2", json_mt_server2);
	cJSON_ReplaceItemInObject(json_all, "mt_port1", json_mt_port1);
	cJSON_ReplaceItemInObject(json_all, "mt_port2", json_mt_port2);
	char* tmp = cJSON_Print(json_all);
	write_file_data(GATEWAY_INFO_PATH, tmp);
	free(tmp);
	cJSON_Delete(json_all);
	printf_gateway_info();

/***************************************************************************/
	mqtt_client_init(&global_ev_all, gateway_info.mt_server1, gateway_info.mt_port1); //连接mqtt

errorhandle:
	if(status <0)
	{
		//todo:
	}
	return status;
}
///*
// * description:
// * self是动态分配的http_cev结构体
// * */
//int http_res_did(void *self, const char *recvbuf, int recvbuf_len, void *callback_para)
//{
////	//解析
////	//获取请求mqtt连接信息
//	printf("http_res_did is %s, len is %d\n", recvbuf, recvbuf_len);
//
////	if(gateway_info.mac ==0)
//	{
//		http_cev *httpcev;
//		httpcev = malloc(sizeof(http_cev));
//		httpcev->cev = &global_ev_all;
//		httpcev->http_client.httpport= 8002;
//		sprintf(httpcev->http_client.httpserver, "%s", "192.168.1.206");
//		sprintf(httpcev->http_client.sendbuf, "%s", "http_res_mtinfo");
//		httpcev->http_client.send_len = sizeof("http_res_mtinfo");
////		http_req_start(httpcev, NULL, http_res_mtinfo);
//		http_req_start("test2",5, "192.168.1.206", 8002,  NULL, http_res_mtinfo);
////		httpcev.cev = &global_ev_all;
////		httpcev.http_client.httpport= 8002;
////		sprintf(httpcev.http_client.httpserver, "%s", "192.168.1.206");
////		sprintf(httpcev.http_client.sendbuf, "%s", "http_res_mtinfo");
////		httpcev.http_client.send_len = sizeof("http_res_mtinfo");
////		http_req_start(&httpcev, NULL, http_res_mtinfo);
//	}
//	free(self);
//	return 0;
//}

int http_res_did(void *self, const char *http_recvbuf,int recvbuf_len, void *callback_para)
{
/***************************************************************************/
	int status =0;
	//gateway_info_st gateway_init;
	//解body
	char * begin = strchr(http_recvbuf, '{');
	char * end = strrchr(http_recvbuf, '}');
	if(!begin || !end)
	{
		status =-1;
		goto errorhandle;
	}
	int http_recvsize = end - begin + 1;
	char http_recvbuf_body[HTTP_BUF_MAXLEN] = {0} ;
	strncpy(http_recvbuf_body, begin, http_recvsize);

	cJSON *json_recv;
	cJSON *json_did;
	json_recv = cJSON_Parse(http_recvbuf_body);
	if(json_recv == NULL)
	{
		status =-1;
		goto errorhandle;
	}
	json_did = cJSON_GetObjectItem(json_recv,"did");
//	strcpy(gateway_info.did,json_did -> valuestring);	//赋值全局变量
	snprintf(gateway_info.did, sizeof(gateway_info.did), "%s", json_did -> valuestring);
	if(json_did == NULL)
	{
		status =-1;
		goto errorhandle;
	}
	int res;
	char read_data[GATEWAY_INFO_FILE_SIZE]={0};
	cJSON* json_all;
	res = read_file_data(GATEWAY_INFO_PATH, read_data);
	if(res == 0)
	{
		json_all = cJSON_Parse(read_data);
		if(json_all == NULL)
		{
			status =-1;
			goto errorhandle;
		}
	}
	else
	{
		status =-1;
		goto errorhandle;
	}
	cJSON_ReplaceItemInObject(json_all, "did", json_did);
	char* tmp = cJSON_Print(json_all);
	write_file_data(GATEWAY_INFO_PATH, tmp);
	cJSON_Delete(json_all);
	printf_gateway_info();

/***************************************************************************/
	//判断是否需要请求mqtt info
	char tembuf[HTTP_BUF_MAXLEN];
	int sendlen;
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
errorhandle:
	if(status <0)
	{
		//todo:
	}
	return status;
}
/*
 *http组包，成功返回sendbuf length，失败返回负值.
 * */
int http_req_did(char *http_sendbuf,int buflen, const char *host, const char *url,
				 const char *passcode, const char *sn, const char *product_key)//多一个url参数
{
	log_printf(LOG_NOTICE, "[ReqDid]\n");
	int rc;
	char temp_sendbuf[HTTP_BUF_MAXLEN] = {0};
	int ContentLen=0;
	//post的数据处理
	cJSON* tmp = cJSON_CreateObject();
	cJSON_AddStringToObject(tmp, "product_key", product_key);
	cJSON_AddStringToObject(tmp, "passcode", passcode);
	cJSON_AddStringToObject(tmp, "sn", sn);
	char *Content = cJSON_Print(tmp);
	ContentLen=strlen(Content);
	//建立http协议的数据
	int flag = snprintf( temp_sendbuf,buflen,"%s%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s%s%s",
						"POST ",url," HTTP/1.1",kCRLFNewLine,
						"Host: ",host,kCRLFNewLine,
						"Connection: close",kCRLFNewLine,
						"Cache-Control: no-cache",kCRLFNewLine,
						"Content-Length: ",ContentLen,kCRLFNewLine,
						"Content-Type: application/json;charset=UTF-8",kCRLFNewLine,
						 kCRLFNewLine,
						Content
						);
	if(flag > buflen)
	{
		return -1;
	}
	rc = snprintf(http_sendbuf, buflen, "%s", temp_sendbuf);
	free(Content);
	cJSON_Delete(tmp);
	return rc;
}
/*
 *http组包，成功返回sendbuf length，失败返回负值.
 * */
int http_req_mtinfo(char *http_sendbuf,int buflen,const char *host,
			const char *url, const char *did)//多一个url参数
{
	log_printf(LOG_NOTICE, "[ReqMqttInfo]\n");
	int rc;
	char temp_sendbuf[HTTP_BUF_MAXLEN] = {0};
	//建立http协议的数据
	int flag = snprintf(temp_sendbuf,buflen,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
						"GET ",url,did," HTTP/1.1",kCRLFNewLine,
						"Host: ",host,kCRLFNewLine,
						"Connection: keep-alive",kCRLFNewLine,
						"Cache-Control: no-cache",kCRLFNewLine,
						"Content-Type: application/json;charset=UTF-8",kCRLFLineEnding
						);
	if(flag > buflen)
	{
		return -1;
	}
	rc = snprintf(http_sendbuf, buflen, "%s", temp_sendbuf);

	return rc;
}

void http_socket_close(httpclient_st *hc)
{
	ev_fd_disconnect(hc->my_socket);
	hc->my_socket = -1;
	hc->isconnected = socket_notconnect;
}
