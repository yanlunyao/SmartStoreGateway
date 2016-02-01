/*
 *	File name   : file_operation.h
 *  Created on  : Jan 21, 2016
 *  Author      : pengzhou
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_FILE_OPERATION_H_
#define INCLUDE_FILE_OPERATION_H_

#define READ_FILE_OK							 0
#define OPENED_FILE_FAILED						-1
#define FILE_DATA_NOT_JSON						-2
#define FILE_NOT_DATA							-3
#define FILE_WRITE_DATA_FAILED					-4

int read_file_data(const char *path, char *data);
int write_file_data(const char *path, const char *data);

#endif /* INCLUDE_FILE_OPERATION_H_ */
