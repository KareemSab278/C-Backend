#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

sqlite3 *DB;

int insertRecord(const char *table, const char *columns, const char *values, const char *db_name)
{ //                         table name         columns              values 

    // will need to check if db exists before inserting
    if (fopen(db_name, "r"))
    {
        char sql[512];
        snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS %s (%s); INSERT INTO %s VALUES (%s);", table, columns, table, values);
        // INSERT INTO table VALUES (0, 'Clark', 'Sales');

        int exit = sqlite3_open(db_name, &DB);
        if (exit)
        {
            fprintf(stderr, "Error opening DB: %s\n", sqlite3_errmsg(DB));
            return -1;
        }

        char *errMsg = 0;
        exit = sqlite3_exec(DB, sql, 0, 0, &errMsg);
        if (exit != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(DB);
            return -1;
        }

        printf("Record inserted successfully!\n");
        sqlite3_close(DB);
        return 0;
    }
    else
    {
        printf("Database does not exist.\n");
        return -1;
    }
}