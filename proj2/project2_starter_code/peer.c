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

int isEqual(char *s1, char *s2, int length){
  DPRINTF(DEBUG_INIT,"Compare:\n");
  showString(s1, length);
  showString(s2, length);
  for(int i = 0;i<length;i++){
    if(s1[i] != s2[i]){
      return 0;
    }
  }
  return 1;
}


void process_inbound_udp(int sock, bt_config_t *config) {
  #define BUFLEN 1500
  struct sockaddr_in from;
  socklen_t fromlen;
  unsigned char buf[BUFLEN];
  char msg[BUFLEN];

  fromlen = sizeof(from);
  int size = spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);

  binary2hex(buf,size,msg);

  printf("PROCESS_INBOUND_UDP SKELETON -- replace!\nIncoming size:%d\n"
	 "Incoming message from %s:%d\n%s\n\n", size,
	 inet_ntoa(from.sin_addr),
	 ntohs(from.sin_port),
	 msg);
  int type = msg[7]-'0';
  int chunk_num = (msg[32]-'0')*16 + (msg[33]-'0');
  DPRINTF(DEBUG_INIT, "type:%d, chunk_num:%d\n", type, chunk_num);
  int chunk_idx = 40;

  if(type == 0){
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
  //GET
  if(type == 1){
    int packet_len = 16+20;
    unsigned char * sendmsg = malloc(sizeof(unsigned char)*packet_len);
    fill_msg_header(sendmsg);
    //PACKET len
    sendmsg[6] = (packet_len>>8) & 0xFF;
    sendmsg[7] = packet_len & 0xFF;
    //type 
    sendmsg[3] = 2;
    for(int i = 0;i<chunk_num;i++){
      for(int j = 0;j<20;j++){
        sendmsg[16+j] = buf[20+i*20+j];
      }
      DPRINTF(DEBUG_INIT, "Sending: %s\n", sendmsg);
      spiffy_sendto(sock, sendmsg, packet_len, 0, &from, fromlen);
    }
    
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
  struct bt_peer_s * peer = config->peers;
  int tolen = sizeof(struct sockaddr_in);
  unsigned char *msg = malloc(sizeof(char)*(20+data->chunks_num*20));
  fill_msg_whohas(msg, data);
  while(peer != NULL){
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
  struct Data data;
  data.state = INITIAL;
  while (1) {
    int nfds;
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);
    
    nfds = select(sock+1, &readfds, NULL, NULL, NULL);
    
    if (nfds > 0) {
      if (FD_ISSET(sock, &readfds)) {
	      process_inbound_udp(sock, config);
      }
      
      if (FD_ISSET(STDIN_FILENO, &readfds)) {
	        process_user_input(STDIN_FILENO, userbuf, handle_user_input,
			   "Currently unused", &data);
          process_send_whohas(sock, config, &data);
      }
    }
  }
}
