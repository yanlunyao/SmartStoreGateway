ARMCC = arm-arago-linux-gnueabi-gcc
ARMLIB_DIR = -L../lib/
ARMBIN_DIR = ../bin/
ARM_CCOPTS = -Wall -ggdb3 -I../include/

PCCC = gcc
PCLIB_DIR = -L../lib/pc/
PCBIN_DIR = ../bin/pc/
PC_CCOPTS = -Wall -ggdb3 -I../include/

#切换编译版本需要配置几个参数：CC，LIB_DIR，BIN_DIR
CC = ${ARMCC}
LIB_DIR = ${ARMLIB_DIR}
BIN_DIR = ${ARMBIN_DIR}
CC_OPTS = ${ARM_CCOPTS}

#CC_OPTS = -Wall -Werror -ggdb3 -I./include/
CC_OPTS = -Wall -ggdb3 -I../include/
CUR_SOURCE=${wildcard *.c}
CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}
LIBS =

PROGS = ${BIN_DIR}net_stat

all:	${PROGS}

${BIN_DIR}net_stat:	${CUR_SOURCE}
	$(CC) $(CC_OPTS) -o $@ ${CUR_SOURCE} ${LIB_DIR} ${LIBS}

clean:
	rm -f *.o *.so
	rm -f ${PROGS}