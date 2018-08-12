#include "handle.h"

void display_chunks(data_t *data){
  for(int i = 0;i<data->chunks_num;i++){
    DPRINTF(DEBUG_INIT, "dusplay_chunks:%s\n", data->chunks[i].hash);
  }
}

void reset_empty(data_t * data){
  data->lastAck = 0;
  data->lastAvailable = data->lastAck + data->window_size;
  data->lastSent = 0;
  data->lastAckCount = 0;
  data->lastAckSent = 0;
  memset(data->recvedpPkg, 0, sizeof(int)*(data->maxAvailable+1));
}

void initial_data(data_t *data, char *has_chunk_file){

  FILE *f = fopen(has_chunk_file, "r");
  char line[CHUNK_LINE_SIZE+10];
  data->has_chunks_num = 0;
  while (fgets(line, CHUNK_LINE_SIZE, f) != NULL) {
    if (line[0] == '#') continue;
    data->has_chunks_num++;
  }
  fclose(f);
  data->has_chunks = malloc(sizeof(struct Chunk)*(data->has_chunks_num+1));

  f = fopen(has_chunk_file,"r");
  //rewind(f);
  DPRINTF(DEBUG_INIT, "chunks_num:%d\n", data->has_chunks_num);
  
  int count = 0;
  while (fgets(line, CHUNK_LINE_SIZE, f) != NULL) {
    DPRINTF(DEBUG_INIT,"read line:%s", line);
    if (line[0] == '#') continue;
    assert(sscanf(line, "%d %s", &data->has_chunks[count].id, data->has_chunks[count].hash) != 0);
    DPRINTF(DEBUG_INIT, "id:%d hash:%s\n", data->has_chunks[count].id, data->has_chunks[count].hash);
    count++;
    
  }


  data->state = INITIAL;
  data->window_size = 1;  //initial is set to 1
  data->lastAck = 0;
  data->lastAvailable = data->lastAck + data->window_size;
  data->lastSent = 0;
  data->maxAvailable = 512*1024/(1500-16) + 1;
  data->lastAckCount = 0;
  data->peer2Idx = NULL;
  data->lastAckSent = 0;
  data->getChunkIdx = 0;

  data->ssthresh = 64;
  data->ccstate = SLOW_START;


  data->getChunk = malloc(sizeof(unsigned char)*(1500));

  data->recvedpPkg = malloc(sizeof(int)*(data->maxAvailable+1));
  memset(data->recvedpPkg, 0, sizeof(int)*(data->maxAvailable+1));

  fclose(f);

  //data->recvedAck = malloc(sizeof(int)*(data->maxAvailable+1));
  //memset(data->recvedAck, 0, sizeof(int)*(data->maxAvailable+1));
}

void process_get(char *chunkfile, char *outputfile, void *data_void) {
  FILE *f;
  f = fopen(chunkfile,"r");
  char line[CHUNK_LINE_SIZE];
  data_t * data = (data_t*)data_void;

  //save outputfile
  data->output_file = malloc(sizeof(char)*strlen(outputfile));
  memcpy(data->output_file, outputfile, strlen(outputfile));

  data->chunks_num = 0;
  while (fgets(line, CHUNK_LINE_SIZE, f) != NULL) {
    
    if (line[0] == '#') continue;
    data->chunks_num++;
  }

  //rewind(f);
  //fclose(f);

  //FILE *f1;
  f = fopen(chunkfile,"r");
  data->chunks = malloc(sizeof(struct Chunk)*(data->chunks_num+1));
  
  
  int count = 0;
  while (fgets(line, CHUNK_LINE_SIZE, f) != NULL) {
    DPRINTF(DEBUG_INIT,"read line:%s", line);
    if (line[0] == '#') continue;
    assert(sscanf(line, "%d %s", &data->chunks[count].id, data->chunks[count].hash) != 0);
    DPRINTF(DEBUG_INIT, "id:%d hash:%s\n", data->chunks[count].id, data->chunks[count].hash);
    count++;
  }
  DPRINTF(DEBUG_INIT, "Before closing file\n");
  DPRINTF(DEBUG_INIT, "Close file\n");

  //change state
  data->state = READY_TO_WHOHAS;

  //create target data array
  DPRINTF(DEBUG_INIT, "chunks_num:%d\n", data->chunks_num);
  

  if(data->targetData == NULL){
    DPRINTF(DEBUG_INIT, "Error malloc!\n");
  }

  data->targetData = malloc((data->chunks_num)*512*1024);

  //fclose(f);
}

void write_to_newfile(data_t * data){
  FILE *f = fopen(data->output_file, "wb");
  fwrite(data->targetData, data->getChunkNum*512*1024, 1, f);
  fclose(f);
}

//increase window size by one
void increase_wsz(data_t *data){
  if(data->state == SLOW_START){
    data->window_size++;
    data->lastAvailable = data->lastAck+data->window_size < data->maxAvailable ? data->lastAck+data->window_size:data->maxAvailable;
    if(data->window_size == data->ssthresh){
      data->state = CONGESTION_AVOIDANCE;
      data->nextAck = data->lastAck + data->window_size;
      
    }
  }else{
    if(data->lastAck == data->nextAck){
      data->window_size++;
      data->lastAvailable = data->lastAck+data->window_size < data->maxAvailable ? data->lastAck+data->window_size:data->maxAvailable;
      data->nextAck = data->lastAck + data->window_size;
    }
  }
  
}