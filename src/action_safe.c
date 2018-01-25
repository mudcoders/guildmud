/*
 * This file handles non-fighting player actions.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"

void cmd_say(D_MOBILE *dMob, char *arg)
{
  if (arg[0] == '\0')
  {
    text_to_mobile(dMob, "Say what?\n\r");
    return;
  }
  communicate(dMob, arg, COMM_LOCAL);
}

void cmd_quit(D_MOBILE *dMob, char *arg)
{
  char buf[MAX_BUFFER];

  /* log the attempt */
  snprintf(buf, MAX_BUFFER, "%s has left the game.", dMob->name);
  log_string(buf);

  save_player(dMob);

  dMob->socket->player = NULL;
  free_mobile(dMob);
  close_socket(dMob->socket, FALSE);
}

void cmd_shutdown(D_MOBILE *dMob, char *arg)
{
  shut_down = TRUE;
}

void cmd_commands(D_MOBILE *dMob, char *arg)
{
  BUFFER *buf = buffer_new(MAX_BUFFER);
  int i, col = 0;

  bprintf(buf, "    - - - - ----==== The full command list ====---- - - - -\n\n\r");
  for (i = 0; tabCmd[i].cmd_name[0] != '\0'; i++)
  {
    if (dMob->level < tabCmd[i].level) continue;

    bprintf(buf, " %-16.16s", tabCmd[i].cmd_name);
    if (!(++col % 4)) bprintf(buf, "\n\r");
  }
  if (col % 4) bprintf(buf, "\n\r");
  text_to_mobile(dMob, buf->data);
  buffer_free(buf);
}

void cmd_who(D_MOBILE *dMob, char *arg)
{
  D_MOBILE *xMob;
  D_SOCKET *dsock;
  ITERATOR Iter;
  BUFFER *buf = buffer_new(MAX_BUFFER);

  bprintf(buf, " - - - - ----==== Who's Online ====---- - - - -\n\r");

  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    if (dsock->state != STATE_PLAYING) continue;
    if ((xMob = dsock->player) == NULL) continue;

    bprintf(buf, " %-12s   %s\n\r", xMob->name, dsock->hostname);
  }
  DetachIterator(&Iter);

  bprintf(buf, " - - - - ----======================---- - - - -\n\r");
  text_to_mobile(dMob, buf->data);

  buffer_free(buf);
}

void cmd_help(D_MOBILE *dMob, char *arg)
{
  if (arg[0] == '\0')
  {
    HELP_DATA *pHelp;
    ITERATOR Iter;
    BUFFER *buf = buffer_new(MAX_BUFFER);
    int col = 0;

    bprintf(buf, "      - - - - - ----====//// HELP FILES  \\\\\\\\====---- - - - - -\n\n\r");

    AttachIterator(&Iter, help_list);
    while ((pHelp = (HELP_DATA *) NextInList(&Iter)) != NULL)
    {
      bprintf(buf, " %-19.18s", pHelp->keyword);
      if (!(++col % 4)) bprintf(buf, "\n\r");
    }
    DetachIterator(&Iter);

    if (col % 4) bprintf(buf, "\n\r");
    bprintf(buf, "\n\r Syntax: help <topic>\n\r");
    text_to_mobile(dMob, buf->data);
    buffer_free(buf);

    return;
  }

  if (!check_help(dMob, arg))
    text_to_mobile(dMob, "Sorry, no such helpfile.\n\r");
}

void cmd_compress(D_MOBILE *dMob, char *arg)
{
  /* no socket, no compression */
  if (!dMob->socket)
    return;

  /* enable compression */
  if (!dMob->socket->out_compress)
  {
    text_to_mobile(dMob, "Trying compression.\n\r");
    text_to_buffer(dMob->socket, (char *) compress_will2);
    text_to_buffer(dMob->socket, (char *) compress_will);
  }
  else /* disable compression */
  {
    if (!compressEnd(dMob->socket, dMob->socket->compressing, FALSE))
    {
      text_to_mobile(dMob, "Failed.\n\r");
      return;
    }
    text_to_mobile(dMob, "Compression disabled.\n\r");
  }
}

void cmd_save(D_MOBILE *dMob, char *arg)
{
  save_player(dMob);
  text_to_mobile(dMob, "Saved.\n\r");
}

void cmd_copyover(D_MOBILE *dMob, char *arg)
{ 
  FILE *fp;
  ITERATOR Iter;
  D_SOCKET *dsock;
  char buf[MAX_BUFFER];
  
  if ((fp = fopen(COPYOVER_FILE, "w")) == NULL)
  {
    text_to_mobile(dMob, "Copyover file not writeable, aborted.\n\r");
    return;
  }

  strncpy(buf, "\n\r <*>            The world starts spinning             <*>\n\r", MAX_BUFFER);

  /* For each playing descriptor, save its state */
  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    compressEnd(dsock, dsock->compressing, FALSE);

    if (dsock->state != STATE_PLAYING)
    {
      text_to_socket(dsock, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r");
      close_socket(dsock, FALSE);
    }
    else
    {
      fprintf(fp, "%d %s %s\n",
        dsock->control, dsock->player->name, dsock->hostname);

      /* save the player */
      save_player(dsock->player);

      text_to_socket(dsock, buf);
    }
  }
  DetachIterator(&Iter);

  fprintf (fp, "-1\n");
  fclose (fp);

  /* close any pending sockets */
  recycle_sockets();
  
  /*
   * feel free to add any additional arguments between the 2nd and 3rd,
   * that is "SocketMud" and buf, but leave the last three in that order,
   * to ensure that the main() function can parse the input correctly.
   */
  snprintf(buf, MAX_BUFFER, "%d", control);
  execl(EXE_FILE, "SocketMud", buf, "copyover", (char *) NULL);

  /* Failed - sucessful exec will not return */
  text_to_mobile(dMob, "Copyover FAILED!\n\r");
}

void cmd_linkdead(D_MOBILE *dMob, char *arg)
{
  D_MOBILE *xMob;
  ITERATOR Iter;
  char buf[MAX_BUFFER];
  bool found = FALSE;

  AttachIterator(&Iter, dmobile_list);
  while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
  {
    if (!xMob->socket)
    {
      snprintf(buf, MAX_BUFFER, "%s is linkdead.\n\r", xMob->name);
      text_to_mobile(dMob, buf);
      found = TRUE;
    }
  }
  DetachIterator(&Iter);

  if (!found)
    text_to_mobile(dMob, "Noone is currently linkdead.\n\r");
}
