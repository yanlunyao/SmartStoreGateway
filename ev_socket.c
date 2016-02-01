/*
 *	File name   : ev_socket.c
 *  Created on  : Jan 14, 2016
 *  Author      : yanly
 *  Description : tcp底层通信接口
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>


#include "log_printf.h"
#include "ev_socket.h"

/*
 * description：
 * input:
 * output: 成功返回实际接收到的数据长度，失败返回-1
 * */
int ev_connect(int *socketfd_ptr, char* addr, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = -1;

		freeaddrinfo(result);
	}
	if (rc == 0)
	{
		*socketfd_ptr = socket(family, type, 0);
		if (*socketfd_ptr != -1)
		{
			setnonblock(*socketfd_ptr);	// Set it non-blocking, it is important
			rc = connect(*socketfd_ptr, (struct sockaddr*)&address, sizeof(address));
		}
	}
	return rc;
}


/*
 * description：
 * input:
 * output: 成功返回实际接收到的数据长度，失败返回-1
 * */
int ev_read(int socketfd, void* buffer, int bufferlen)
{
	int n;
	int rc =-1;
	n = recv(socketfd, buffer, bufferlen, 0);
	if (n<=0)
	{
		if (n ==0)
		{
			log_printf(LOG_ERROR, "ev_read, socket %d disconnect\n", socketfd);
			//an orderly disconnect
			goto exit;
		}
		else {
			if (errno != ENOTCONN && errno != ECONNRESET) {
				log_printf(LOG_ERROR, "ev_read, socket %d error\n", socketfd);
				goto exit;	//error
			}
		}
	}
	else
	{
		if(n == bufferlen)
		{
			log_printf(LOG_ERROR, "ev_read, socket %d buffer too short\n", socketfd);
			//buffer长度不够用
			goto exit;
		}
	}
	rc = n;
exit:
	return rc;
}
/*
 * description：
 * input:
 * output: 成功返回0，失败返回-1
 * */
int ev_write(int socketfd, const void* buffer, int sendlen)
{
	int rc = -1,
        sent = 0,
		n =0;

    while (sent < sendlen)
    {
        n = write(socketfd, buffer, sendlen);
        if (n < 0)  // there was an error writing the data
        {
        	break;
        }
        sent += n;
    }
    if (sent == sendlen)
    {
    	//send success
        rc = 0;
    }
    else
    {
    	rc =-1;
    }
	return rc;
}
/*
 * description:
 * input:
 * output:
 * */
void ev_fd_disconnect(int socketfd)
{
	close(socketfd);
}
/*
 *description: Simply adds O_NONBLOCK to the file descriptor of choice，
 * The manual recommendation 'ev_io' best use non-blocking mode.
 */
int setnonblock(int socketfd)
{
  int flags;

  flags = fcntl(socketfd, F_GETFL);
  flags |= O_NONBLOCK;
  return fcntl(socketfd, F_SETFL, flags);
}

/***************************************************************************
  Function: getmac
  Description: get mac address
  Input: ifname
  Output: mac
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int getmac(const char *ifname, unsigned char *mac)
{
	int sockfd;
	struct ifreq ifr;

	if (ifname == NULL || mac == NULL)
	return -1;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);
	ioctl(sockfd, SIOCGIFHWADDR, &ifr);
	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

	close(sockfd);

	return 0;
}

