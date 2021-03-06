#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>
#include "debug.h"
#include<string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


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
    FINISHED_GET,
    SENDING_MSG,
    FINISHED_SENDING
};

//congestion control state
enum CC_State{
    SLOW_START,
    CONGESTION_AVOIDANCE
};

//this part is only for retransmit after time out
int prev_sock;
struct sockaddr_in *prev_from;
socklen_t prev_fromlen;

struct Data{
    chunk_t * chunks;
    chunk_t * has_chunks;
    char* dstFile;
    int chunks_num;
    int has_chunks_num;
    enum State state;
    unsigned char * targetData;
    

    int lastAckCount;

    // for peer which want to download files from other peer
    int reqDataId;
    peerDataIdx_t * peer2Idx;
    unsigned char*getChunk; // chunk that needed to fetch
    int getChunkIdx; // current Fetch Chunk
    int getChunkNum; // total number of chunk to get from one peer
    char *output_file;
    int lastAckSent;
    int *recvedpPkg;

    // for peer sending files
    int window_size;
    int lastAck;
    int lastSent;
    int lastAvailable;
    int maxAvailable;
    int ssthresh;
    enum CC_State ccstate;
    int nextAck;
    
};



typedef struct Data data_t;

void process_get(char *chunkfile, char *outputfile, void *data);
void initial_data(data_t *data, char* has_chunk_file);
void reset_empty(data_t * data);
void write_to_newfile(data_t * data);
void increase_wsz(data_t *data);
void handle_timeout(data_t *data);
void print_message_function (int sock, int t, int ws);
