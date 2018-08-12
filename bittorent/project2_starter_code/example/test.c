#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    FILE *f = fopen("/media/gin/hacker/web_server/proj2/project2_starter_code/example/C.tar\n","rb");
    fseek(f, 0, SEEK_SET);
    unsigned char msg[5];
    fread(msg, 4, 1, f);
    printf("%s\n", msg);
    
}
