/*
 *	File name   : http_client.h
 *  Created on  : Jan 14, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_HTTP_H_
#define INCLUDE_HTTP_H_


#define HTTP_BUF_MAXLEN		65535+1
#define HTTP_SERVER_LEN		100+1

enum httpsendcmd { send_none=0, req_did = 1, req_mt_info = 2};
typedef struct http_client_st httpclient_st;
//函数指针
//typedef int (*httphander)(httpclient_st*, void*);
//typedef int (*http_callback_hander)(const char*, int, void*);
typedef int (*http_callback_hander)(void *, const char*, int, void*);

//struct http_client_st {
//
//    int send_len; //actual send length
//    int recv_len;
//    char send_type; //in order to distinguish what type was send
//    char sendbuf[HTTP_BUF_MAXLEN]; //will be send buffer
//    char readbuf[HTTP_BUF_MAXLEN];
//    unsigned int timeout;
//    int isconnected;
//    int my_socket;
//    char httpserver[HTTP_SERVER_LEN];
//    int httpport;
//
//    int (*fp) (httpclient_st*, void*);
//    void *callback_data;		//callback传入参数
//};
struct http_client_st {

    int send_len; //actual send length
    int recv_len;
//    char send_type; //in order to distinguish what type was send
    char sendbuf[HTTP_BUF_MAXLEN]; //will be send buffer
    char readbuf[HTTP_BUF_MAXLEN];
    unsigned int timeout;
    int isconnected;
    int my_socket;
    char httpserver[HTTP_SERVER_LEN];
    int httpport;

    int (*fp) (void *, const char*, int, void*);  //传入接收的buffer，buffer length， 自定义的callback para
    void *callback_para;		//callback传入参数
};

int http_req_did(char *http_sendbuf,int buflen, const char *host, const char *url,
				 const char *passcode, const char *sn, const char *product_key);
int http_req_mtinfo(char *http_sendbuf,int buflen,const char *host,
			const char *url, const char *did);
int http_res_mtinfo(void *self, const char *recvbuf, int recvbuf_len, void *callback_para);
int http_res_did(void *self, const char *recvbuf, int recvbuf_len, void *callback_para);

void http_socket_close(httpclient_st *hc);
#endif /* INCLUDE_HTTP_H_ */
