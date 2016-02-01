#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "file_operation.h"

int read_file_data(const char *path, char *data)
{
	int res =0;

    FILE* fp;
    int fd;
    int file_size;
    char file_buf[65535];	//xpz to change

    fp = fopen(path, "a+");
	if (fp == NULL)
	{
		fclose(fp);
		res = OPENED_FILE_FAILED;
		return res;
	}

	fd = fileno(fp);
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);

	rewind(fp);

	if (file_size > 0)
	{
		fread(file_buf, sizeof(char), file_size, fp);
		file_buf[file_size] = '\0';
		memcpy(data, file_buf, file_size+1);
	}
	else
	{
		res = FILE_NOT_DATA;
	}
	fclose(fp);

	return res;
}

int write_file_data(const char *path, const char *data)
{
	int res =0;

    FILE* fp;
    int fd;
    int file_size;

    fp = fopen(path, "a+");
	if (fp == NULL)
	{
		fclose(fp);
		res = OPENED_FILE_FAILED;
		return res;
	}

	fd = fileno(fp);
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);
	if (file_size > 0)
	{
		if(ftruncate(fd, 0)<0) {  //清空文件内容
			perror("ftruncate failed\n");
			res = FILE_WRITE_DATA_FAILED;
		}
		else
			fwrite(data, sizeof(char), strlen(data)+1, fp);
	}

	fclose(fp);

	return res;
}
