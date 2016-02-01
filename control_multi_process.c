/*
 *	File name   : control_multi_process.c
 *  Created on  : Jan 26, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include "zigbee_protocol.h"
#include "alarm_control.h"
#include "log_printf.h"


//macro
#define ALARM_TOOL			"mplayer -loop"
#define ALARM_FILE_PATH		"/gl/res/"

//global variable
const char *alarm_file_name[10] = {  //播放报警音乐的文件名字
		"none",
		"burglar",
		"fire",
		"emergency",
		"devicetrouble",
		"lowbattery",
		"doorbell"
};

static int system_mul_process(const char *string)
{
	int res;
	pid_t	pid;
	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return(pid);				//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);					//child process
		return (pid);
	}
	res = system(string);
	if(res <0) {
		log_printf(LOG_ERROR, "system call failed,res is: %d\n", res);
	}
	exit(0);
	return 0;
}
/*
 * descriptinon:
 * mode--报警音模式
 * duration--报警时长，为0时表示停止报警。默认报警时长使用gateway_info.duration
 * */
void alarm_control(enum alarm_mode mode, int duration)
{
	char cmd[100] ={0};
	if(duration <0)
		return;
	if(mode > max_alarm)
		return;

	system("killall -9 mplayer");
	if(duration ==0)
		return;
	sprintf(cmd, "%s %d %s%s", ALARM_TOOL, duration, ALARM_FILE_PATH, alarm_file_name[mode]);
	printf("alarm cmd=%s\n", cmd);
	system_mul_process(cmd);
}
