#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>
#include "debug.h"
#include<string.h>

#define CHUNK_LINE_SIZE 500

struct Chunk{
    int id;
    char hash[50];
};


//peer ip to data id
struct peerDataIdx{
    int id;
    char address[50];
    int port;
    struct peerDataIdx * next;
};

typedef struct Chunk chunk_t;
typedef struct peerDataIdx peerDataIdx_t;

enum State{
    INITIAL,
    READY_TO_WHOHAS,
    READY_TO_RECV,
    SEND_IHAVE,
    GET_NEXT_CHUNK,
    FINISHED_GET
};

struct Data{
    chunk_t * chunks;
    chunk_t * has_chunks;
    char* dstFile;
    int chunks_num;
    int has_chunks_num;
    enum State state;
    unsigned char * targetData;
    int window_size;
    int lastAck;
    int lastSent;
    int lastAvailable;
    int maxAvailable;

    int lastAckCount;

    int reqDataId;
    peerDataIdx_t * peer2Idx;
    unsigned char*getChunk; // chunk that needed to fetch
    int getChunkIdx; // current Fetch Chunk
    int getChunkNum; // total number of chunk to get from one peer

    int lastAckSent;
    int *recvedpPkg;
    char *output_file;
    
};



typedef struct Data data_t;

void process_get(char *chunkfile, char *outputfile, void *data);
void initial_data(data_t *data, char* has_chunk_file);
void reset_empty(data_t * data);
void write_to_newfile(data_t * data);