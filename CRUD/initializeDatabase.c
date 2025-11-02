#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../sqlite3.h"

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