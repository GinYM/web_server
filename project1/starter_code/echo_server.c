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

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096
#define MAX_CLIENT_NUM 1000

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

//introduction of select https://www.mkssoftware.com/docs/man3/select.3.asp

int main(int argc, char* argv[])
{
    int master_sock, rc;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    //int client_sock[MAX_CLIENT_NUM];

    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((master_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Creating master socket %d\n", master_sock);
    

    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    int opt = 1;  
    if( setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        fprintf(stderr, "set sockopt fail!\n"); 
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
        perror("ioctl() failed");
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
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(master_sock, 5))
    {
        close_socket(master_sock);
        fprintf(stderr, "Error listening on socket.\n");
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
        fprintf(stdout, "Start select!\n");

        rv = select(max_sd+1, &working_set, NULL, NULL, NULL);
        fprintf(stdout,"After select!\n");
        if(rv == -1){
            perror("select");
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
                        fprintf(stdout, "new sock %d\n", new_sock);
                        if(new_sock == -1){
                            perror("Connect fail!\n");
                            return EXIT_FAILURE;
                        }
                        FD_SET(new_sock, &master_set);
                        max_sd = new_sock>max_sd?new_sock:max_sd;
                    //}while(new_sock!=-1);
                }else{
                    fprintf(stdout, "Here!!!in else\n");
                    //exsiting connection
                    readret = 0;

                    if((readret = recv(i, buf, BUF_SIZE, 0)) >= 1)
                    {
                        fprintf(stdout, "Start sending msg\n");
                        if (send(i, buf, readret, 0) != readret)
                        {
                            
                            close_socket(i);
                            FD_CLR(i, &master_set);
                            if(i == max_sd){
                                while(FD_ISSET(max_sd, &master_set) == 0){
                                    max_sd--;
                                }
                            }
                            //fprintf(stderr, "Error sending to client.\n");
                            return EXIT_FAILURE;
                        }
                        fprintf(stdout, "Sending size %ld\n", readret);
                        memset(buf, 0, BUF_SIZE);
                    }
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
                        fprintf(stdout, "close connection!");
                        if(readret == -1){
                            fprintf(stderr, "Error reading from client socket.\n");
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
