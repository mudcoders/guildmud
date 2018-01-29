#include <stdio.h>
#include <sqlite3.h>

/* main header file */
#include "mud.h"

sqlite3     *  db;

void open_database()
{
  int rc = sqlite3_open(DATABASE_FILE, &db);

  if (rc != SQLITE_OK)
  {
    bug("Cannot open database: %s\n", sqlite3_errmsg(db));
    close_database();
    return;
  }

  return;
}

void close_database()
{
  sqlite3_close(db);
  return;
}