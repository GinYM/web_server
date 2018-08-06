/*****************************************************************************
 *                                                                           *
 * This file represents a hard-coded CGI driver example.  It will execute a  *
 * file, setup environment variables, and pipe stdin and stdout              *
 * appropriately.                                                            *
 *                                                                           *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>                        *
 *          Charles Rang <rang@cs.cmu.edu>                                   *
 *          Wolfgang Richter <wolf@cs.cmu.edu>                               *
 *****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>


#if !defined(UTIL)
#include "../util/util.h"
#endif

#if !defined(PARSER)
#include "../parser/parse.h"
#endif

//#if !defined(HANDLE_REQUEST)
//#include "../server/handle_request.h"
//#endif


/**************** BEGIN CONSTANTS ***************/
#define FILENAME "./cgi_script.py"
#define BUF_SIZE 4096

typedef struct CGI_param {
    char *argv[2];
    char *envp[22];
} CGI_param;

/* note: null terminated arrays (null pointer) */
/**************** END CONSTANTS ***************/

CGI_param * initializeCGI(Request *request);
char* getHeader(char* header_name, Request *request);

/**************** BEGIN UTILITY FUNCTIONS ***************/
/* error messages stolen from: http://linux.die.net/man/2/execve */
void execve_error_handler();
/**************** END UTILITY FUNCTIONS ***************/

//int handle_cgi(Request *request, fd_set* cgi_readt_set, fd_set*cgi_write_set);
int handle_cgi_read(struct Pool * p, int fd, fd_set* cgi_read_set, fd_set*cgi_write_set);
int handle_cgi_write(struct Pool * p, int fd, fd_set* cgi_read_set, fd_set*cgi_write_set);