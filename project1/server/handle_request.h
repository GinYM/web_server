/*********************************************************************************
* handle_request.h                                                               *
*                                                                                *
* Description: Header for handle_request.c                                       *                                   
*                                                                                *
* Authors: Yiming Jin <jinyiming1996@gmail.com>                                  *
*                                                                                *
*********************************************************************************/

#define HANDLE_REQUEST

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
#include <sys/stat.h>

#include "../CGI/cgi.h"

//timmer
#include <time.h>

#if !defined(UTIL)
#include "../util/util.h"
#endif





struct Pool pool[MAX_CLIENT_NUM];

int handle_request(char *buffer, int size, int socketFd, char **resp_buf,int *isCGI, struct Pool * p, int fd);
int respond_error(char *buffer, int size, int socketFd, char **resp_buf);
