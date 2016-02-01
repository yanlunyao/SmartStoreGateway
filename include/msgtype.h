/*
 *	File name   : msgtype.h
 *  Created on  : Jan 28, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_MSGTYPE_H_
#define INCLUDE_MSGTYPE_H_


enum request_msgtype{
	get_gateway_msg_reqtype=1,
	get_devlist_reqtype,
	set_allow_jonin_net_reqtype,
	del_dev_reqtype,
	stop_alarm_reqtype=7,

};
enum upload_msgtype{
	new_dev_added_updtype=300,
	del_dev_updtype,
	dev_alarm_udptype=305,
};

#define EPS_ATTR_COUNT	7

enum eps_attr_order
{
	aa=1,
};

#endif /* INCLUDE_MSGTYPE_H_ */
