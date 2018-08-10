#include "handle.h"

void display_chunks(data_t *data){
  for(int i = 0;i<data->chunks_num;i++){
    DPRINTF(DEBUG_INIT, "dusplay_chunks:%s\n", data->chunks[i].hash);
  }
}

void initial_data(data_t *data){
  data->state = INITIAL;
  data->window_size = 8;
  data->lastAck = 0;
  data->lastAvailable = data->lastAck + data->window_size;
  data->lastSent = 0;
  data->maxAvailable = 512*1024/(1500-16) + 1;
  data->lastAckCount = 0;
  data->peer2Idx = NULL;
  data->lastAckSent = 0;


  data->getChunk = malloc(sizeof(unsigned char)*(1500));

  data->recvedpPkg = malloc(sizeof(int)*(data->maxAvailable+1));
  memset(data->recvedpPkg, 0, sizeof(int)*(data->maxAvailable+1));

  //data->recvedAck = malloc(sizeof(int)*(data->maxAvailable+1));
  //memset(data->recvedAck, 0, sizeof(int)*(data->maxAvailable+1));
}

void process_get(char *chunkfile, char *outputfile, void *data_void) {
  FILE *f;
  f = fopen(chunkfile,"r");
  char line[CHUNK_LINE_SIZE];
  data_t * data = (data_t*)data_void;
  data->chunks_num = 0;
  while (fgets(line, CHUNK_LINE_SIZE, f) != NULL) {
    
    if (line[0] == '#') continue;
    data->chunks_num++;
  }

  if(fclose(f)){
      printf("Closing failed!\n");
  }
  

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
  data->targetData = malloc(sizeof(unsigned char)*data->chunks_num*512*1024);
}