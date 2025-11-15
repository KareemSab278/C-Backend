#ifndef DB_H
#define DB_H

// int createRecord(const char *table, const char *columns, const char *values, const char *db_name);
//                           table name         columns name         values name         database name
// char *readRecord(const char *database, const char *table, const char *conditions);
//                      database name       table name        conditions (WHERE ...)
bool doesDatabaseExist(const char *db_name, const char *table_creation_sql);
//                                 database name        table creation sql
char *runSQL(const char *database, const char *query);
//                     database name       sql query

void initializeDB(const char *database);
#endif