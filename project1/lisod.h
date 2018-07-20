#define LISOD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "server/server.h"


extern char *HTTP_port;
extern char *HTTPS_port;
extern char *log_file;
extern char *lock_file;
extern char *www_folder;
extern char *cgi_script_path;
extern char *private_key_file;
extern char *certificate_file;
extern FILE *f_log;

