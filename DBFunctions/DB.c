#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../sqlite3.h"
#include <stdbool.h>
#include "helpers/helpers.h"

sqlite3 *DB;
#define BUFFER_SIZE 4096
// this func does the initialization and thats it. It creates the db file if it doesnt exist.
// returns 1 if created or exists, 0 on error - will show messages in terminal.
bool doesDatabaseExist(const char *db_name, const char *table_creation_sql)
{
    if (fopen(db_name, "r"))
    {
        printf("Database already exists.\n");
        sqlite3_close(DB);
        return true;
    }
    else
    {
        int exit = sqlite3_open(db_name, &DB);
        if (exit)
        {
            fprintf(stderr, "Error creating DB: %s\n", sqlite3_errmsg(DB));
            return false;
        }
        printf("Database created successfully!\n");
        sqlite3_close(DB);
        return true;
    }
}

char *runSQL(const char *database, const char *query) // general purpose.
{
    int rc;
    rc = sqlite3_open(database, &DB);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(DB));
        return NULL;
    }

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(DB, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(DB));
        sqlite3_close(DB);
        return NULL;
    }

    if (sqlite3_column_count(stmt) == 0) // does not return rows it is for insert, update, delete
    {
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc != SQLITE_DONE)
        {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(DB));
            sqlite3_close(DB);
            return NULL;
        }

        char *result = malloc(256);
        if (result)
        {
            snprintf(result, 256, "{\"status\":\"success\",\"message\":\"Query executed successfully\",\"affected_rows\":%d}",
                     sqlite3_changes(DB));
        }

        sqlite3_close(DB);
        return result;
    }

    size_t bufsize = 4096;
    size_t current_size = bufsize;
    char *result = malloc(current_size); // returns select queries
    if (!result)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return NULL;
    }

    strcpy(result, "[");
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
            char *escapedValue = escape_JSON(colText ? colText : "null");
            char temp[1024];
            snprintf(temp, sizeof(temp), "\"%s\":\"%s\"", colName, escapedValue);
            free(escapedValue);

            size_t temp_len = strlen(temp);
            if (strlen(result) + temp_len + 10 > current_size)
            {
                current_size *= 2;
                char *new_result = realloc(result, current_size);
                if (!new_result)
                {
                    free(result);
                    sqlite3_finalize(stmt);
                    sqlite3_close(DB);
                    return NULL;
                }
                result = new_result;
            }

            strncat(result, temp, current_size - strlen(result) - 1);
        }
        strcat(result, "}");
    }

    strcat(result, "]");

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return result;
}

char table_creation_sql[BUFFER_SIZE / 4] = "CREATE TABLE IF NOT EXISTS SQL_IN_C ( id INTEGER PRIMARY KEY NOT NULL, name TEXT NOT NULL, job TEXT NOT NULL );";
void initializeDB(const char *database)
{
    bool db_exists = doesDatabaseExist(database, table_creation_sql);
    if (db_exists)
    {
        printf("%s exists\n", database);
        runSQL(database, table_creation_sql);
        return;
    }
    else
    {
        printf("Error initializing database: %s\n", database);
        return;
    }
}
