// gcc server.c main.c -o backend -lws2_32
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
// #include <winsock2.h>
#include <arpa/inet.h>
#include "sqlite3.h"
#include <unistd.h>
#include <string.h>
#include "DBFunctions/DB.h"
#include "helpers/helpers.h"

#define PORT 6969        // def port
#define BUFFER_SIZE 4096 // how many bytes to handle at once (1kb)

char table_creation_sql[BUFFER_SIZE / 4] = "CREATE TABLE IF NOT EXISTS SQL_IN_C ( id INTEGER PRIMARY KEY NOT NULL, name TEXT NOT NULL, job TEXT NOT NULL );";

typedef struct success_response
{
    int status_code;
    const char *status_text;
    const char *content_type;
    const char *body;
} success_response;

success_response response = {
    200,
    "OK",
    "application/json",
    "{\"message\":\"Empty Success Response\"}"};

char *populateResponseBody(success_response *response, const char *status_code, const char *status_text, const char *bodyContent)
{
    response->status_code = atoi(status_code); // (A)scii (TO) (I)nt -> ATOI
    response->status_text = status_text;
    response->content_type = "application/json";
    response->body = bodyContent;
}

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

int server()
{
    int serverSocket;
    int clientSocket;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    initializeDB("SQL_IN_C.db"); // create or initialize database

    if (serverSocket == -1)
    {
        perror("Failed to start server");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) < 0)
    { // up to 5 can queue
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // if all is successful show listening port
    printf("Listening on port http://localhost:%d !\n", PORT);
    bool isRunning = true;

    // ===============================================================================================
    // RUN ALL SQL HERE

    runSQL("SQL_IN_C.db", "INSERT OR REPLACE INTO SQL_IN_C (name, job) VALUES ('Alice', 'Engineer');");
    runSQL("SQL_IN_C.db", "INSERT OR REPLACE INTO SQL_IN_C (name, job) VALUES ('Bob', 'Manager');");
    runSQL("SQL_IN_C.db", "INSERT OR REPLACE INTO SQL_IN_C (name, job) VALUES ('Charlie', 'Designer');");
    runSQL("SQL_IN_C.db", "select * from SQL_IN_C;");

    // ===============================================================================================
    // RUN SERVER LOOP
    while (isRunning)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_len);
        if (clientSocket < 0)
        {
            perror("Accept failed");
            continue;
        }
        printf("Connected\n");

        char buffer[BUFFER_SIZE] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead < 0)
        {
            perror("Receive failed");
            close(clientSocket);
            continue;
        }
        buffer[bytesRead] = '\0';
        printf("Received: %s\n", buffer);

        char *sql_result = runSQL("SQL_IN_C.db", "SELECT * FROM SQL_IN_C");

        if (sql_result)
        {
            char response[8192];
            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n"
                     "%s",
                     strlen(sql_result), sql_result);

            send(clientSocket, response, strlen(response), 0);
            free(sql_result);
        }

        close(clientSocket);
    }
    printf("Disconnected\n");
    close(serverSocket);

    return 0;
}