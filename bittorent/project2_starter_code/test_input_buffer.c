#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "input_buffer.h"
#include "handle.h"

void printline(char *line, void *cbdata, void *data) {
  printf("LINE:  %s\n", line);
  printf("CBDATA:  %s\n", (char *)cbdata);
}


int main() {

  
  struct user_iobuf *u;

  u = create_userbuf();
  assert(u != NULL);

  data_t data;

  while (1) {
    process_user_input(STDIN_FILENO, u, printline, "Cows moo!", &data);
  }
  
  return 0;
}
