#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

/* main header file */
#include "mud.h"

sqlite3     *  db;

sqlite3_stmt *db_prepare_internal(const char *sql, va_list vars);

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
  va_list vars;
  sqlite3_stmt *stmt;

  va_start(vars, sql);

  stmt = db_prepare_internal(sql, vars);

  va_end(vars);

  if ( stmt == NULL ) {
    return FALSE;
  }


  if ( db_step(stmt) != SQLITE_DONE ) {
    bug("Failed to step through statement: %s", sqlite3_errmsg(db));

    return FALSE;
  }

  if ( db_finalize(stmt) != SQLITE_OK ) {
    bug("Failed to finalize statement: %s", sqlite3_errmsg(db));

    return FALSE;
  }

  return TRUE;
}

sqlite3_stmt *db_prepare(const char *sql, ...)
{
  va_list vars;
  sqlite3_stmt *stmt;

  va_start(vars, sql);

  stmt = db_prepare_internal(sql, vars);

  va_end(vars);

  return stmt;
}

int db_step(sqlite3_stmt *stmt) {
  return sqlite3_step( stmt );
}

int db_finalize(sqlite3_stmt *stmt) {
  return sqlite3_finalize( stmt );
}

sqlite3_stmt *db_prepare_internal(const char *sql, va_list vars)
{
  sqlite3_stmt *stmt;
  int index = 1;
  int i;

  char* buffer = (char*) malloc( (strlen(sql) + 1) * sizeof(char) );
  memcpy( buffer, sql, strlen(sql) + 1 );

  /* convert the query into an appropriate prepared statement */
  for ( i = 0; buffer[i] != '\0'; i++ )
  {
    /* convert %% into % for LIKE statements */
    if ( buffer[i] == '%' && buffer[i + 1] == '%' )
    {
        buffer[i + 1] = ' ';
        continue;
    }

    /* replace % with ? */
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

    return NULL;
  }

  /* re-iterate through the query and bind relevant values */
  for ( i = 0; sql[i] != '\0'; i++ )
  {
    /* % symbol */
    if ( sql[i] == '%' && sql[i + 1] == '%' )
    {
      /* skip the next % */
      i++;
      continue;
    }

    /* int */
    if ( sql[i] == '%' && sql[i + 1] == 'i' )
    {
      sqlite3_bind_int( stmt, index, va_arg( vars, int ) );
      index++;
      continue;
    }

    /* string */
    if ( sql[i] == '%' && sql[i + 1] == 's' )
    {
      sqlite3_bind_text( stmt, index, va_arg( vars, char* ), -1, NULL );
      index++;
      continue;
    }

    /* float */
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

  free( buffer );

  return stmt;
}

void db_migrate()
{

  if ( !db_open() )
  {
    abort();
  }

  /* players table */
  if ( !db_execute("CREATE TABLE IF NOT EXISTS players (id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, password TEXT NOT NULL, level INTEGER)") )
  {
    abort();
  }

  db_close();

  return;
}
