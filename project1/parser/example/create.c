#include<stdio.h>

int main(){
    FILE *f = fopen("sample_request_test", "w");
    if(f == NULL){
        printf("Error opening file!\n");
        return 1;
    }
    const char * text = "GET /~prs/15-441-F15/ HTTP/1.1\r\nstatus: keepalive\r\nabc: 123\r\n\r\n123";
    fprintf(f, "%s", text);
    fclose(f);
    return 0;
}
