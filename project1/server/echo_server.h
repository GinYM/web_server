#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#if !defined(UTIL)
#include "../util/util.h"
#endif

#define ECHO_PORT 9999
#define BUF_SIZE 4096
#define MAX_CLIENT_NUM 1000

struct pool{
    char buf[BUF_SIZE];
};

int start_server();