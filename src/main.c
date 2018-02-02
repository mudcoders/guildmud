/*
 * This file contains the main code.
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

/* including main header file */
#include "mud.h"

/*
 * This is where it all starts, nothing special.
 */
int main(int argc, char **argv)
{
  bool fCopyOver;

  /* get the current time */
  current_time = time(NULL);

  /* allocate memory for socket and mobile lists'n'stacks */
  dsock_free = AllocStack();
  dsock_list = AllocList();
  dmobile_free = AllocStack();
  dmobile_list = AllocList();

  /* note that we are booting up */
  log_string("Program starting.");

  /* first and foremost, run the migrations */
  db_migrate();

  /* initialize the event queue - part 1 */
  init_event_queue(1);

  if (argc > 2 && !strcmp(argv[argc-1], "copyover") && atoi(argv[argc-2]) > 0)
  {
    fCopyOver = TRUE;
    control = atoi(argv[argc-2]);
  }
  else fCopyOver = FALSE;

  /* initialize the socket */
  if (!fCopyOver)
    control = init_socket();

  /* load all external data */
  load_muddata(fCopyOver);

  /* initialize the event queue - part 2*/
  init_event_queue(2);

  /* main game loop */
  GameLoop(control);

  /* close down the socket */
  close(control);

  /* terminated without errors */
  log_string("Program terminated without errors.");

  /* and we are done */
  return 0;
}
