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
int sendall(int client_sock_name, char *buf, ssize_t content_size)
{
    if(content_size > BUF_SIZE){
        print_log("[ERROR] No moew than BUF_SIZE:%d, current:%ld", BUF_SIZE, content_size);
    }
    ssize_t left = content_size;
    ssize_t ret_size = 0;
    while(left > 0){
        print_log("[DEBUUG] Sending :%s\n", buf);
        ret_size = send(client_sock_name, buf, left, 0);
        buf += ret_size;
        left -= ret_size;
        
    }
    return 0;
        
}

//introduction of select https://www.mkssoftware.com/docs/man3/select.3.asp
int start_server()
//int main(int argc, char* argv[])
{
    int master_sock, rc;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    //int client_sock[MAX_CLIENT_NUM];

    print_log("----- Start Server -----\n");
    
    /* all networked programs must create a socket */
    if ((master_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        print_log("[ERROR]: Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    print_log("[INFO]: Creating master socket %d\n", master_sock);
    

    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    int opt = 1;  
    if( setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
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

    rc = ioctl(master_sock, FIONBIO, (char *)&opt);
    if (rc < 0)
    {
        print_log("[ERROR]: ioctl() failed");
        close(master_sock);
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(master_sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(master_sock);
        print_log("[ERROR]: Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(master_sock, 5))
    {
        close_socket(master_sock);
        print_log("[ERROR]: Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    /* finally, loop waiting for input and then write it back */
    //set of socket descriptors 
    fd_set working_set, master_set;  
    int max_sd = master_sock;
    //int maxIdx = 0; // store the max non empty slot 
    int rv, new_sock;
    FD_ZERO(&working_set);
    FD_ZERO(&master_set);
    FD_SET(master_sock, &master_set);
    while (1)
    {

        //FD_ZERO(&working_set);
        memcpy(&working_set, &master_set, sizeof(master_set));

        //FD_SET(master_sock, &readfds);
        print_log("[INFO]: Start select!\n");

        rv = select(max_sd+1, &working_set, NULL, NULL, NULL);
        print_log("[INFO]: After select!\n");
        if(rv == -1){
            print_log("[ERROR]: select");
        }

        for(int i = 0;i<=max_sd;i++){
            if(FD_ISSET(i, &working_set)){
                // creating new connection
                if(i == master_sock){
                    //fprintf(stdout, "New connection!\n");
                    //do{
                        //fprintf(stdout, "Start receiving new sock!\n");
                        new_sock = accept(master_sock, (struct sockaddr *) &cli_addr,
                                    &cli_size);
                        print_log("[INFO]: new sock %d\n", new_sock);
                        if(new_sock == -1){
                            print_log("[ERROR]: Connect fail!\n");
                            return EXIT_FAILURE;
                        }
                        FD_SET(new_sock, &master_set);
                        max_sd = new_sock>max_sd?new_sock:max_sd;
                    //}while(new_sock!=-1);
                }else{
                    //fprintf(stdout, "Here!!!in else\n");
                    //exsiting connection
                    readret = 0;

                    if((readret = recv(i, buf, BUF_SIZE, 0)) >= 1)
                    {
                        print_log("[INFO]: Start sending msg\n");
                        char * respond_buf;
                        readret = handle_request(buf, readret, i, &respond_buf);
                        if (sendall(i, respond_buf, readret) != readret)
                        {
                            
                            close_socket(i);
                            FD_CLR(i, &master_set);
                            if(i == max_sd){
                                while(FD_ISSET(max_sd, &master_set) == 0){
                                    max_sd--;
                                }
                            }
                            print_log("[ERROR] Error sending to client.\n");
                            
                            //fprintf(stderr, "Error sending to client.\n");
                            //return EXIT_FAILURE;
                        }
                        //free(respond_buf);
                        print_log("[INFO]: Sending size %ld\n", readret);
                        
                    }
                    memset(buf, 0, BUF_SIZE);
                    //close the connection!
                    
                    if (readret == 0 || readret == -1)
                    {
                        close_socket(i);
                        FD_CLR(i, &master_set);
                        if(i == max_sd){
                            while(FD_ISSET(max_sd, &master_set) == 0){
                                max_sd--;
                            }
                        }
                        print_log("[INFO]: close connection!");
                        if(readret == -1){
                            print_log("[ERROR]: Error reading from client socket.\n");
                            return EXIT_FAILURE;
                        }
                        
                    }
                }
            }
        }

    }

    for(int i = 0;i<max_sd;i++){
        close_socket(i);
    }


    return EXIT_SUCCESS;
}
