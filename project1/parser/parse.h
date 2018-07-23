#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(UTIL)
#include "../util/util.h"
#endif

#define SUCCESS 0

//Header field
typedef struct
{
	char header_name[4096];
	char header_value[4096];
} Request_header;

typedef struct
{
	char *data;
	long capacity;
	long length;
} Request_body;

//HTTP Request Header
typedef struct
{
	char http_version[50];
	char http_method[50];
	char http_uri[4096];
	char http_connection[4096];
	Request_header *headers;
	int header_capacity;
	int header_count;
	Request_body body;

	//int suc; // whether parsing is successed
} Request;

Request* parse(char *buffer, int size,int socketFd);
