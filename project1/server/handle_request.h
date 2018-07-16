/*********************************************************************************
* handle_request.h                                                               *
*                                                                                *
* Description: Header for handle_request.c                                       *                                   
*                                                                                *
* Authors: Yiming Jin <jinyiming1996@gmail.com>                                  *
*                                                                                *
*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "../parser/parse.h"

#if !defined(UTIL)
#include "../util/util.h"
#endif

int handle_request(char *buffer, int size, int socketFd, char **resp_buf);
int respond_error(char *buffer, int size, int socketFd, char **resp_buf);
