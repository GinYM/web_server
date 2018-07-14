#include "lisod.h"

char *HTTP_port;
char *HTTPS_port;
char *log_file;
char *lock_file;
char *www_folder;
char *cgi_script_path;
char *private_key_file;
char *certificate_file;

int main(int argc, char const *argv[])
{
    if(argc != 9){
        fprintf(stderr, "Please input 8 arguments!\n");
        exit(-1);
    }
    
    HTTP_port = strdup(argv[1]);
    HTTPS_port = strdup(argv[2]);
    log_file = strdup(argv[3]);
    lock_file = strdup(argv[4]);
    www_folder = strdup(argv[5]);
    cgi_script_path = strdup(argv[6]);
    private_key_file = strdup(argv[7]);
    certificate_file = strdup(argv[8]);

    f_log = fopen(log_file, "w");
    start_server();
    fclose(f_log);

    return 0;
}
