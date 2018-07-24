#define UTIL

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

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