// gcc server.c main.c -o backend -lws2_32
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <winsock2.h>
#include "sqlite3.h"
#include <unistd.h>
#include <string.h>
#include "DBFunctions/DB.h"
#include "helpers/helpers.h"

#define PORT 6969        // def port
#define BUFFER_SIZE 4096 // how many bytes to handle at once (1kb)

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

char table_creation_sql[BUFFER_SIZE / 4] = "CREATE TABLE IF NOT EXISTS SQL_IN_C ( id INTEGER PRIMARY KEY NOT NULL, name TEXT NOT NULL, job TEXT NOT NULL );";

void runSQLQuery(const char *database)
{
    int db_exists = initializeDatabase(database, table_creation_sql); // should return 1 else 0
    if (db_exists)
    {
        printf("%s exists\n", database);
        return;
    }
    else
    {
        printf("Error initializing database: %s\n", database);
        return;
    }
}

void populateResponseBody(success_response *response, const char *status_code, const char *status_text, const char *bodyContent)
{
    response->status_code = atoi(status_code); // (A)scii (TO) (I)nt -> ATOI
    response->status_text = status_text;
    response->content_type = "application/json";
    response->body = bodyContent;
}

int server()
{
    int serverSocket;
    int clientSocket;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    runSQLQuery("SQL_IN_C.db"); // create or initialize database

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
    while (isRunning)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_len);
        if (clientSocket < 0)
        {
            perror("Accept failed");
            continue;
        }
        printf("Connected\n");

        char *records = readRecord("SQL_IN_C.db", "SQL_IN_C", NULL);
        success_response response = {
            .status_code = 200,
            .status_text = "OK",
            .content_type = "application/json",
            .body = records};

        char json[BUFFER_SIZE * 4];
        if (records)
        {
            char *escaped = escape_JSON(records);
            snprintf(json, sizeof(json), "{\"records\":%s}", records);
            populateResponseBody(&response, "200", "OK", json);
            printf("records terminal response: %s\n", json);
            free(records);
        }
        else
        {
            populateResponseBody(&response, "200", "OK", "{\"records\":[]}");
        }

        char buffer[BUFFER_SIZE * 4];
        snprintf(buffer, sizeof(buffer),
                 "HTTP/1.1 %d %s\r\nContent-Type: %s\r\n\r\n%s",
                 response.status_code, response.status_text, response.content_type, response.body);

        send(clientSocket, buffer, strlen(buffer), 0);
        close(clientSocket);
    }
    printf("Disconnected\n");

    close(serverSocket);
    return 0;
}