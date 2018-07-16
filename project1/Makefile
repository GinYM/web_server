################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for liso server.              #
#                                                                              #
# Authors: Yiming Jin <jinyiming1996@gmail.com>,                               #
#                                                                              #
################################################################################

DEPS = lisod.h util/util.* server/server.h server/server.c server/handle* parser/parse.h parser/y.tab.h parser/*.c
CC   = gcc
CFLAGS = -Wall
LDFLAGS = 
OBJFILES = lisod.o
TARGET = lisod

all: ${TARGET}

${TARGET}: ${OBJFILES}
	cd parser && ${MAKE} && cd ..
	${CC} ${CFLAGS} -o ${TARGET} ${OBJFILES} ${DEPS} ${LDFLAGS}


clean:
	rm -f ${OBJFILES} ${TARGET} *~ parser/*~ parser/*.o parser/lex.yy.c parser/y.tab.c parser/y.tab.h
