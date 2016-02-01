/*
 *	File name   : gatewayHwControl.h
 *  Created on  : Aug 14, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef GATEWAY_GATEWAYHWCONTROL_H_
#define GATEWAY_GATEWAYHWCONTROL_H_

//gpio47 SET_GPIO  控制按键
#define SETIO_L				"echo 0 > /sys/class/gpio/gpio47/value"
#define SETIO_H				"echo 1 > /sys/class/gpio/gpio47/value"

//cloud led
#define SET_DARK_CLOUD 		"echo 0 > /sys/class/leds/cloud_led/brightness"
#define SET_LIGHT_CLOUD 	"echo 1 > /sys/class/leds/cloud_led/brightness"

////zigbee led
//#define SET_DARK_ZIGBEE
//#define SET_LIGHT_ZIGBEE

//sys led
#define SET_TIMER_SYS    	"echo timer > /sys/class/leds/sys_led/trigger"
#define	SET_DELAY_ON_SYS	"echo 80 > /sys/class/leds/sys_led/delay_on"
#define SET_DELAY_OFF_SYS	"echo 80 > /sys/class/leds/sys_led/delay_off"
#endif /* GATEWAY_GATEWAYHWCONTROL_H_ */
