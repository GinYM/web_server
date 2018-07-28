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



#include "cgi.h"


/**************** BEGIN CONSTANTS ***************/
//#define FILENAME "./cgi_script.py"
//#define BUF_SIZE 4096

/* note: null terminated arrays (null pointer) */





char* ENVP_SET[] = {
                    "CONTENT_LENGTH",
                    "CONTENT-TYPE",
                    "GATEWAY_INTERFACE",
                    "QUERY_STRING",
                    "REMOTE_ADDR",
                    "REMOTE_HOST",
                    "REQUEST_METHOD",
                    "SCRIPT_NAME",
                    "HOST_NAME",
                    "SERVER_PORT",
                    "SERVER_PROTOCOL",
                    "SERVER_SOFTWARE",
                    "HTTP_ACCEPT",
                    "HTTP_REFERER",
                    "HTTP_ACCEPT_ENCODING",
                    "HTTP_ACCEPT_LANGUAGE",
                    "HTTP_ACCEPT_CHARSET",
                    "HTTP_COOKIE",
                    "HTTP_USER_AGENT",
                    "HTTP_CONNECTION",
                    "HTTP_HOST",
                    NULL
               };


char* POST_BODY = "This is the stdin body...\n";
/**************** END CONSTANTS ***************/

char* getHeader(char* header_name, Request *request){
    for(int i = 0;i<request->header_count;i++){
        if(strcmp(request->headers->header_name, header_name) == 0){
            return request->headers->header_value;
        }
    }
    return "";
}

CGI_param * initializeCGI(Request *request){
    CGI_param *cgi = malloc(sizeof(CGI_param));
    cgi->argv[0] = strdup(cgi_script_path);
    strcat(cgi->argv[0], request->http_uri+4);
    cgi->argv[1] = NULL;
    char tmp[1024];
    sprintf(tmp, "CONTENT_LENGTH=%ld",request->body.length);
    cgi->envp[0] = strdup(tmp);
    char *value;
    for(int i = 1;i<21;i++){
        value = getHeader(ENVP_SET[i], request);
        sprintf(tmp, "%s=%s", ENVP_SET[i], value);
        cgi->envp[i] = strdup(tmp);
    }
    cgi->envp[21] = NULL;
    
    


    return cgi;
}



/**************** BEGIN UTILITY FUNCTIONS ***************/
/* error messages stolen from: http://linux.die.net/man/2/execve */
void execve_error_handler()
{
    switch (errno)
    {
        case E2BIG:
            fprintf(stderr, "The total number of bytes in the environment \
(envp) and argument list (argv) is too large.\n");
            return;
        case EACCES:
            fprintf(stderr, "Execute permission is denied for the file or a \
script or ELF interpreter.\n");
            return;
        case EFAULT:
            fprintf(stderr, "filename points outside your accessible address \
space.\n");
            return;
        case EINVAL:
            fprintf(stderr, "An ELF executable had more than one PT_INTERP \
segment (i.e., tried to name more than one \
interpreter).\n");
            return;
        case EIO:
            fprintf(stderr, "An I/O error occurred.\n");
            return;
        case EISDIR:
            fprintf(stderr, "An ELF interpreter was a directory.\n");
            return;
        case ELIBBAD:
            fprintf(stderr, "An ELF interpreter was not in a recognised \
format.\n");
            return;
        case ELOOP:
            fprintf(stderr, "Too many symbolic links were encountered in \
resolving filename or the name of a script \
or ELF interpreter.\n");
            return;
        case EMFILE:
            fprintf(stderr, "The process has the maximum number of files \
open.\n");
            return;
        case ENAMETOOLONG:
            fprintf(stderr, "filename is too long.\n");
            return;
        case ENFILE:
            fprintf(stderr, "The system limit on the total number of open \
files has been reached.\n");
            return;
        case ENOENT:
            fprintf(stderr, "The file filename or a script or ELF interpreter \
does not exist, or a shared library needed for \
file or interpreter cannot be found.\n");
            return;
        case ENOEXEC:
            fprintf(stderr, "An executable is not in a recognised format, is \
for the wrong architecture, or has some other \
format error that means it cannot be \
executed.\n");
            return;
        case ENOMEM:
            fprintf(stderr, "Insufficient kernel memory was available.\n");
            return;
        case ENOTDIR:
            fprintf(stderr, "A component of the path prefix of filename or a \
script or ELF interpreter is not a directory.\n");
            return;
        case EPERM:
            fprintf(stderr, "The file system is mounted nosuid, the user is \
not the superuser, and the file has an SUID or \
SGID bit set.\n");
            return;
        case ETXTBSY:
            fprintf(stderr, "Executable was open for writing by one or more \
processes.\n");
            return;
        default:
            fprintf(stderr, "Unkown error occurred with execve().\n");
            return;
    }
}
/**************** END UTILITY FUNCTIONS ***************/

int handle_cgi_read(struct Pool * p, int fd, fd_set* cgi_read_set, fd_set*cgi_write_set)
{
    //initialize cgi param
    //CGI_param* param = initializeCGI(p[fd].request);

    /*************** BEGIN VARIABLE DECLARATIONS **************/
    //int stdin_pipe[2];
    //int stdout_pipe[2];
    
    int readret;
    /*************** END VARIABLE DECLARATIONS **************/

    /*************** BEGIN PIPE **************/
    /* 0 can be read from, 1 can be written to */

    /* you want to be looping with select() telling you when to read */
    /*
    while((readret = read(stdout_pipe[0], buf, BUF_SIZE-1)) > 0)
    {
        buf[readret] = '\0'; 
        fprintf(stdout, "Got from CGI: %s\n", buf);
    }
    */

    readret = read(p[fd].stdout_pipe[0], p[fd].buf, BUF_SIZE-1);
    p[fd].buflen = readret;
    p[fd].buf[readret] = '\0';
    FD_CLR(fd, cgi_read_set);

    if (readret >0 && readret <= BUF_SIZE)
    {
        fprintf(stdout, "CGI spawned process returned with EOF as \
expected.\n");
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int handle_cgi_write(struct Pool *p, int fd, fd_set* cgi_read_set, fd_set*cgi_write_set)
{
    //initialize cgi param
    CGI_param* param = initializeCGI(p[fd].request);

    /*************** BEGIN VARIABLE DECLARATIONS **************/
    pid_t pid;
    //int stdin_pipe[2];
    //int stdout_pipe[2];
    //char buf[BUF_SIZE];
    //int readret;
    /*************** END VARIABLE DECLARATIONS **************/

    /*************** BEGIN PIPE **************/
    /* 0 can be read from, 1 can be written to */
    if (pipe(p[fd].stdin_pipe) < 0)
    {
        print_log("[ERROR]: Error piping for stdin.\n");
        return EXIT_FAILURE;
    }

    if (pipe(p[fd].stdout_pipe) < 0)
    {
        print_log("[ERROR]: Error piping for stdout.\n");
        return EXIT_FAILURE;
    }
    /*************** END PIPE **************/

    /*************** BEGIN FORK **************/
    pid = fork();
    /* not good */
    if (pid < 0)
    {
        print_log("[ERROR]: Something really bad happened when fork()ing.\n");
        return EXIT_FAILURE;
    }

    /* child, setup environment, execve */
    if (pid == 0)
    {
        /*************** BEGIN EXECVE ****************/
        close(p[fd].stdout_pipe[0]);
        close(p[fd].stdin_pipe[1]);
        dup2(p[fd].stdout_pipe[1], fileno(stdout));
        dup2(p[fd].stdin_pipe[0], fileno(stdin));
        /* you should probably do something with stderr */

        /* pretty much no matter what, if it returns bad things happened... */
        if (execve(param->argv[0], param->argv, param->envp))
        {
            execve_error_handler();
            print_log("[ERROR]: Error executing execve syscall.\n");
            return EXIT_FAILURE;
        }
        /*************** END EXECVE ****************/
        FD_SET(fd, cgi_read_set);
    }

    if (pid > 0)
    {
        print_log("[INFO]: Parent: Heading to select() loop.\n");
        close(p[fd].stdout_pipe[1]);
        close(p[fd].stdin_pipe[0]);

        // send message body
        if (write(p[fd].stdin_pipe[1], p[fd].request->body.data, p[fd].request->body.length) < 0)
        {
            fprintf(stderr, "Error writing to spawned CGI program.\n");
            return EXIT_FAILURE;
        }

        close(p[fd].stdin_pipe[1]); /* finished writing to spawn */
        FD_CLR(fd, cgi_write_set);

    }
    /*************** END FORK **************/

    fprintf(stderr, "Process exiting, badly...how did we get here!?\n");
    return EXIT_FAILURE;
}


int handle_cgi(Request *request, fd_set* cgi_read_set, fd_set*cgi_write_set, char*read_bug, char* write_buf)
{
    //initialize cgi param
    CGI_param* param = initializeCGI(request);

    /*************** BEGIN VARIABLE DECLARATIONS **************/
    pid_t pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
    char buf[BUF_SIZE];
    int readret;
    /*************** END VARIABLE DECLARATIONS **************/

    /*************** BEGIN PIPE **************/
    /* 0 can be read from, 1 can be written to */
    if (pipe(stdin_pipe) < 0)
    {
        print_log("[ERROR]: Error piping for stdin.\n");
        return EXIT_FAILURE;
    }

    if (pipe(stdout_pipe) < 0)
    {
        print_log("[ERROR]: Error piping for stdout.\n");
        return EXIT_FAILURE;
    }
    /*************** END PIPE **************/

    /*************** BEGIN FORK **************/
    pid = fork();
    /* not good */
    if (pid < 0)
    {
        print_log("[ERROR]: Something really bad happened when fork()ing.\n");
        return EXIT_FAILURE;
    }

    /* child, setup environment, execve */
    if (pid == 0)
    {
        /*************** BEGIN EXECVE ****************/
        close(stdout_pipe[0]);
        close(stdin_pipe[1]);
        dup2(stdout_pipe[1], fileno(stdout));
        dup2(stdin_pipe[0], fileno(stdin));
        /* you should probably do something with stderr */

        /* pretty much no matter what, if it returns bad things happened... */
        if (execve(param->argv[0], param->argv, param->envp))
        {
            execve_error_handler();
            print_log("[ERROR]: Error executing execve syscall.\n");
            return EXIT_FAILURE;
        }
        /*************** END EXECVE ****************/ 
    }

    if (pid > 0)
    {
        print_log("[INFO]: Parent: Heading to select() loop.\n");
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);

        // send message body
        if (write(stdin_pipe[1], request->body.data, request->body.length) < 0)
        {
            fprintf(stderr, "Error writing to spawned CGI program.\n");
            return EXIT_FAILURE;
        }

        close(stdin_pipe[1]); /* finished writing to spawn */

        /* you want to be looping with select() telling you when to read */
        while((readret = read(stdout_pipe[0], buf, BUF_SIZE-1)) > 0)
        {
            buf[readret] = '\0'; /* nul-terminate string */
            fprintf(stdout, "Got from CGI: %s\n", buf);
        }

        close(stdout_pipe[0]);
        close(stdin_pipe[1]);

        if (readret == 0)
        {
            fprintf(stdout, "CGI spawned process returned with EOF as \
expected.\n");
            return EXIT_SUCCESS;
        }
    }
    /*************** END FORK **************/

    fprintf(stderr, "Process exiting, badly...how did we get here!?\n");
    return EXIT_FAILURE;
}
