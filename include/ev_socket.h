/*
 *	File name   : ev_socket.h
 *  Created on  : Jan 14, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_EV_SOCKET_H_
#define INCLUDE_EV_SOCKET_H_

int ev_connect(int *socketfd_ptr, char* addr, int port);
int ev_write(int socketfd, const void* buffer, int sendlen);
int ev_read(int socketfd, void* buffer, int bufferlen);

int setnonblock(int socketfd);
void ev_fd_disconnect(int socketfd);
int getmac(const char *ifname, unsigned char *mac);

#endif /* INCLUDE_EV_SOCKET_H_ */
