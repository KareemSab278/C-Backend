// gcc server.c main.c -o backend -lws2_32
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <winsock2.h> // has to be like this on windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
// #include <mysql.h> // oracle decided to replace mysql.h with jdbc for c/c++ yet idk how to use it... mysql.h doesnt work rn...
// WTF ORACLE - GOOFY AHH COMPANY FIX YOUR DOCS!!!
#include "sqlite3.h"
#include <unistd.h>
#include <string.h>
#include "CRUD/initializeDatabase.c"

#define PORT 6969        // def port
#define BUFFER_SIZE 1024 // how many bytes to handle at once (1kb)

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

// too lazy to dynamically allocate memory for this so just make it smaller for now lol
char table_creation_sql[BUFFER_SIZE / 4] = "CREATE TABLE IF NOT EXISTS SQL_IN_C ( id INTEGER PRIMARY KEY NOT NULL, name TEXT NOT NULL, job TEXT NOT NULL );";

void runSQLQuery(const char *database)
{
    if (initializeDatabase(database, table_creation_sql) == 0)
    {
        printf("Database already exists: %s\n", database);
        return;
    }
    else if (initializeDatabase(database, table_creation_sql) == -1)
    {
        printf("Error initializing database: %s\n", database);
        return;
    }
    else
    {
        printf("Database created: %s\n", database);
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

        success_response response = {
            .status_code = 200,
            .status_text = "OK",
            .content_type = "application/json",
            .body = "{\"message\":\"Hello, World!\"}"};

        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE,
                 "HTTP/1.1 %d %s\r\nContent-Type: %s\r\n\r\n%s",
                 response.status_code, response.status_text, response.content_type, response.body);

        send(clientSocket, buffer, strlen(buffer), 0);
    }
    close(clientSocket);
    printf("Disconnected\n");

    close(serverSocket);
    return 0;
}