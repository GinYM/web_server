#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "handle_request.h"
#include <openssl/ssl.h>
#include <fcntl.h>

#if !defined(UTIL)
#include "../util/util.h"
#endif


//#define ECHO_PORT 9999
#define BUF_SIZE 5000
#define MAX_CLIENT_NUM 1024

struct pool{
    char buf[BUF_SIZE];
};

int start_server();