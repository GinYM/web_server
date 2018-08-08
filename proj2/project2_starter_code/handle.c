#include "handle.h"

void display_chunks(data_t *data){
  for(int i = 0;i<data->chunks_num;i++){
    DPRINTF(DEBUG_INIT, "dusplay_chunks:%s\n", data->chunks[i].hash);
  }
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
}