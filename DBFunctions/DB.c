#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../sqlite3.h"
#include <stdbool.h>

sqlite3 *DB;
// this func does the initialization and thats it. It creates the db file if it doesnt exist.
// returns 1 if created or exists, 0 on error - will show messages in terminal.
int initializeDatabase(const char *db_name, const char *table_creation_sql)
{
    if (fopen(db_name, "r"))
    {
        printf("Database already exists.\n");
        sqlite3_close(DB);
        return 1;
    }
    else
    {
        int exit = sqlite3_open(db_name, &DB);
        if (exit)
        {
            fprintf(stderr, "Error creating DB: %s\n", sqlite3_errmsg(DB));
            return 0;
        }
        printf("Database created successfully!\n");
        sqlite3_close(DB);
        return 1;
    }
}

char *readRecord(const char *database, const char *table, const char *conditions)
// this function reads all records from the specified table in the database.
{
    int rc;
    rc = sqlite3_open(database, &DB);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(DB));
        return NULL;
    }

    char sql[1024];
    if (conditions == NULL || strlen(conditions) == 0)
        snprintf(sql, sizeof(sql), "SELECT * FROM %s;", table);
    else
        snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE %s;", table, conditions);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(DB, sql, -1, &stmt, NULL); // this one was hard.
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(DB));
        sqlite3_close(DB);
        return NULL;
    }

    size_t bufsize = 4096;
    char *result = malloc(bufsize);
    if (!result)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return NULL;
    }
    result[0] = '\0';
    strcat(result, "[");

    bool first = true;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (!first)
            strcat(result, ",");
        first = false;
        strcat(result, "{");
        int cols = sqlite3_column_count(stmt);
        for (int i = 0; i < cols; i++)
        {
            if (i > 0)
                strcat(result, ",");
            const char *colName = sqlite3_column_name(stmt, i);
            const char *colText = (const char *)sqlite3_column_text(stmt, i);
            char temp[512];
            snprintf(temp, sizeof(temp), "\"%s\":\"%s\"", colName, colText ? colText : "null");
            strncat(result, temp, bufsize - strlen(result) - 1);
        }
        strcat(result, "}");
    }
    strcat(result, "]");
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return result;
}

// maybe i should make a function that just runs an sql query and returns the res and status????? like a general purpose one?
// i think it would be better to write sql queries directly for more complex stuff. So just run the query and return status if success or fail and maybe return the data if any.
// thats how we do it at work...


int createRecord(const char *table, const char *columns, const char *values, const char *db_name)
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