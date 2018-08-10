/*
 * peer.c
 *
 * Authors: Ed Bardsley <ebardsle+441@andrew.cmu.edu>,
 *          Dave Andersen
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2.
 *
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "spiffy.h"
#include "bt_parse.h"
#include "input_buffer.h"
#include "handle.h"
#include "chunk.h"
#include <assert.h>

void peer_run(bt_config_t *config);



int main(int argc, char **argv) {
  bt_config_t config;

  bt_init(&config, argc, argv);

  DPRINTF(DEBUG_INIT, "peer.c main beginning\n");

#ifdef TESTING
  config.identity = 1; // your group number here
  strcpy(config.chunk_file, "chunkfile");
  strcpy(config.has_chunk_file, "haschunks");
#endif

  bt_parse_command_line(&config);

#ifdef DEBUG
  if (debug & DEBUG_INIT) {
    bt_dump_config(&config);
  }
#endif
  
  peer_run(&config);
  return 0;
}

void showString(char *s, int length){
  for(int i = 0;i<length;i++){
    DPRINTF(DEBUG_INIT, "%c", s[i]);
  }
  DPRINTF(DEBUG_INIT, "\n");
}

void showBytes(unsigned char *s, int length){
  for(int i = 0;i<length;i++){
    DPRINTF(DEBUG_INIT, "%u", s[i]);
  }
  DPRINTF(DEBUG_INIT, "\n");
}

int isEqual(char *s1, char *s2, int length){
  //DPRINTF(DEBUG_INIT,"Compare:\n");
  showString(s1, length);
  showString(s2, length);
  for(int i = 0;i<length;i++){
    if(s1[i] != s2[i]){
      return 0;
    }
  }
  return 1;
}

void sendData(int sock, bt_config_t *config, data_t *data, struct sockaddr_in * from, socklen_t fromlen){
  FILE*f = fopen(config->chunk_file,"r");
  char master_chunk[CHUNK_LINE_SIZE];
  fgets(master_chunk, CHUNK_LINE_SIZE, f);
  //chunk name, strip \n
  master_chunk[strcspn(master_chunk, "\n")] = '\0';
  //DPRINTF(DEBUG_INIT, "Open file:%s\n", master_chunk+6);
  fclose(f);
  f = fopen(master_chunk+6, "rb");
  if(f == NULL){
    fprintf("Error opening:%s\n", master_chunk+6);
  }
  int header_len = 16;
  int sent_len = 0;
  long filelen;
  int packet_len = 1500;
  int maxpacket_len = 1500;
  int total_len = 512*1024;
  unsigned char * sendmsg = malloc(sizeof(unsigned char)*packet_len);
  int seq_num = 0;
  fill_msg_header(sendmsg);
  //type DATA
  sendmsg[3] = 3;
  while(data->lastSent < data->lastAvailable){
    sent_len = data->lastSent*(1500-16);
    data->lastSent++;
    seq_num = data->lastSent;
    DPRINTF(DEBUG_INIT, "Send seq_num:%d\n", seq_num);
    
    //DPRINTF(DEBUG_INIT, "Sending seq_num:%d\n", seq_num);
    fseek(f,(long)data->reqDataId*512*1024+sent_len,SEEK_SET);
    //filelen = ftell(f);
    //rewind(f);
    if(data->lastSent != data->maxAvailable){
      fread(sendmsg+16,maxpacket_len-16, 1, f);
      packet_len = maxpacket_len;
    }else{
      sendmsg[6] = ((total_len-sent_len)>>8) & 0xFF;
      sendmsg[7] = (total_len - sent_len) & 0xFF;
      packet_len = total_len - sent_len + 16;
      fread(sendmsg+16,total_len - sent_len, 1, f);
      
    }
    assert(packet_len <= 1500);

    //seq_num
    sendmsg[8] = (seq_num >> 24) & 0xFF;
    sendmsg[9] = (seq_num >> 16) & 0xFF;
    sendmsg[10] = (seq_num >> 8) & 0xFF;
    sendmsg[11] = seq_num & 0xFF;

    //DPRINTF(DEBUG_INIT, "While Send: \n");

    //showBytes(sendmsg+16, packet_len-16);

    spiffy_sendto(sock, sendmsg, packet_len, 0, from, fromlen);

    //debug
    //break;
    
  }
  fclose(f);
}

void process_inbound_udp(int sock, bt_config_t *config, data_t * data) {
  #define BUFLEN 1500
  struct sockaddr_in from;
  socklen_t fromlen;
  unsigned char buf[BUFLEN];
  char msg[BUFLEN*2+1];

  fromlen = sizeof(from);
  int size = spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);

  binary2hex(buf,size,msg);

  //showString(msg, size);

  /*
  printf("PROCESS_INBOUND_UDP SKELETON -- replace!\nIncoming size:%d\n"
	 "Incoming message from %s:%d\n%s\n\n", size,
	 inet_ntoa(from.sin_addr),
	 ntohs(from.sin_port),
	 msg);
   */
  int type = msg[7]-'0';
  int chunk_num = (msg[32]-'0')*16 + (msg[33]-'0');
  if(type <= 1)
    DPRINTF(DEBUG_INIT, "type:%d, chunk_num:%d\n", type, chunk_num);
  else{
    //DPRINTF(DEBUG_INIT, "Receive type:%d\n", type);
  }
  int chunk_idx = 40;

  if(type == 0){
    reset_empty(data);
    // whohas
    FILE*f = fopen(config->has_chunk_file, "r");
    char line[CHUNK_LINE_SIZE];
    int id;
    char hash[50];
    unsigned char * sendmsg = malloc(sizeof(unsigned char)*(20+20*chunk_num));
    fill_msg_ihave_header(sendmsg);
    int count_hash = 0;
    while (fgets(line, CHUNK_LINE_SIZE, f) != NULL) {
      
      if (line[0] == '#') continue;
      assert(sscanf(line, "%d %s", &id, hash) != 0);
      DPRINTF( DEBUG_INIT, "read from has_chunk_file, id:%d, hash:%s\n", id, hash);
      for(int i = 0;i<chunk_num;i++){
        if(isEqual(hash, msg+chunk_idx+i*40,40)){
          append_chunk_hash(sendmsg, hash, count_hash++);
        }
      }
    }
    //update packet_len
    int packet_len = 20+20*count_hash;
    sendmsg[6] = (packet_len>>8)&0xFF;
    sendmsg[7] = packet_len & 0xFF;

    //update chunk_num
    sendmsg[16] = count_hash;
    DPRINTF(DEBUG_INIT, "Send to :%s:%d, count_hash:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port), count_hash);
    if(count_hash > 0){
      spiffy_sendto(sock, sendmsg, packet_len, 0, &from, fromlen);
    }
  }
  //send GET
  if(type == 1){
    reset_empty(data);
    //data->ihaveMsg = malloc(sizeof(char)*(size*2+1));
    //memcpy(data->ihaveMsg, msg, size*2);
    int packet_len = 16+20;
    unsigned char * sendmsg = malloc(sizeof(unsigned char)*packet_len);
    fill_msg_header(sendmsg);
    //PACKET len
    sendmsg[6] = (packet_len>>8) & 0xFF;
    sendmsg[7] = packet_len & 0xFF;
    //type 
    sendmsg[3] = 2;
    data->getChunkNum = chunk_num;
    memcpy(data->getChunk, buf+20, 20*chunk_num);
    data->getChunkIdx = 0;
    memcpy(sendmsg+16, data->getChunk+data->getChunkIdx*20, 20);
    spiffy_sendto(sock, sendmsg, packet_len, 0, &from, fromlen);
    
  }

  //Send Data, After GET
  if(type == 2){
    reset_empty(data);
    char hash[50] ;
    memcpy(hash, msg+32,40);
    DPRINTF(DEBUG_INIT, "GET %s\n", hash);
    int id=-1;
    for(int i = 0;i<data->chunks_num;i++){
      if(isEqual(hash, data->chunks[i].hash,40)){
        id = data->chunks[i].id;
        break;
      }
    }
    if(id == -1){
      printf("Error, No such file!\n");
      return;
    }
    DPRINTF(DEBUG_INIT, "id:%d\n", id);

    //here we initial reqDataId
    data->reqDataId = id;

    sendData(sock, config, data, &from, fromlen);
    
  }

  //DATA
  if(type == 3){
    int seq_num = 0;
    for(int i = 16;i<16+8;i++){
      //DPRINTF(DEBUG_INIT,"Seq Num:%c\n", msg[i]);
      if(msg[i] >= '0' && msg[i] <= '9'){
        seq_num += (msg[i]-'0')<<(4*(16+8-i-1));
      }else{
        seq_num += ( (int)(msg[i]-'a') + 10)<<(4*(16+8-i-1));
      }
      
    }
    //DPRINTF(DEBUG_INIT, "msg:%s\n", msg);
    DPRINTF(DEBUG_INIT, "Get seq_num:%d, size:%d\n", seq_num, size);

    //store data update recved seq_num
    memcpy(data->targetData+data->getChunkIdx*(512*1024)+(seq_num-1)*(1500-16),buf+16, size-16);
    data->recvedpPkg[seq_num] = 1;
    //send ACK
    int packet_len = 16;
    unsigned char * sendmsg = malloc(sizeof(unsigned char)*packet_len);
    fill_msg_header(sendmsg);
    sendmsg[6] = (packet_len>>8) & 0xFF;
    sendmsg[7] = packet_len & 0xFF;
    
    //type
    sendmsg[3] = 4;
    
    
    
    //memcpy(data->targetData + , buf+16, size-16);
    if(data->lastAckSent >= seq_num-1 ){
      DPRINTF(DEBUG_INIT, "Sending:%d\n", seq_num);
      sendmsg[12] = (seq_num >> 24) & 0xFF;
      sendmsg[13] = (seq_num >> 16) & 0xFF;
      sendmsg[14] = (seq_num >> 8) & 0xFF;
      sendmsg[15] = seq_num & 0xFF;
      
      while(data->lastAckSent < data->maxAvailable && data->recvedpPkg[data->lastAckSent+1] == 1){
        data->lastAckSent++;
      }
    }else{
      sendmsg[12] = (data->lastAckSent >> 24) & 0xFF;
      sendmsg[13] = (data->lastAckSent >> 16) & 0xFF;
      sendmsg[14] = (data->lastAckSent >> 8) & 0xFF;
      sendmsg[15] = data->lastAckSent & 0xFF;
    }

    //send ACK
    spiffy_sendto(sock, sendmsg, packet_len, 0, &from, fromlen);

    DPRINTF(DEBUG_INIT, "Receiver: lastAckSent:%d\n", data->lastAckSent);

    //check whether is Finished
    if(data->lastAckSent == data->maxAvailable){
      //Finished
      DPRINTF(DEBUG_INIT, "Finished for chunk:%d\n", data->getChunkIdx);
      if(data->getChunkIdx >= data->getChunkNum-1){
        DPRINTF(DEBUG_INIT, "Finished All\n");
      }else{
        //Request New chunk
        int packet_len = 16+20;
        unsigned char * sendmsg = malloc(sizeof(unsigned char)*packet_len);
        fill_msg_header(sendmsg);
        //PACKET len
        sendmsg[6] = (packet_len>>8) & 0xFF;
        sendmsg[7] = packet_len & 0xFF;
        //type 
        sendmsg[3] = 2;
        //data->getChunkNum = chunk_num;
        //memcpy(data->getChunk, buf+20, 20*chunk_num);
        data->getChunkIdx++;
        memcpy(sendmsg+16, data->getChunk+data->getChunkIdx*20, 20);
        spiffy_sendto(sock, sendmsg, packet_len, 0, &from, fromlen);
        reset_empty(data);
        //reset to zero
      }
    }

  }

  //recv data
  if(type == 4){
    //int ackNum  = (msg[24]-'0')<<
    int ack_num = 0;
    for(int i = 24;i<24+8;i++){
      if(msg[i] >= '0' && msg[i] <= '9'){
        ack_num += (msg[i]-'0')<<(4*(24+8-i-1));
      }else{
        ack_num += (msg[i]-'a'+10)<<(4*(24+8-i-1));
      }
      
    }

    if(ack_num > data->lastAvailable || ack_num > data->lastSent){
      DPRINTF(DEBUG_INIT, "Old Msg\n");
      return;
    }
    
    if(ack_num != data->lastAck){
      data->lastAckCount = 1;
    }else{
      data->lastAckCount++;
    }
    //DPRINTF(DEBUG_INIT, "Recv ACK:%d LastACK Count:%d\n", ack_num, data->lastAckCount);
    data->lastAck = ack_num > data->lastAck ? ack_num : data->lastAck;
    data->lastAvailable = data->lastAck+data->window_size < data->maxAvailable ? data->lastAck+data->window_size:data->maxAvailable;
    
    

    if(data->lastAckCount >= 3){
      data->lastSent = data->lastAck;
      printf("Error!\n");
      exit(-1);
    }

    DPRINTF(DEBUG_INIT, "ReqDataId:%d lastAck:%d ack_num:%d LastACK Count:%d lastAvailable:%d maxAvailable:%d lastSent:%d\n", data->reqDataId, data->lastAck, ack_num,  data->lastAckCount, data->lastAvailable, data->maxAvailable, data->lastSent);

    sendData(sock, config, data, &from, fromlen);
  }

}

void fill_msg_ihave_header(unsigned char *msg){
  msg[0] = (15441>>8)&0xFF;
  msg[1] = 15441 & 0xFF;
  msg[2] = 1;
  msg[3] = 1;
  //header len
  msg[4] = (16>>8) & 0xFF;
  msg[5] = 16 & 0xFF;

  //packet len
  int packet_len = 0;
  msg[6] = (packet_len>>8)&0xFF;
  msg[7] = packet_len & 0xFF;

  //invalid from 8 to 15

  //chunk num
  msg[16] = 0;
  //padding 17,18,19

  //chunk hash
}

void fill_msg_header(unsigned char *msg){
  msg[0] = (15441>>8)&0xFF;
  msg[1] = 15441 & 0xFF;
  msg[2] = 1;
  msg[3] = 1;
  //header len
  msg[4] = (16>>8) & 0xFF;
  msg[5] = 16 & 0xFF;

  //packet len
  int packet_len = 0;
  msg[6] = (packet_len>>8)&0xFF;
  msg[7] = packet_len & 0xFF;

  //invalid from 8 to 15
  for(int i = 8;i<=15;i++){
    msg[i] = 0;
  }
}

// append the hash_idx(th) hash to msg
void append_chunk_hash(unsigned char *msg, char *hash, int hash_idx){
  ascii2hex(hash, 40, msg+20+hash_idx*20);
}

void fill_msg_whohas(unsigned char *msg, data_t *data){
  msg[0] = (15441>>8)&0xFF;
  msg[1] = 15441 & 0xFF;
  msg[2] = 1;
  msg[3] = 0;
  //header len
  msg[4] = (16>>8) & 0xFF;
  msg[5] = 16 & 0xFF;

  //packet len
  int packet_len = 5*4+20*data->chunks_num;
  msg[6] = (packet_len>>8)&0xFF;
  msg[7] = packet_len & 0xFF;

  //invalid from 8 to 15

  //chunk num
  msg[16] = data->chunks_num;
  //padding 17,18,19

  //chunk hash
  for(int i = 0;i<data->chunks_num;i++){
    DPRINTF(DEBUG_INIT, "data->chunks hash:%s\n", data->chunks[i].hash);
    ascii2hex(data->chunks[i].hash,40,msg+20+i*20);
  }

}


void process_send_whohas(int sock, bt_config_t *config, data_t *data){
  DPRINTF(DEBUG_INIT, "Here in process_send_whohas\n");
  struct bt_peer_s * peer = config->peers;
  int tolen = sizeof(struct sockaddr_in);
  unsigned char *msg = malloc(sizeof(char)*(20+data->chunks_num*20));
  fill_msg_whohas(msg, data);
  int idx = 0;
  char address[50];
  while(peer != NULL){
    peerDataIdx_t *p2Idx = malloc(sizeof(peerDataIdx_t));
    //char address[50];
    p2Idx->id = 0;
    
    memcpy(p2Idx->address, inet_ntoa(peer->addr.sin_addr), strlen(inet_ntoa(peer->addr.sin_addr)));
    //memcpy(address+strlen(inet_ntoa(peer->addr.sin_addr)), ntohs(peer->addr.sin_port), strlen(ntohs(peer->addr.sin_port)));
    p2Idx->port = ntohs(peer->addr.sin_port);
    //DPRINTF(DEBUG_INIT, "Addr:%s\n", address);
    p2Idx->next = data->peer2Idx;
    data->peer2Idx = p2Idx;
    

    DPRINTF(DEBUG_INIT, "Addr:%s, port:%d\n", data->peer2Idx->address, data->peer2Idx->port);

    
    spiffy_sendto(sock, msg, (20+data->chunks_num*20), 0, &peer->addr, tolen);
    peer = peer->next;
  }
  data->state = READY_TO_RECV;
}



void handle_user_input(char *line, void *cbdata, void *data) {
  char chunkf[128], outf[128];

  bzero(chunkf, sizeof(chunkf));
  bzero(outf, sizeof(outf));

  if (sscanf(line, "GET %120s %120s", chunkf, outf)) {
    if (strlen(outf) > 0) {
      process_get(chunkf, outf, data);
    }
  }
}


void peer_run(bt_config_t *config) {
  int sock;
  struct sockaddr_in myaddr;
  fd_set readfds;
  struct user_iobuf *userbuf;
  
  if ((userbuf = create_userbuf()) == NULL) {
    perror("peer_run could not allocate userbuf");
    exit(-1);
  }
  
  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
    perror("peer_run could not create socket");
    exit(-1);
  }
  
  bzero(&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(config->myport);
  
  if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
    perror("peer_run could not bind socket");
    exit(-1);
  }
  
  spiffy_init(config->identity, (struct sockaddr *)&myaddr, sizeof(myaddr));

  //initial data
  struct Data data;
  initial_data(&data);
  

  while (1) {
    int nfds;
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);
    
    nfds = select(sock+1, &readfds, NULL, NULL, NULL);
    
    if (nfds > 0) {
      if (FD_ISSET(sock, &readfds)) {
	      process_inbound_udp(sock, config, &data);
      }
      
      if (FD_ISSET(STDIN_FILENO, &readfds)) {
	        process_user_input(STDIN_FILENO, userbuf, handle_user_input,
			   "Currently unused", &data);
          process_send_whohas(sock, config, &data);
      }
    }
  }
}
