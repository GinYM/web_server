/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include "server.h"
//#include "../util/util.h"



int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}


//warpper for send, send all data
int sendall_nossl(int client_sock_name, char *buf, ssize_t content_size)
{
    if(content_size > BUF_SIZE){
        print_log("[ERROR] No more than BUF_SIZE:%d, current:%ld", BUF_SIZE, content_size);
    }
    ssize_t left = content_size;
    ssize_t ret_size = 0;
    while(left > 0){
        print_log("[DEBUUG] Sending :%s\n", buf);
        ret_size = send(client_sock_name, buf, left, 0);
        buf += ret_size;
        left -= ret_size;
        
    }
    print_log("[DEBUG]: in sendall_nossl send size:%d\n", content_size-left);
    return content_size-left;    
}



//warpper for send, send all data
int sendall(SSL *client_context, char *buf, ssize_t content_size)
{
    if(content_size > BUF_SIZE){
        print_log("[ERROR] No moew than BUF_SIZE:%d, current:%ld", BUF_SIZE, content_size);
    }
    ssize_t left = content_size;
    ssize_t ret_size = 0;
    while(left > 0){
        print_log("[DEBUG]: Sending :%s\n", buf);
        
        ret_size = SSL_write(client_context, buf, left);
        print_log("[DEBUG]: ret_size:%d\n", ret_size);
        //ret_size = send(client_sock_name, buf, left, 0);
        buf += ret_size;
        left -= ret_size;
        
    }
    return content_size-left;  
        
}

SSL_CTX* ssl_init(){

    /************ SSL INIT ************/
    SSL_CTX *ssl_context;
    SSL_load_error_strings();
    SSL_library_init();

    /* we want to use TLSv1 only */
    if ((ssl_context = SSL_CTX_new(TLSv1_server_method())) == NULL)
    {
        fprintf(stderr, "Error creating SSL context.\n");
        return NULL;
    }

    /* register private key */
    print_log("[DEBUG]: private_key_file: %s\n", private_key_file);
    if (SSL_CTX_use_PrivateKey_file(ssl_context, private_key_file,
                                    SSL_FILETYPE_PEM) == 0)
    {
        SSL_CTX_free(ssl_context);
        fprintf(stderr, "Error associating private key.\n");
        return NULL;
    }

    /* register public key (certificate) */
    print_log("[DEBUG]: certificate_file: %s\n", certificate_file);
    if (SSL_CTX_use_certificate_file(ssl_context, certificate_file,
                                     SSL_FILETYPE_PEM) == 0)
    {
        SSL_CTX_free(ssl_context);
        fprintf(stderr, "Error associating certificate.\n");
        return NULL;
    }
    /************ END SSL INIT ************/
    return ssl_context;
}

void set_fl(int fd, int flags) /* flags are file status flags to turn on */
{
  int val;

  if ((val = fcntl(fd, F_GETFL, 0)) < 0)
    print_log("fcntl F_GETFL error");

  val |= flags;		/* turn on flags */

  if (fcntl(fd, F_SETFL, val) < 0)
    print_log("fcntl F_SETFL error");
}

//introduction of select https://www.mkssoftware.com/docs/man3/select.3.asp
int start_server()
//int main(int argc, char* argv[])
{
    int master_sock, rc, ssl_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr, master_addr;
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    //int isSSL[MAX_CLIENT_NUM];
    //memset(isSSL, 0, MAX_CLIENT_NUM);
    char *allbuf;
    //int client_sock[MAX_CLIENT_NUM];

    /************ VARIABLE DECLARATIONS ************/
    SSL_CTX *ssl_context;
    SSL *client_context;
    SSL *all_ssl_context[MAX_CLIENT_NUM];

    ssl_context = ssl_init();

    if(ssl_context == NULL){
        print_log("[ERROR]: initializing ssl_context");
    }
    /************ END SSL INIT ************/

    /************ ANOTHER SERVER SOCKET SETUP ************/

    print_log("----- Start Server -----\n");
    
    /* all networked programs must create a socket */
    if ((ssl_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        SSL_CTX_free(ssl_context);
        print_log("[ERROR]: Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    print_log("[INFO]: Creating ssl socket %d\n", ssl_sock);
    

    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    int opt = 1;  
    if( setsockopt(ssl_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        print_log("[ERROR]: set sockopt fail!\n"); 
        return EXIT_FAILURE; 
    } 

    /*************************************************************/
    /* Set socket to be nonblocking. All of the sockets for      */
    /* the incoming connections will also be nonblocking since   */
    /* they will inherit that state from the listening socket.   */
    /*************************************************************/

    rc = ioctl(ssl_sock, FIONBIO, (char *)&opt);
    if (rc < 0)
    {
        print_log("[ERROR]: ioctl() failed");
        close(ssl_sock);
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTPS_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    print_log("HTTPS_port:%d\n", HTTPS_port);

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(ssl_sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(ssl_sock);
        SSL_CTX_free(ssl_context);
        print_log("[ERROR]: Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(ssl_sock, 5))
    {
        close_socket(master_sock);
        SSL_CTX_free(ssl_context);
        print_log("[ERROR]: Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    /************ END ANOTHER SERVER SOCKET SETUP ************/


    /************ SERVER SOCKET SETUP ************/

    print_log("----- Start Server -----\n");
    
    /* all networked programs must create a socket */
    if ((master_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        SSL_CTX_free(ssl_context);
        print_log("[ERROR]: Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    print_log("[INFO]: Creating master socket %d\n", master_sock);
    

    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    int opt_master = 1;  
    if( setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_master, 
          sizeof(opt_master)) < 0 )  
    {  
        print_log("[ERROR]: set sockopt fail!\n"); 
        return EXIT_FAILURE; 
    } 

    /*************************************************************/
    /* Set socket to be nonblocking. All of the sockets for      */
    /* the incoming connections will also be nonblocking since   */
    /* they will inherit that state from the listening socket.   */
    /*************************************************************/

    
    rc = ioctl(master_sock, FIONBIO, (char *)&opt_master);
    if (rc < 0)
    {
        print_log("[ERROR]: ioctl() failed");
        close(master_sock);
        exit(-1);
    }
    

    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(HTTP_port);
    master_addr.sin_addr.s_addr = INADDR_ANY;

    print_log("HTTP_port:%d\n", HTTP_port);

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(master_sock, (struct sockaddr *) &master_addr, sizeof(master_addr)))
    {
        close_socket(master_sock);
        SSL_CTX_free(ssl_context);
        print_log("[ERROR]: Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(master_sock, 5))
    {
        close_socket(master_sock);
        SSL_CTX_free(ssl_context);
        print_log("[ERROR]: Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    

    /************ END SERVER SOCKET SETUP ************/

    /* finally, loop waiting for input and then write it back */
    //set of socket descriptors 
    fd_set working_set, master_set, ssl_set;  
    int max_sd = master_sock > ssl_sock ? master_sock : ssl_sock;
    int max_fd, max_fd_ssl;
    max_fd = master_sock;
    max_fd_ssl = ssl_sock;
    //int maxIdx = 0; // store the max non empty slot 
    int rv, new_sock;
    FD_ZERO(&working_set);
    FD_ZERO(&master_set);
    FD_ZERO(&ssl_set);
    FD_SET(master_sock, &master_set);
    FD_SET(ssl_sock, &master_set);

    print_log("max_sd:%d\n", max_sd);

    while (1)
    {
        FD_ZERO(&working_set);
        for(int i = 0;i<=max_sd;i++){
            if(FD_ISSET(i, &master_set) || FD_ISSET(i, &ssl_set)){
                FD_SET(i, &working_set);
            }
        }

        //FD_ZERO(&working_set);
        //memcpy(&working_set, &master_set, sizeof(master_set));

        //FD_SET(master_sock, &readfds);
        print_log("[INFO]: Start select!\n");

        rv = select(max_sd+1, &working_set, NULL, NULL, NULL);
        print_log("[INFO]: After select!\n");
        if(rv == -1){
            print_log("[ERROR]: select");
        }

        for(int i = 0;i<=max_sd;i++){
            if(FD_ISSET(i, &working_set)){
                //set_fl(i, O_NONBLOCK);
                // creating new connection
                if( i == ssl_sock ){
                    //fprintf(stdout, "New connection!\n");
                    //do{
                        //fprintf(stdout, "Start receiving new sock!\n");
                        new_sock = accept(ssl_sock, (struct sockaddr *) &cli_addr,
                                    &cli_size);
                        print_log("[INFO]: new sock ssl %d\n", new_sock);
                        if(new_sock == -1){
                            print_log("[ERROR]: Connect fail!\n");
                            SSL_CTX_free(ssl_context);
                            return EXIT_FAILURE;
                        }
                        FD_SET(new_sock, &ssl_set);
                        max_sd = new_sock>max_sd?new_sock:max_sd;

                        max_fd_ssl = new_sock > max_fd_ssl ? new_sock:max_fd_ssl;
                        
                        //isSSL[new_sock] = 1;
                    //}while(new_sock!=-1);

                    //no need multiple ssl_ctx SSL INIT
                    //if(ssl_init(ssl_context) != 0){
                    //    print_log("[ERROR]: initializing ssl_context");
                    //}

                    /************ WRAP SOCKET WITH SSL ************/
                    if ((client_context = SSL_new(ssl_context)) == NULL)
                    {
                        close(master_sock);
                        SSL_CTX_free(ssl_context);
                        print_log("Error creating client SSL context.\n");
                        return EXIT_FAILURE;
                    }

                    if (SSL_set_fd(client_context, new_sock) == 0)
                    {
                        close(master_sock);
                        SSL_free(client_context);
                        SSL_CTX_free(ssl_context);
                        print_log("Error creating client SSL context.\n");
                        return EXIT_FAILURE;
                    }  

                    if (SSL_accept(client_context) <= 0)
                    {
                        close(master_sock);
                        SSL_free(client_context);
                        SSL_CTX_free(ssl_context);
                        print_log("Error accepting (handshake) client SSL context.\n");
                        return EXIT_FAILURE;
                    }

                    all_ssl_context[new_sock] = client_context;
                    /************ END WRAP SOCKET WITH SSL ************/

                }else if(i == master_sock){
                    new_sock = accept(master_sock, (struct sockaddr *) &cli_addr,
                                    &cli_size);
                    print_log("[INFO]: new sock normal %d\n", new_sock);
                    if(new_sock == -1){
                        print_log("[ERROR]: Connect fail!\n");
                        SSL_CTX_free(ssl_context);
                        return EXIT_FAILURE;
                    }
                    FD_SET(new_sock, &master_set);
                    max_sd = new_sock>max_sd?new_sock:max_sd;
                    //isSSL[new_sock] = 1;
                    max_fd = new_sock > max_fd ? new_sock : max_fd;
                }
                else if(FD_ISSET(i, &ssl_set)){

                    client_context = all_ssl_context[i];

                    //fprintf(stdout, "Here!!!in else\n");
                    //exsiting connection
                    readret = 0;

                    // read all data
                    //allbuf = malloc(sizeof(char)*BUF_SIZE);
                    int buflen = 0;
                    char *tmpbuf;
                    memset(buf, 0, BUF_SIZE);
                    readret = SSL_read(client_context, buf, BUF_SIZE);
                    //SSL_write(client_context, buf, readret);
                    /*while((readret = SSL_read(client_context, buf, BUF_SIZE)) > 0){
                        //buflen += readret;
                        if(buflen ==0){
                            allbuf = malloc(sizeof(char)*(buflen+readret+1));
                        }else{
                            tmpbuf = malloc(sizeof(char)*(buflen+readret+1));
                            memcpy(tmpbuf, allbuf, buflen);
                            allbuf = tmpbuf;
                        }

                        memcpy(allbuf+buflen, buf, readret);
                        buflen += readret;
                        SSL_write(client_context, buf, readret);
                        //sendall(client_context, buf, readret);
                        print_log("[DEBUG]: test1\n");
                        memset(buf, 0, BUF_SIZE);
                    }*/

                    print_log("[DEBUG]: test2\n");
                    //SSL_write(client_context, allbuf, buflen);

                    if(readret > 0 )
                    {
                        //SSL_write(client_context, allbuf, buflen);
                        print_log("[DEBUG]: test3\n");
                        print_log("[INFO]: Start sending msg\n");
                        char * respond_buf;
                        readret = handle_request(buf, readret, i, &respond_buf);
                        //print_log("respond:\n%s\n", respond_buf);
                        //respond_buf[readret] = '\0';
                        print_log("sending :\n");
                        for(int ii = 0;ii<readret;ii++){
                            print_log("%c", respond_buf[ii]);
                        }
                        print_log("\n");
                        //SSL_write(client_context, allbuf, buflen);
                        int sendret = sendall(client_context, respond_buf, readret);
                        if (sendret != readret)
                        {
                            //close_socket(i);
                            FD_CLR(i, &ssl_set);
                            if(i == max_fd_ssl){
                                while(FD_ISSET(max_fd_ssl, &ssl_set) == 0){
                                    max_fd_ssl--;
                                }
                            }
                            max_sd = max_fd_ssl > max_fd ? max_fd_ssl : max_fd;
                            print_log("[INFO] Sending bytes:%d to client.\n", sendret);
                            SSL_shutdown(client_context);
                            SSL_free(client_context);
                            close_socket(i);
                            //close_socket(master_sock);
                            //SSL_CTX_free(ssl_context);
                            //fprintf(stderr, "Error sending to client.\n");
                            return EXIT_FAILURE;
                        }
                        //close_socket(i);
                        //free(respond_buf);
                        print_log("[INFO]: Sending size %ld\n", readret);
                    }
                    else{
                        close_socket(i);
                        FD_CLR(i, &ssl_set);
                        if(i == max_fd_ssl){
                            while(FD_ISSET(max_fd_ssl, &ssl_set) == 0){
                                max_fd_ssl--;
                            }
                        }
                        max_sd = max_fd_ssl > max_fd ? max_fd_ssl : max_fd;
                        print_log("[INFO]: close connection!\n");
                        

                        //SSL_shutdown(client_context);
                        //SSL_free(client_context);
                        
                        //SSL_CTX_free(ssl_context);
                        print_log("Error reading from client socket.\n");
                        if(readret == -1){
                            close_socket(master_sock);
                            print_log("[ERROR]: Error reading from client socket.\n");
                            return EXIT_FAILURE;
                        }
                    }
                }else{
                    // normal connection without ssl
                    //fprintf(stdout, "Here!!!in else\n");
                    //exsiting connection
                    print_log("[INFO]: In normal socket\n");
                    readret = 0;

                    int buflen = 0;
                    char *tmpbuf;
                    readret = recv(i, buf, BUF_SIZE, 0);
                    /*
                    while((readret = recv(i, buf, BUF_SIZE, 0)) > 0){
                        //buflen += readret;
                        if(buflen ==0){
                            allbuf = malloc(sizeof(char)*(buflen+readret+1));
                        }else{
                            tmpbuf = malloc(sizeof(char)*(buflen+readret+1));
                            memcpy(tmpbuf, allbuf, buflen);
                            allbuf = tmpbuf;
                        }

                        memcpy(allbuf+buflen, buf, readret);
                        buflen += readret;
                    }*/

                    if(readret > 0)
                    {
                        print_log("[INFO]: Start sending msg\n");
                        char * respond_buf;
                        readret = handle_request(buf, readret, i, &respond_buf);
                        print_log("[INFO]: readret: %d\n", readret);
                        if (sendall_nossl(i, respond_buf, readret) != readret)
                        {
                            
                            close_socket(i);
                            FD_CLR(i, &master_set);
                            if(i == max_fd){
                                while(FD_ISSET(max_fd, &master_set) == 0){
                                    max_fd--;
                                }
                            }
                            max_sd = max_fd > max_fd_ssl ? max_fd : max_fd_ssl;
                            print_log("[ERROR] Error sending to client.\n");
                            
                            //fprintf(stderr, "Error sending to client.\n");
                            //return EXIT_FAILURE;
                        }
                        //free(respond_buf);
                        print_log("[INFO]: Sending size %ld\n", readret);
                        
                    }else{
                        close_socket(i);
                        FD_CLR(i, &master_set);
                        if(i == max_fd){
                            while(FD_ISSET(max_fd, &master_set) == 0){
                                max_fd--;
                            }
                        }
                        max_sd = max_fd > max_fd_ssl ? max_fd: max_fd_ssl;
                        print_log("[INFO]: close connection!\n");
                        if(readret == -1){
                            print_log("[ERROR]: Error reading from client socket.\n");
                            return EXIT_FAILURE;
                        }
                    }
                    //memset(buf, 0, BUF_SIZE);
                    //close the connection!
                    
                    
                }
            }
        }

    }

    for(int i = 0;i<max_sd;i++){
        close_socket(i);
    }


    return EXIT_SUCCESS;
}
