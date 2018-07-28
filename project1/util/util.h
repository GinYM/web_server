#define UTIL

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

//#if !defined(PARSER)
//#include "../parser/parse.h"
//#endif

int HTTP_port;
int HTTPS_port;
char *log_file;
char *lock_file;
char *www_folder;
char *cgi_script_path;
char *private_key_file;
char *certificate_file;
int SSL_PORT;

FILE* f_log;

void print_log(const char* format, ...);

#define BUF_SIZE 4096
#define MAX_CLIENT_NUM 1024



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

struct Pool{
    Request *request;
    int stdin_pipe[2];
    int stdout_pipe[2];
    char buf[BUF_SIZE];
    int buflen;
};