#	MQTT lib

ARMCC = arm-arago-linux-gnueabi-gcc
ARMLIB_DIR = ../lib/

PCCC = gcc
PCLIB_DIR = ../lib/pc/

#切换编译版本需要配置几个参数：CC，LIB_DIR
CC = ${PCCC}
LIB_DIR = ${PCLIB_DIR}

CC_OPTS = -Wall -Werror -ggdb3 -I../include/
CUR_SOURCE = ${wildcard *.c}
CUR_OBJS = ${patsubst %.c, %.o, $(CUR_SOURCE)}

all: ${LIB_DIR}libmqttc.so

${LIB_DIR}libmqttc.so: ${CUR_OBJS}
	${CC} -shared -fPCI -o ${LIB_DIR}libmqttc.so ${CUR_OBJS}

${CUR_OBJS} : ${CUR_SOURCE}
	${CC} ${CC_OPTS} -c ${CUR_SOURCE}
	
clean:
	rm -f *.o *.so
#	rm -f ${LIB_DIR}libmqttc.so
