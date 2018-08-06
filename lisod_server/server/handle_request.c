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
int cat_str(char**buf1, int* buf_len,  char *buf2, int buf2_len, int capacity){
    if(*buf_len + buf2_len >= capacity){
        capacity = (*buf_len + buf2_len)*2+1;
        char *buf = malloc(sizeof(char)*((*buf_len + buf2_len)*2+1));
        //*buf_len = *buf_len + buf2_len;
        memcpy(buf, *buf1, *buf_len);
        *buf1 = buf;
    }
    //strcat(*buf1, buf2);
    memcpy(*buf1+*buf_len,buf2, buf2_len);
    *buf_len = *buf_len + buf2_len;
    return capacity;
}

char * getGMT(){
    time_t rawtime;
    struct tm * timeinfo;
    static char buffer [80];

    time (&rawtime);
    timeinfo = gmtime (&rawtime);

    strftime (buffer,80,"%a, %d %b %G %T GMT",timeinfo);
    return buffer;
}

char* getSuffix(char *str){
    static char sub[128];
    int idx = strlen(str)-1;
    while(idx>=0 && str[idx]!='\0'){
        if(str[idx] == '.'){
            break;
        }
        idx--;
    }
    sub[0] = '\0';
    if(idx == -1){
        return sub;
    }else{
        memcpy(sub, &str[idx+1], strlen(str)-1-(idx+1) + 1+1);
        return sub;
    }
}

//used for debug
void show(char *buf, int buf_len){
    for(int i = 0;i<buf_len;i++){
        printf("%c", buf[i]);
    }
    printf("\n");
}

int handle_get(Request* request, char**buf){
    //int fd_in = open(request->, O_RDONLY);
    char *file_name;
    char *status;
    //char *buf;
    int capacity = 4096;
    *buf = malloc(sizeof(char)*capacity);
    int buf_len = 0;
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
    long content_length = 0;
    if(fp == NULL){
        status = "404";
    }else{
        status = "200";
        fseek (fp , 0 , SEEK_END);
        content_length = ftell (fp);
        rewind (fp);
    }
    //status line
    char str_status_line[4096];
    status_line(request, status, str_status_line);
    print_log("status_line:%s\n", str_status_line);
    
    capacity = cat_str(buf, &buf_len, str_status_line, strlen(str_status_line), capacity);
    
    //general header
    //connection
    capacity = cat_str(buf, &buf_len, "Connection: keep-alive\r\n", 24, capacity);
    //printf("%c%c\n", buf[0], buf[1]);
    //show(*buf, buf_len);
    //date
    char *date_buf = getGMT();
    print_log("[DEBUG] Current date:%s\n", date_buf);
    capacity = cat_str(buf, &buf_len,  "Date: ",  6, capacity);
    capacity = cat_str(buf, &buf_len, date_buf, strlen(date_buf), capacity);
    capacity = cat_str(buf, &buf_len, "\r\n", 2, capacity);

    //response header
    capacity = cat_str(buf, &buf_len, "Server: Liso/1.0\r\n", 18 ,capacity);

    //printf("Buf: %s\n", buf);

    

    //int content_length = 0;
    char *content;
    int cap = 8192;
    content = malloc(sizeof(char)*(content_length+2));

    //show(*buf, buf_len);
    
    //content[0] = '\0';
    //printf("request->http_method : %s\n", request->http_method);
    //printf("status: %s\n", status);
    if(strcmp(status,"200") == 0 && strcmp(request->http_method,"GET") == 0 ){
        //char content_tmp[1001];
        //if(fgets(content, 1000, fp) != NULL){
        //    content_length += strlen(content);
        //}else{
        //    content_length = 0;
        //}
        int result;
        //printf("[DEBUG] real content_length:%d\n", content_length);
        result = fread (content,1,content_length,fp);
        //content[result] = '\0';
        if(result != content_length){
            print_log("[ERROR] Reading error!\n");
        }
        
        print_log("[DEBUG] real content_length:%d, resut:%d\n", content_length, result);
        
    }

    if(fp!=NULL){
        fclose(fp);
    }
    

    //entity-header
    char content_length_str[1024];
    sprintf(content_length_str, "Content-Length: %ld\r\n", content_length);
    
    capacity = cat_str(buf, &buf_len, content_length_str, strlen(content_length_str), capacity);

    // Content-Type   = "Content-Type" ":" media-type
    char* subfix = getSuffix(file_name);
    //printf("subfix:%s\n", subfix);
    if(strcmp(subfix, "html")==0 || strcmp(subfix, "css")==0){
        capacity = cat_str(buf, &buf_len, "Content-Type: text/", 19,capacity);
        capacity = cat_str(buf, &buf_len, subfix, strlen(subfix) ,capacity);
        capacity = cat_str(buf, &buf_len, "\r\n", 2, capacity);
    }else if(strcmp(subfix, "png") == 0 ){
        capacity = cat_str(buf, &buf_len, "Content-Type: image/", 20, capacity);
        capacity = cat_str(buf, &buf_len, subfix, strlen(subfix), capacity);
        capacity = cat_str(buf, &buf_len, "\r\n", 2, capacity);
    }else if(strcmp(subfix, "jpg")==0 || strcmp(subfix, "jpeg") == 0){
        capacity = cat_str(buf, &buf_len, "Content-Type: image/jpeg\r\n", 26, capacity);
        //capacity = cat_str(buf, "\r\n", capacity);
    }else{
        capacity = cat_str(buf, &buf_len, "Content-Type: text/plain\r\n", 12, capacity);
    }

    //Last-Modified
    struct tm *foo;
    struct stat attrib;
    char lastModifiedDate[128];
    if(strcmp(status, "200") == 0){
        stat(file_name, &attrib);
        foo = gmtime(&(attrib.st_mtime));
        strftime (lastModifiedDate,128,"%a, %d %b %G %T GMT",foo);
        capacity = cat_str(buf, &buf_len, "Last-Modified: ", 15, capacity);
        capacity = cat_str(buf, &buf_len, lastModifiedDate, strlen(lastModifiedDate), capacity);
        capacity = cat_str(buf, &buf_len, "\r\n", 2, capacity);
    }
    //printf("Here??");
    
    

    capacity = cat_str(buf, &buf_len, "\r\n", 2, capacity);

    //message-body
    //printf("error! 404aa\n");
    if(strcmp(request->http_method,"GET") == 0)
        capacity = cat_str(buf, &buf_len, content, content_length, capacity);
    else if(strcmp(request->http_method, "POST") == 0){
        capacity = cat_str(buf, &buf_len, request->body.data, request->body.length, capacity);
    }
    //capacity = cat_str(buf, &buf_len, "\r\n", 2, capacity);
    //show(*buf, buf_len);
    print_log("[DEBUG] handle_get: %s\n", *buf);
    return buf_len;

}

int respond_error(char *buffer, int size, int socketFd, char **resp_buf){
  //Read from the file the sample
  Request *request = parse(buffer,size,socketFd);
  //Just printing everything
  status_line(request, "503", resp_buf);

  int capacity = 4096;
  int buf_len = strlen(*resp_buf);
  capacity = cat_str(resp_buf, &buf_len, "\r\n", 2, capacity);
  //printf("Http Method %s\n",request->http_method);
  //printf("Http Version %s\n",request->http_version);
  //printf("Http Uri %s\n",request->http_uri);
  //printf("header_count: %d\n", request->header_count);
  
  //printf("Finished parsing!\n");
  free(request->headers);
  free(request);
  return buf_len;
}




int handle_request(char *buffer, int size, int socketFd, char **resp_buf, int *isCGI, struct Pool * p, int fd){
  //Read from the file the sample
  Request *request = parse(buffer,size,socketFd);
  //Just printing everything
  int buf_size = 0;

  char preFix[6];
  memcpy(preFix, request->http_uri, 5);
  if(strcmp(preFix, "/cgi/") == 0){
      *isCGI = 1;
      return 0;
  }

  if(strcmp(request->http_method,"GET") == 0 || strcmp(request->http_method,"HEAD") == 0 || strcmp(request->http_method, "POST") == 0){
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
  print_log("Body: %s\n", request->body.data);
  print_log("Finished parsing!\n");
  p[fd].request = request;

  

  return buf_size;
}
