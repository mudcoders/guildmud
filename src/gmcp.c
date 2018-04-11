/*
 * gmcp.c - support functions for GMCP
 *
 * see https://www.gammon.com.au/gmcp
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "mud.h"

const unsigned char gmcp_head [] = { IAC, SB, TELOPT_GMCP, 0 };
const unsigned char gmcp_tail [] = { IAC, SE, 0 };

bool gmcpEnable(D_S *dsock)
{
  dsock->gmcp_enabled = true;
  gmcpSend(dsock, "Test data");
  return dsock->gmcp_enabled;
}

bool gmcpSend(D_S *dsock, const char *data)
{
  if (!dsock->gmcp_enabled)
    return false;

  text_to_socket(dsock, (char *)  gmcp_head);
  text_to_socket(dsock, (char *)  data);
  text_to_socket(dsock, (char *)  gmcp_tail);
  return true;
}

void gmcpReceived(D_S *dsock)
{
  /* TODO: Extend this with something useful */
}
