#include "parse.h"

/**
* Given a char buffer returns the parsed request headers
*/
Request * parse(char *buffer, int size, int socketFd) {
  //Differant states in the state machine
	enum {
		STATE_START = 0, STATE_CR, STATE_CRLF, STATE_CRLFCR, STATE_CRLFCRLF
	};

	int i = 0, state;
	size_t offset = 0;
	char ch;
	char buf[8192];
	memset(buf, 0, 8192);

	//printf("buffer %s\n", buffer);

	state = STATE_START;
	while (state != STATE_CRLFCRLF) {
		char expected = 0;

		//printf("Here!\n");
		//printf("STATE:%d\n", state);

		if (i == size)
			break;

		ch = buffer[i++];
		buf[offset++] = ch;
		printf("buf[%d]:%c\n", offset-1, buf[offset-1]);

		switch (state) {
		case STATE_START:
		case STATE_CRLF:
			expected = '\r';
			break;
		case STATE_CR:
		case STATE_CRLFCR:
			expected = '\n';
			break;
		default:
			state = STATE_START;
			continue;
		}

		if (ch == expected)
			state++;
		else
			state = STATE_START;

	}

	//printf("i:%d size:%d\n", i, size);

	//message body
	if(state == STATE_CRLFCRLF && !(i == size-1 && buffer[i] == '\0')){
		
		while(i != size){
			ch = buffer[i++];
			buf[offset++] = ch;
			printf("buf[%d]:%c %d\n", offset-1, buf[offset-1], (int)(buf[offset-1]));
		}
	}

	// remove \0
	while(i>=1 && buf[i-1] == '\0'){
		i--;
		printf("remove buf[%d]: %c", i, buf[i]);
	}

	// strip only \n
	if(i-1 >= 0 && buf[i-1] == '\n' && (i-2 < 0 || (i-2 >=0 && buf[i-2]!='\r') )){
		i--;
	}

  //Valid End State
	if (state == STATE_CRLFCRLF) {
		//printf("Valid end state\n");
		Request *request = (Request *) malloc(sizeof(Request));
    request->header_count=0;
		request->header_capacity = 1;
		request->body.capacity = 4096;
		request->body.length = 0;
		request->body.data = malloc(sizeof(char)*4096);
		//request->suc = 1;
    //TODO You will need to handle resizing this in parser.y
		//printf("Buf: %s\n", buf);
		printf("last one: %c\n", buf[i-1]);
    request->headers = (Request_header *) malloc(sizeof(Request_header)*1);
		set_parsing_options(buf, i, request);

		if (yyparse() == SUCCESS) {
      return request;
		}
	}
  //TODO Handle Malformed Requests
  printf("Parsing Failed\n");
	return NULL;

}
