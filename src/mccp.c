/*
 * mccp.c - support functions for the Mud Client Compression Protocol
 *
 * see http://www.randomly.org/projects/MCCP/
 *
 * Copyright (c) 1999, Oliver Jowett <oliver@randomly.org>
 *
 * This code may be freely distributed and used if this copyright
 * notice is retained intact.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mud.h"

/* local functions */
bool  processCompressed       ( D_SOCKET *dsock );

const unsigned char enable_compress  [] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, 0 };
const unsigned char enable_compress2 [] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, 0 };

/*
 * Memory management - zlib uses these hooks to allocate and free memory
 * it needs
 */
void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
  return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
  free(address);
}

/*
 * Begin compressing data on `desc'
 */
bool compressStart(D_SOCKET *dsock, unsigned char teleopt)
{
  z_stream *s;

  /* already compressing */
  if (dsock->out_compress)
    return TRUE;

  /* allocate and init stream, buffer */
  s = (z_stream *) malloc(sizeof(*s));
  dsock->out_compress_buf = (unsigned char *) malloc(COMPRESS_BUF_SIZE);

  s->next_in    =  NULL;
  s->avail_in   =  0;
  s->next_out   =  dsock->out_compress_buf;
  s->avail_out  =  COMPRESS_BUF_SIZE;
  s->zalloc     =  zlib_alloc;
  s->zfree      =  zlib_free;
  s->opaque     =  NULL;

  if (deflateInit(s, 9) != Z_OK)
  {
    free(dsock->out_compress_buf);
    free(s);
    return FALSE;
  }

  /* version 1 or 2 support */
  if (teleopt == TELOPT_COMPRESS)
    text_to_socket(dsock, (char *) enable_compress);
  else if (teleopt == TELOPT_COMPRESS2)
    text_to_socket(dsock, (char *) enable_compress2);
  else
  {
    bug("Bad teleoption %d passed", teleopt);
    free(dsock->out_compress_buf);
    free(s);
    return FALSE;
  }

  /* now we're compressing */
  dsock->compressing = teleopt;
  dsock->out_compress = s;

  /* success */
  return TRUE;
}

/* Cleanly shut down compression on `desc' */
bool compressEnd(D_SOCKET *dsock, unsigned char teleopt, bool forced)
{
  unsigned char dummy[1];

  if (!dsock->out_compress)
    return TRUE;

  if (dsock->compressing != teleopt)
    return FALSE;

  dsock->out_compress->avail_in = 0;
  dsock->out_compress->next_in = dummy;
  dsock->top_output = 0;

  /* No terminating signature is needed - receiver will get Z_STREAM_END */
  if (deflate(dsock->out_compress, Z_FINISH) != Z_STREAM_END && !forced)
    return FALSE;

  /* try to send any residual data */
  if (!processCompressed(dsock) && !forced)
    return FALSE;

  /* reset compression values */
  deflateEnd(dsock->out_compress);
  free(dsock->out_compress_buf);
  free(dsock->out_compress);
  dsock->compressing      = 0;
  dsock->out_compress     = NULL;
  dsock->out_compress_buf = NULL;

  /* success */
  return TRUE;
}

/* Try to send any pending compressed-but-not-sent data in `desc' */
bool processCompressed(D_SOCKET *dsock)
{
  int iStart, nBlock, nWrite, len;

  if (!dsock->out_compress)
    return TRUE;
    
  len = dsock->out_compress->next_out - dsock->out_compress_buf;
  if (len > 0)
  {
    for (iStart = 0; iStart < len; iStart += nWrite)
    {
      nBlock = UMIN (len - iStart, 4096);
      if ((nWrite = write(dsock->control, dsock->out_compress_buf + iStart, nBlock)) < 0)
      {
        if (errno == EAGAIN || errno == ENOSR)
          break;

        /* write error */
        return FALSE;
      }
      if (nWrite <= 0)
        break;
    }

    if (iStart)
    {
      if (iStart < len)
        memmove(dsock->out_compress_buf, dsock->out_compress_buf+iStart, len - iStart);

      dsock->out_compress->next_out = dsock->out_compress_buf + len - iStart;
    }
  }

  /* success */
  return TRUE;
}
