#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

/* main header file */
#include "mud.h"

sqlite3     *  db;

bool db_open()
{
  if (sqlite3_open(DATABASE_FILE, &db) != SQLITE_OK)
  {
    bug("Cannot open database: %s", sqlite3_errmsg(db));

    db_close();

    return FALSE;
  }

  return TRUE;
}

bool db_close()
{
  if (sqlite3_close(db) != SQLITE_OK )
  {
    bug("Unable to close database: %s", sqlite3_errmsg(db));

    return FALSE;
  }

  return TRUE;
}

bool db_execute(const char *sql, ...)
{
  sqlite3_stmt *stmt;
  va_list vars;
  int index = 1;

  char* buffer = (char*) malloc( (strlen(sql) + 1) * sizeof(char) );
  memcpy( buffer, sql, strlen(sql) + 1 );

  // convert the query into an appropriate prepared statement
  for ( int i = 0; buffer[i] != '\0'; i++ )
  {
    // convert %% into % for LIKE statements
    if ( buffer[i] == '%' && buffer[i + 1] == '%' )
    {
        buffer[i + 1] = ' ';
        continue;
    }

    // replace % with ?
    if ( buffer[i] == '%' )
    {
        buffer[i] = '?';
        buffer[i + 1] = ' ';
    }
  }

  if (sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) != SQLITE_OK )
  {
    bug("Failed to prepare SQL statement (%s): %s", buffer, sqlite3_errmsg(db));
    free( buffer );

    return FALSE;
  }

  va_start(vars, sql);

  // re-iterate through the query and bind relevant values
  for ( int i = 0; sql[i] != '\0'; i++ )
  {
    // % symbol
    if ( sql[i] == '%' && sql[i + 1] == '%' )
    {
      // skip the next %
      i++;
      continue;
    }

    // int
    if ( sql[i] == '%' && sql[i + 1] == 'i' )
    {
      sqlite3_bind_int( stmt, index, va_arg( vars, int ) );
      index++;
      continue;
    }

    // string
    if ( sql[i] == '%' && sql[i + 1] == 's' )
    {
      sqlite3_bind_text( stmt, index, va_arg( vars, char* ), -1, NULL );
      index++;
      continue;
    }

    // float
    if ( sql[i] == '%' && sql[i + 1] == 'f' )
    {
      sqlite3_bind_double( stmt, index, (double)va_arg( vars, double ) );
      index++;
      continue;
    }

    if ( sql[i] == '%' && sql[i + 1] != '\0' )
    {
      bug("Unknown binding (%s): %c%c\n", sql, sql[i], sql[i + 1]);
    }
  }

  va_end(vars);

  free( buffer );

  if (sqlite3_step( stmt ) != SQLITE_DONE )
  {
    bug("Unable to step through statement (%s): %s", buffer, sqlite3_errmsg(db));

    return FALSE;
  }

  if ( sqlite3_finalize( stmt ) != SQLITE_OK ) {
    bug("Unable to destroy prepared statement (%s): %s", buffer, sqlite3_errmsg(db));

    return FALSE;
  }

  return TRUE;
}