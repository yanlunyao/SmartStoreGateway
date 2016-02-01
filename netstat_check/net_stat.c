/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName:      net_stat.c
  Author:        马晓槟
  Version :      1.1
  Date:          2014-11-11
  Description:   检测网络状态。当网线连接时杀死网关主模块，当网线断开时熄灭
                 CLOUD指示灯并杀死网关主模块。
                 主模块被杀死后会由systemd重启。
  History:        
      <author>    <time>    <version>    <desc>
      马晓槟     2014-11-11   v1.1      删除killall命令-9参数；
                                        网线连接时也杀死主模块
	  yanly		2015-01-29   v1.0		智能家居项目移植到智慧商铺项目用。
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "gatewayHwControl.h"

#define ETHTOOL_GLINK        0x0000000A
#define SIOCETHTOOL          0x00008946
#define UNCONNECT            0
#define CONNECT              1

struct ethtool_value {
    int cmd;
    int data;
}; 

int main()
{
    int sock, last;
    struct ifreq ifr;
    struct ethtool_value edata;

    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;
   
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1)
    {
        return -1;
    }

    strcpy(ifr.ifr_name, "eth0");
    ifr.ifr_data = (char *)&edata;

    // 获取状态
    ioctl(sock, SIOCETHTOOL, &ifr);   
    last = edata.data;

    // 不断的读取网卡的状态
    while (1)
    {
        sleep(1);
        ioctl(sock, SIOCETHTOOL, &ifr);

        // 如果状态没有改变就跳过，直接执行下一次查询
        if (edata.data == last)
        {
            continue;
        }
        else
        {
            if (last == UNCONNECT)      // 如果网线被插上了
            {
                last = CONNECT;         // 把状态改为已连接
                system("killall -9 GatewayMain");
            }
            else if (last == CONNECT)   // 如果断开了网线
            {
                //system("echo none > /sys/class/leds/14/trigger");
            	system(SET_DARK_CLOUD);
                system("killall -9 GatewayMain");
                last = UNCONNECT;       // 状态改为已断开
            }
        }   
        waitpid(-1, NULL, 0);           // 回收子进程
    }
    return 0;
}

