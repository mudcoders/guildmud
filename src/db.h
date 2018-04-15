/* file: db.h
 *
 * Headerfile for sqlite interface
 */

#ifndef _DB_HEADER
#define _DB_HEADER

#include <sqlite3.h>
#include <stdbool.h>

/*
 * db.c
 */
bool           db_open        ( void );
bool           db_close       ( void );
bool           db_execute     ( const char *sql, ... );
sqlite3_stmt  *db_prepare     (const char *sql, ...);
int            db_step        (sqlite3_stmt *stmt);
int            db_finalize    (sqlite3_stmt *stmt);
void           db_migrate     ( void );
  
#endif
