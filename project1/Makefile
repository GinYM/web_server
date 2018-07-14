################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for liso server.              #
#                                                                              #
# Authors: Yiming Jin <jinyiming1996@gmail.com>,                               #
#                                                                              #
################################################################################

HEADERS = lisod.h util/util.* server/echo_server.h server/echo_server.c
CC   = gcc
CFLAGS = -Wall
LDFLAGS = 
OBJFILES = lisod.o
TARGET = lisod

all: ${TARGET}

${TARGET}: ${OBJFILES}
	${CC} ${CFLAGS} -o ${TARGET} ${OBJFILES} ${HEADERS} ${LDFLAGS}


clean:
	rm -f ${OBJFILES} ${TARGET} *~
