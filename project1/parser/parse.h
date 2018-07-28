#define PARSER

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(UTIL)
#include "../util/util.h"
#endif

#define SUCCESS 0

//Header field


Request* parse(char *buffer, int size,int socketFd);
