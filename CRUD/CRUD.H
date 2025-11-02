#ifndef CRUD_H
#define CRUD_H

int insertRecord(const char *table, const char *columns, const char *values, const char *db_name);
//                           table name         columns name         values name         database name
void readRecord(const char *table, const char *conditions);
int initializeDatabase(const char *db_name, const char *table_creation_sql);

#endif