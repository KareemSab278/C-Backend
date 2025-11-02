#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../sqlite3.h"


void readRecord(const char *database, const char *table, const char *conditions)
{
    sqlite3 *db;
    char *errMsg = 0;
    int rc;
    // database = "SQL_IN_C.db";
    rc = sqlite3_open(database, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    char sql[1024];
    if (conditions == NULL || strlen(conditions) == 0)
        snprintf(sql, sizeof(sql), "SELECT * FROM %s;", table);
    else {
        snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE %s;", table, conditions);
    }

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int cols = sqlite3_column_count(stmt);
        for (int i = 0; i < cols; i++)
        {
            const char *colText = (const char *)sqlite3_column_text(stmt, i);
            printf("%s ", colText ? colText : "NULL");
        }
        printf("\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}