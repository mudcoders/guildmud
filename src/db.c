#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* main header file */
#include "mud.h"
#include "db.h"

sqlite3     *  db;

sqlite3_stmt *db_prepare_internal(const char *sql, va_list vars);
int get_db_schema();

/*
 * Array of commands to be executed by db_migrate to apply changes to the database.
 * 
 */

typedef struct migration_commands {
    int schema;
    char *action;
} migration_commands;

migration_commands command[] = {
/*  { DB_SCHEMA, Action to upgrade the database to the DB_SCHEMA } */  
    { 0, "CREATE TABLE IF NOT EXISTS players (id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, password TEXT NOT NULL, level INTEGER)" },
    { 1, "CREATE TABLE IF NOT EXISTS NEXT_ID (ID DECIMAL(9,0) NOT NULL)" },
    { 1, "INSERT INTO NEXT_ID (ID) VALUES(1)"},
    {-1, NULL } // Sentinel to identify end of commands. DO NOT REMOVE.
}; 



/*
 * Open the global database.
 * 
 */

bool db_open()
{
  if (sqlite3_open(DATABASE_FILE, &db) != SQLITE_OK)
  {
    bug("Cannot open database: %s", sqlite3_errmsg(db));

    db_close();

    return false;
  }

  return true;
}

/*
 * Close the global database.
 * 
 */

bool db_close()
{
  if (sqlite3_close(db) != SQLITE_OK )
  {
    bug("Unable to close database: %s", sqlite3_errmsg(db));

    return false;
  }

  return true;
}

/*
 * Execute a query to the database expecting no answer.
 * Used for things like DROP, INSERT, CREATE TABLE...
 */

bool db_execute(const char *sql, ...)
{
  va_list vars;
  sqlite3_stmt *stmt;

  va_start(vars, sql);

  stmt = db_prepare_internal(sql, vars);

  va_end(vars);

  if ( stmt == NULL ) {
    return false;
  }

  if ( db_step(stmt) != SQLITE_DONE ) {
    bug("Failed to step through statement: %s", sqlite3_errmsg(db));

    return false;
  }

  if ( db_finalize(stmt) != SQLITE_OK ) {
    bug("Failed to finalize statement: %s", sqlite3_errmsg(db));

    return false;
  }

  return true;
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

  int db_schema = get_db_schema();
  int i = 0;

  if ( !db_open() )
  {
    abort();
  }

  if (-1 == db_schema) {
    if(  !db_execute("CREATE TABLE IF NOT EXISTS DB_SCHEMA (db_version DECIMAL(6,0) NOT NULL)")
      || !db_execute("INSERT INTO DB_SCHEMA (db_version) VALUES(1)")) {
      abort();
    }
  }

  db_execute("BEGIN EXCLUSIVE TRANSACTION");
  while(command[i].schema != -1) {
    if (command[i].schema > db_schema) {
      if ( !db_execute(command[i].action) ) {
        db_execute("ROLLBACK TRANSACTION");  
        abort();
      }
    }
    if( !db_execute("UPDATE DB_SCHEMA SET db_version = %i", command[i].schema) ) {
      db_execute("ROLLBACK TRANSACTION");  
      abort();
    }
    i++;
  }
  db_execute("COMMIT TRANSACTION");


  if(command[i-1].schema > db_schema ) {
    log_string("db_schema updated to version %d", command[i-1].schema);
  } else {
    log_string("Current db_schema: %d", db_schema);
  }

  db_close();

  return;
}


/*
 * Retrieve the schema version.
 * 
 * SELECT db_version FROM DB_SCHEMA
 * 
 */

int get_db_schema()
{
  sqlite3_stmt *stmt;
  int db_schema = -1;
  
  if ( !db_open() )
  {
    abort();
  }

  stmt = db_prepare("SELECT db_version FROM DB_SCHEMA");

  if ( stmt == NULL )
  {
    db_close();
    return db_schema;
  }

  if ( db_step(stmt) == SQLITE_ROW ) {
    db_schema      = sqlite3_column_int(stmt, 0);
  }

  if ( db_finalize(stmt) != SQLITE_OK ) {
    bug("Failed to finalize statement: %s", sqlite3_errmsg(db));

    abort();
  }

  db_close();

  return db_schema;
}
