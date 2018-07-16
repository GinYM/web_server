/*********************************************************************************
* handle_request.c                                                               *
*                                                                                *
* Description: This file contains the C source code for handling request.        *
*              Get the Request parsed by lex and yacc parser, send respond       *                                   
*                                                                                *
* Authors: Yiming Jin <jinyiming1996@gmail.com>                                  *
*                                                                                *
*********************************************************************************/



#include "handle_request.h"

void status_line(Request* request, char* status_code, char* buf){
    //char buf[4096];
    buf[0] = '\0';
    //http-version
    strcat(buf, request->http_version);
    strcat(buf, " ");
    strcat(buf, status_code);
    strcat(buf, " ");
    strcat(buf, "\r\n");
    //print_log("buf in status_line:%s\n", buf);
    return;
}

//wrapper for strcat, dynamically malloc buf1
int cat_str(char**buf1, char *buf2, int capacity){
    if(strlen(*buf1) + strlen(buf2) >= capacity){
        capacity = (strlen(buf1) + strlen(buf2))*2+1;
        char *buf = malloc(sizeof(char)*((strlen(buf1) + strlen(buf2))*2+1));
        memcpy(buf, buf1, strlen(buf1)+1);
        *buf1 = buf;
    }
    strcat(*buf1, buf2);
    return capacity;
}

int handle_get(Request* request, char**buf){
    //int fd_in = open(request->, O_RDONLY);
    char *file_name;
    char *status;
    //char *buf;
    int capacity = 4096;
    *buf = malloc(sizeof(char)*capacity);
    int size = 0;
    (*buf)[0] = '\0';
    file_name = malloc(sizeof(char)*4096);
    file_name[0] = '\0';
    if(strcmp(request->http_uri, "/")==0){
        strcat(file_name, "./www/index.html");
        //status = "200";
    }else{
        strcat(file_name, "./www");
        strcat(file_name, request->http_uri);
    }

    FILE *fp;
    fp = fopen(file_name,"r");
    if(fp == NULL){
        status = "404";
    }else{
        status = "200";
    }
    //status line
    char str_status_line[4096];
    status_line(request, status, str_status_line);
    print_log("status_line:%s\n", str_status_line);
    capacity = cat_str(buf, str_status_line, capacity);
    
    if(strcmp(status,"200") == 0){
        char content[8192];
        while(fgets(content, 1000, fp) != NULL){
            capacity = cat_str(buf, content, capacity);
        }
        fclose(fp);
        
    }
    capacity = cat_str(buf, "\r\n", capacity);
    print_log("[DEBUG] handle_get: %s\n", *buf);
    return strlen(*buf);

}

int respond_error(char *buffer, int size, int socketFd, char **resp_buf){
  //Read from the file the sample
  Request *request = parse(buffer,size,socketFd);
  //Just printing everything
  status_line(request, "503", resp_buf);

  int capacity = 4096;
  
  capacity = cat_str(resp_buf, "\r\n", capacity);
  //printf("Http Method %s\n",request->http_method);
  //printf("Http Version %s\n",request->http_version);
  //printf("Http Uri %s\n",request->http_uri);
  //printf("header_count: %d\n", request->header_count);
  
  //printf("Finished parsing!\n");
  free(request->headers);
  free(request);
  return strlen(resp_buf);
}


int handle_request(char *buffer, int size, int socketFd, char **resp_buf){
  //Read from the file the sample
  Request *request = parse(buffer,size,socketFd);
  //Just printing everything
  int buf_size = 0;
  if(strcmp(request->http_method,"GET") == 0){
      buf_size = handle_get(request, resp_buf);
  }
  print_log("Http Method %s\n",request->http_method);
  print_log("Http Version %s\n",request->http_version);
  print_log("Http Uri %s\n",request->http_uri);
  print_log("header_count: %d\n", request->header_count);
  for(int index = 0;index < request->header_count;index++){
    print_log("Request Header\n");
    print_log("Header name %s Header Value %s\n",request->headers[index].header_name,request->headers[index].header_value);
  }
  print_log("Finished parsing!\n");
  free(request->headers);
  free(request);
  print_log("[DEBUG] handle_request resp_buf:%s\n", *resp_buf);
  return buf_size;
}
