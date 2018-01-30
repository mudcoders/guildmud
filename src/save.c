#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* main header file */
#include "mud.h"

void save_player(D_MOBILE *dMob)
{
  if (!dMob)
  {
    return;
  }

  if ( !db_open() )
  {
    return;
  }

  if ( !db_execute("CREATE TABLE IF NOT EXISTS players (id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, password TEXT NOT NULL, level INTEGER)") )
  {
    return;
  }

  if ( !db_execute("INSERT INTO PLAYERS (name, password, level) VALUES (%s, %s, %i)", dMob->name, dMob->password, dMob->level) )
  {
    return;
  }

  db_close();
}

D_MOBILE *load_player(char *player)
{
  FILE *fp;
  D_MOBILE *dMob = NULL;
  char pfile[MAX_BUFFER];
  char pName[MAX_BUFFER];
  char *word;
  bool done = FALSE, found;
  int i, size;

  pName[0] = toupper(player[0]);
  size = strlen(player);
  for (i = 1; i < size && i < MAX_BUFFER - 1; i++)
    pName[i] = tolower(player[i]);
  pName[i] = '\0';

  /* open the pfile so we can write to it */
  snprintf(pfile, MAX_BUFFER, "../players/%s.pfile", pName);
  if ((fp = fopen(pfile, "r")) == NULL)
    return NULL;

  /* create new mobile data */
  if (StackSize(dmobile_free) <= 0)
  {
    if ((dMob = malloc(sizeof(*dMob))) == NULL)
    {
      bug("Load_player: Cannot allocate memory.");
      abort();
    }
  }
  else
  {
    dMob = (D_MOBILE *) PopStack(dmobile_free);
  }
  clear_mobile(dMob);

  /* load data */
  word = fread_word(fp);
  while (!done)
  {
    found = FALSE;
    switch (word[0])
    {
      case 'E':
        if (!strcasecmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
        break;
      case 'L':
        IREAD( "Level",     dMob->level     );
        break;
      case 'N':
        SREAD( "Name",      dMob->name      );
        break;
      case 'P':
        SREAD( "Password",  dMob->password  );
        break;
    }
    if (!found)
    {
      bug("Load_player: unexpected '%s' in %s's pfile.", word, player);
      free_mobile(dMob);
      return NULL;
    }

    /* read one more */
    if (!done) word = fread_word(fp);
  }

  fclose(fp);
  return dMob;
}

/*
 * This function loads a players profile, and stores
 * it in a mobile_data... DO NOT USE THIS DATA FOR
 * ANYTHING BUT CHECKING PASSWORDS OR SIMILAR.
 */
D_MOBILE *load_profile(char *player)
{
  FILE *fp;
  D_MOBILE *dMob = NULL;
  char pfile[MAX_BUFFER];
  char pName[MAX_BUFFER];
  char *word;
  bool done = FALSE, found;
  int i, size;

  pName[0] = toupper(player[0]);
  size = strlen(player);
  for (i = 1; i < size && i < MAX_BUFFER - 1; i++)
    pName[i] = tolower(player[i]);
  pName[i] = '\0';

  /* open the pfile so we can write to it */
  snprintf(pfile, MAX_BUFFER, "../players/%s.profile", pName);
  if ((fp = fopen(pfile, "r")) == NULL)
    return NULL;

  /* create new mobile data */
  if (StackSize(dmobile_free) <= 0)
  {
    if ((dMob = malloc(sizeof(*dMob))) == NULL)
    {
      bug("Load_profile: Cannot allocate memory.");
      abort();
    }
  }
  else
  {
    dMob = (D_MOBILE *) PopStack(dmobile_free);
  }
  clear_mobile(dMob);

  /* load data */
  word = fread_word(fp);
  while (!done)
  {
    found = FALSE;
    switch (word[0])
    {
      case 'E':
        if (!strcasecmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
        break;
      case 'N':
        SREAD( "Name",      dMob->name      );
        break;
      case 'P':
        SREAD( "Password",  dMob->password  );
        break;
    }
    if (!found)
    {
      bug("Load_player: unexpected '%s' in %s's pfile.", word, player);
      free_mobile(dMob);
      return NULL;
    }

    /* read one more */
    if (!done) word = fread_word(fp);
  }

  fclose(fp);
  return dMob;
}
