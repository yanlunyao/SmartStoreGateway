///*
// *	File name   : test.c
// *  Created on  : Jan 11, 2016
// *  Author      : yanly
// *  Description :
// *  Version     :
// *  History     : <author>		<time>		<version>		<desc>
// */
//
//#include "ev_wrap.h"
//#include "log_printf.h"
//#include "mt_topic_handle.h"
//#include "common.h"
//#include "http_client.h"
//#include "gateway_init.h"
//
//void cfinish(int sig)
//{
//	signal(SIGINT, NULL);
//}
//int main()
//{
//	log_printf(LOG_VERBOSE, "[StartProgram]...........\n");
//	unsigned char buf[MQTT_BUFFER_LEN];  //
//	unsigned char readbuf[MQTT_BUFFER_LEN];
//
//	signal(SIGINT, cfinish);
//	signal(SIGTERM, cfinish);
//
//	global_loop = EV_DEFAULT;
//	global_ev_all.mainloop = global_loop;
//
//	Network n;  //必须  //mqtt network实体
//	gateway_info_init();
//	NewNetwork(&n);
//	MQTTClient(&global_ev_all.client, &n, KEEPALIVE_INTERVAL, buf, MQTT_BUFFER_LEN, readbuf, MQTT_BUFFER_LEN);
//
//	mqtt_init();
//	zigbee_init();
//	ev_run(global_ev_all.mainloop, 0); //run...
//
//	//http组串后调用http_req_start
////	http_cev *_hcv;
////	_hcv = malloc(sizeof(http_cev));
//////	_hcv.cev = &global_ev_all;
//////	_hcv.http_client.httpport= 8002;
//////	sprintf(_hcv.http_client.httpserver, "%s", "192.168.1.206");
//////	sprintf(_hcv.http_client.sendbuf, "%s", "http_res_did");
//////	_hcv.http_client.send_len = sizeof("http_res_did");
////////	http_req_start(&_cev, "192.168.1.206", 8002, "httptest", sizeof("httptest"), http_res_mtinfo);
//////	http_req_start(&_hcv, NULL, http_res_did);
////	_hcv->cev = &global_ev_all;
////	_hcv->http_client.httpport= 8002;
////	sprintf(_hcv->http_client.httpserver, "%s", "192.168.1.206");
////	sprintf(_hcv->http_client.sendbuf, "%s", "http_res_did");
////	_hcv->http_client.send_len = sizeof("http_res_did");
////
////	http_req_start("test",4, "192.168.1.206", 8002,  NULL, http_res_did);
////	http_req_start(_hcv, NULL, http_res_did);
////
////	ev_run(global_ev_all.mainloop, 0); //run...
//
//
//	printf("Stopping\n");
//	MQTTDisconnect(&global_ev_all.client);
//	n.disconnect(&n);
//
//	return 0;
//}
