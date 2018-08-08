#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>
#include "debug.h"

#define CHUNK_LINE_SIZE 50

struct Chunk{
    int id;
    char hash[50];
};

typedef struct Chunk chunk_t;

enum State{
    INITIAL,
    READY_TO_WHOHAS,
    READY_TO_RECV,
    SEND_IHAVE
};

struct Data{
    chunk_t * chunks;
    char* dstFile;
    int chunks_num;
    enum State state;
};



typedef struct Data data_t;

void process_get(char *chunkfile, char *outputfile, void *data);