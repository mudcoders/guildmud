/*
 * This file handles input/output to files (including log)
 */
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>  
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

/* include main header file */
#include "mud.h"

extern FILE *stderr;
time_t current_time;

/*
 * Nifty little extendable logfunction,
 * if it wasn't for Erwins social editor,
 * I would never have known about the
 * va_ functions.
 */
void log_string(const char *txt, ...)
{
  FILE *fp;
  char logfile[MAX_BUFFER];
  char buf[MAX_BUFFER];
  char *strtime = get_time();
  va_list args;

  va_start(args, txt);
  vsnprintf(buf, MAX_BUFFER, txt, args);
  va_end(args);

  /* point to the correct logfile */
  snprintf(logfile, MAX_BUFFER, "../log/%6.6s.log", strtime);

  /* try to open logfile */
  if ((fp = fopen(logfile, "a")) == NULL)
  {
    communicate(NULL, "log: cannot open logfile", COMM_LOG);
    return;
  }

  fprintf(fp, "%s: %s\n", strtime, buf);
  fclose(fp);

  communicate(NULL, buf, COMM_LOG);
}

/*
 * Nifty little extendable bugfunction,
 * if it wasn't for Erwins social editor,
 * I would never have known about the
 * va_ functions.
 */
void bug(const char *txt, ...)
{
  FILE *fp;
  char buf[MAX_BUFFER];
  va_list args;
  char *strtime = get_time();

  va_start(args, txt);
  vsnprintf(buf, MAX_BUFFER, txt, args);
  va_end(args);

  /* try to open logfile */
  if ((fp = fopen("../log/bugs.txt", "a")) == NULL)
  {
    communicate(NULL, "bug: cannot open bugfile", COMM_LOG);
    return;
  }

  fprintf(fp, "%s: %s\n", strtime, buf);
  fclose(fp);

  communicate(NULL, buf, COMM_LOG);
}

/*
 * This function will return the time of
 * the last modification made to helpfile.
 */
time_t last_modified(char *helpfile)
{
  char fHelp[MAX_BUFFER];
  struct stat sBuf;
  time_t mTime = 0;

  snprintf(fHelp, MAX_BUFFER, "../help/%s", helpfile);
  if (stat(fHelp, &sBuf) >= 0)
    mTime = sBuf.st_mtime;

  return mTime;
}

char *read_help_entry(const char *helpfile)
{
  FILE *fp;
  static char entry[MAX_HELP_ENTRY];
  char fHelp[MAX_BUFFER];
  int c, ptr = 0;

  /* location of the help file */
  snprintf(fHelp, MAX_BUFFER, "../help/%s", helpfile);

  /* if there is no help file, return NULL */
  if ((fp = fopen(fHelp, "r")) == NULL)
    return NULL;

  /* just to have something to work with */
  c = getc(fp);

  /* read the file in the buffer */
  while (c != EOF)
  {
    if (c == '\n')
      entry[ptr++] = '\r';
    entry[ptr] = c;
    if (++ptr > MAX_HELP_ENTRY - 2)
    {
      bug("Read_help_entry: String to long.");
      abort();
    }
    c = getc(fp);
  }
  entry[ptr] = '\0';

  fclose(fp);

  /* return a pointer to the static buffer */
  return entry;
}

/*
 * Reads one line from a file, and returns a
 * pointer to a _static array_ holding the line.
 */
char *fread_line(FILE *fp)
{
  static char line[MAX_BUFFER];
  int c, entry = 0;

  /* read one line from the file */
  c = getc(fp);
  while (c != EOF && c != '\n')
  {
    line[entry] = c;
    if (++entry > MAX_BUFFER - 1)
    {
      bug("Fread_line: Line to long.");
      abort();
    }
    c = getc(fp);
  }
  line[entry] = '\0';

  /* did we actually read anything ? */
  if (entry == 0) return NULL;

  /* return a pointer to the static buffer */
  return line;
}

/*
 * Reads one number from a file, returning
 * the value as an integer, a leading '-' will
 * be intepreted as a negative value, and this
 * is the only non-digit that the function
 * will not choke and die upon reading.
 */
int fread_number(FILE *fp)
{
  int c, number = 0;
  bool negative = FALSE;

  /* initial read */
  c = getc(fp);

  /* speed through leading spaces */
  while (c != EOF && (c == ' ' || c == '\n'))
    c = getc(fp);

  /* so what did we get ? */
  if (c == EOF)
  {
    bug("Fread_number: EOF encountered.");
    abort();
  }
  else if (c == '-')
    negative = TRUE;
  else if (!isdigit(c))
  {
    bug("Fread_number: Not a number.");
    abort();
  }
  else
    number = c - '0';

  /* keep counting up */
  while (isdigit(c = getc(fp)))
    number = number * 10 + c - '0';

  /* push back the non-digit */
  ungetc(c, fp);

  /* we have a number */
  return (negative ? (0 - number) : number);
}

/*
 * Reads one full block of text, which ends with a
 * '~' (tilde). The result will be copied into an
 * allocated buffer and a pointer to that buffer
 * will be returned. Remember to free the memory when
 * you are done using it.
 */
char *fread_string(FILE *fp)
{
  char buf[4 * MAX_BUFFER];
  int c, count = 0;

  /* initial read */
  c = getc(fp);

  /* speed through leading spaces */
  while (c != EOF && (c == ' ' || c == '\n'))
    c = getc(fp);

  /* better not have reached the end of the file */
  if (c == EOF)
  {
    bug("Fread_string: EOF encountered.");
    abort();
  }

  /* and keep reading */
  while (c != EOF && c != '~')
  {
    if (c == '\n')
    {
      buf[count++] = '\r';
      buf[count] = '\n';
    }
    else if (c == '\r')
    {
      c = getc(fp);
      continue;
    }
    else buf[count] = c;
    if (++count > (4 * MAX_BUFFER - 2))
    {
      bug("Fread_string: String to long.");
      abort();
    }
    c = getc(fp);
  }
  buf[count] = '\0';

  if (c == EOF)
    bug("Fread_string: Non-fatal error, encountered EOF instead of ~.");

  return strdup(buf);
}

/*
 * Reads one single word, ending with the first
 * encountered space ' '. Any leading spaces will
 * be ignored, and the result will be copied into
 * a static buffer, and a pointer to that buffer
 * will be returned.
 */
char *fread_word(FILE *fp)
{
  static char buf[MAX_BUFFER];
  int c, count = 0;

  /* initial read */
  c = getc(fp); 

  /* speed through leading spaces and linebreaks */
  while (c != EOF && (c == ' ' || c == '\n'))
    c = getc(fp);

  /* better not have reached the end of the file */
  if (c == EOF)
  {
    bug("Fread_word: EOF encountered.");
    abort();
  }

  /* and keep reading */
  while (c != EOF && c != ' ' && c != '\n')
  {
    buf[count] = c;
    if (++count > MAX_BUFFER - 1)
    {
      bug("Fread_word: Word to long.");
      abort();
    }
    c = getc(fp);
  }
  buf[count] = '\0';

  /* push back the last read if it was EOF */
  if (c == EOF) ungetc(c, fp);

  return buf;
}
