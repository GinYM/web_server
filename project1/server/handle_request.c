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

char* status_line(Request* request, char* status_code){
    char buf[4096];
    buf[0] = '\0';
    //http-version
    strcat(buf, request->http_version);
    strcat(buf, " ");
    strcat(buf, status_code);
    strcat(buf, " ");
    strcat(buf, "\r\n");
    return buf;
}

//wrapper for strcat, dynamically malloc buf1
int cat_str(char*buf1, char *buf2, int capacity){
    if(strlen(buf1) + strlen(buf2) >= capacity){
        capacity = (strlen(buf1) + strlen(buf2))*2+1;
        char *buf = malloc(sizeof(char)*((strlen(buf1) + strlen(buf2))*2+1));
        
        memcpy(buf, buf1, strlen(buf1)+1);
    }
    strcat(buf1, buf2);
    return capacity;
}

int handle_get(Request* request, char*buf){
    int fd_in = open(request->, O_RDONLY);
    char *file_name;
    char *status;
    //char *buf;
    int capacity = 4096;
    buf = malloc(sizeof(char)*capacity);
    int size = 0;
    buf[0] = '\0'
    if(strcmp(request->http_uri, "/")){
        file_name = "../www/index.html";
        status = "200";
    }else{
        status = "404";
    }
    //status line

    char* str_status_line = status_line(request, status);
    if(strlen(buf) + strlen() )
    capacity = cat_str(buf, str_status_line, capacity);
    FILE *fp;
    if(strcmp(status,"200") == 0){
        char content[8192];
        fp = fopen(file_name,"r");
        if(fp!=NULL){
            while(fgets(content, 1000, fp) != NULL){
                capacity = cat_str(buf, content, capacity);
            }
            fclose(fp);
        }
    }
    capacity = cat_str(buf, "\r\n", capacity);
    return strlen(buf);

}

int respond_error(char *buffer, int size, int socketFd, char *resp_buf){
  //Read from the file the sample
  Request *request = parse(buf,readRet,fd_in);
  //Just printing everything
  resp_buf = status_line(request, "503");

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


int handle_request(char *buffer, int size, int socketFd, char *resp_buf){
  //Read from the file the sample
  Request *request = parse(buf,readRet,fd_in);
  //Just printing everything
  int buf_size = 0;
  if(strcmp(request->http_method,"GET")){
      buf_size = handle_get(request, resp_buf);
  }
  print_log("Http Method %s\n",request->http_method);
  print_log("Http Version %s\n",request->http_version);
  print_log("Http Uri %s\n",request->http_uri);
  print_log("header_count: %d\n", request->header_count);
  for(index = 0;index < request->header_count;index++){
    print_log("Request Header\n");
    print_log("Header name %s Header Value %s\n",request->headers[index].header_name,request->headers[index].header_value);
  }
  print_log("Finished parsing!\n");
  free(request->headers);
  free(request);
  return buf_size;
}
