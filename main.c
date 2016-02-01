/*
 *	File name   : main.c
 *  Created on  : Jan 12, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include "ev_wrap.h"
#include "log_printf.h"
#include "mt_topic_handle.h"
#include "common.h"
#include "http_client.h"
#include "gateway_init.h"

#define HW_VERSION			"GL-HGZB02A-V0.1.0"
#define SW_VERSION			"alpha-V1.0.0"

void cfinish(int sig)
{
	signal(SIGINT, NULL);
}

int main()
{
	log_printf(LOG_VERBOSE, "[StartProgram]...........\n");
	log_printf(LOG_VERBOSE, "[HW-VERSION]: %s\n", HW_VERSION);
	log_printf(LOG_VERBOSE, "[SW_VERSION]: %s\n", SW_VERSION);
	log_printf(LOG_NOTICE, "COMPILE_TIME[%s: %s]\n", __DATE__, __TIME__);

#ifdef USE_IN_ARM
	sleep(10);
#endif
	unsigned char buf[MQTT_PLAYLOADLEN];
	unsigned char readbuf[MQTT_PLAYLOADLEN];

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	global_loop = EV_DEFAULT;
	global_ev_all.mainloop = global_loop;

	Network n;  //必须  //mqtt network实体
	gateway_setting_init();
	gwmsg_init();
	devlist_gateway_msg_init();
//	dev_init();
	NewNetwork(&n);
	MQTTClient(&global_ev_all.client, &n, KEEPALIVE_INTERVAL, buf, MQTT_PLAYLOADLEN, readbuf, MQTT_PLAYLOADLEN);

	mqtt_init();
	zigbee_init();
	ev_run(global_ev_all.mainloop, 0); //run...

	log_printf(LOG_ERROR, "[Stopping]...\n");
	MQTTDisconnect(&global_ev_all.client);
	n.disconnect(&n);

	return 0;
}
