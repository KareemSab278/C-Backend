// gcc server.c main.c -o backend -lws2_32
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <winsock2.h> // windows
// #include <arpa/inet.h> // linux
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

char *getHTTPRequestLine(const char *http_request, char *buffer, size_t buffer_size)
{
    const char *line_end = strstr(http_request, "\r\n");
    if (!line_end)
    {
        return NULL;
    }

    size_t line_length = line_end - http_request;
    if (line_length >= buffer_size)
    {
        return NULL;
    }

    strncpy(buffer, http_request, line_length);
    buffer[line_length] = '\0';
    return buffer;
}

char *getPostParam(const char *body, const char *param)
{
    char *start = strstr(body, param);
    if (!start)
        return NULL;
    start += strlen(param) + 1; // Skip "param="
    char *end = strchr(start, '&');
    if (!end)
        end = start + strlen(start);
    size_t len = end - start;
    char *value = malloc(len + 1);
    if (!value)
        return NULL;
    strncpy(value, start, len);
    value[len] = '\0';
    return value;
}

char *runFrontendFunctionCall(const char *function_call, const char *parameters)
{
    if (strcmp(function_call, "get_all_records") == 0)
    {
        return runSQL("SQL_IN_C.db", "SELECT * FROM SQL_IN_C;");
    }


    if (strcmp(function_call, "add_record") == 0)
    {
        char query[BUFFER_SIZE];
        //                              function=add_record&parameters='jacon', 'manager'
        snprintf(query, sizeof(query), "INSERT INTO SQL_IN_C (name, job) VALUES (%s);", parameters); // parameters should be formatted as "name', 'job"
        return runSQL("SQL_IN_C.db", query);
    }
    else
    {
        return strdup("{\"status\":\"error\",\"message\":\"Invalid function call\"}");
    }
}

char *RETURN_API_OUTPUT(char *output_query)
{
    return runSQL("SQL_IN_C.db", output_query);
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

        char *bodyStart = strstr(buffer, "\r\n\r\n");
        char *API_OUTPUT = NULL;
        if (bodyStart)
        {
            bodyStart += 4;
            char *function = getPostParam(bodyStart, "function");
            char *parameters = getPostParam(bodyStart, "parameters");


            if (function && strcmp(function, "add_record") == 0 && parameters)
            {
                API_OUTPUT = runFrontendFunctionCall("add_record", parameters);
            }


            if (function && strcmp(function, "get_all_records") == 0)
            {
                API_OUTPUT = runFrontendFunctionCall("get_all_records", NULL);
            }


            free(function);
            free(parameters);
        }
        else
        {
            API_OUTPUT = strdup("{\"status\":\"error\",\"message\":\"Invalid function call\"}");
        }

        if (!API_OUTPUT)
        {
            API_OUTPUT = strdup("{\"status\":\"error\",\"message\":\"Invalid request\"}");
        }

        char responseBuffer[BUFFER_SIZE * 4];
        snprintf(responseBuffer, sizeof(responseBuffer), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s", API_OUTPUT);
        send(clientSocket, responseBuffer, strlen(responseBuffer), 0);

        if (API_OUTPUT != NULL)
        {
            free(API_OUTPUT);
        }

        closesocket(clientSocket);
    }
    closesocket(serverSocket);
    printf("Disconnected\n");

    return 0;
}