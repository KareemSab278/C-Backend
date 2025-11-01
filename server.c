// gcc server.c main.c -o backend -lws2_32
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <winsock2.h>    // has to be like this on windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
#include <mysql.h> // oracle decided to replace mysql.h with jdbc for c/c++ yet idk how to use it... mysql.h doesnt work rn...
// WTF ORACLE - GOOFY AHH COMPANY FIX YOUR DOCS!!!
#include <unistd.h>
#include <string.h>

#define PORT 6969        // def port
#define BUFFER_SIZE 1024 // how many bytes to handle at once

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
    NULL};

MYSQL *mysql_real_connect(MYSQL *mysql,
                          const char *host,
                          const char *user,
                          const char *passwd,
                          const char *db,
                          unsigned int port,
                          const char *unix_socket,
                          unsigned long client_flag);

void authenticateSQLDBConnection(const char *host, const char *user, const char *passwd, const char *database)
{
    MYSQL *conn = mysql_init(NULL);
    conn = mysql_real_connect(conn, host, user, passwd, database, 0, NULL, 0);
    if (conn == NULL)
    {
        fprintf(stderr, "MySQL connection failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("MySQL connection successful\n");
    }
}

void runSQLQuery(const char *query)
{
    // the query should be here and do something idk yet tho lol
    printf("Executing SQL Query: %s\n", query);
}

void populateResponseBody(success_response *response, const char *status_code, const char *status_text, const char *bodyContent)
{
    response->status_code = atoi(status_code); // atoi is ascii to int. Ascii TO Int -> ATOI (lol)
    response->status_text = status_text;
    response->content_type = "application/json";
    response->body = bodyContent;
}

// delete existing html file if exists for fresh creation
void deleteHTMLFile(const char *filePath)
{
    FILE *htmlFile = fopen(filePath, "r");
    if (htmlFile)
    {
        fclose(htmlFile);
        remove(filePath);
        perror("Deleting existing HTML file");
        fclose(htmlFile);
        if (fopen(filePath, "r") == NULL)
        {
            printf("Existing HTML file deleted successfully.\n");
        }
        return;
    }
}

void populateNewHTMLFile(const char *filePath, const struct success_response response)
{
    deleteHTMLFile(filePath);

    printf("Creating HTML file: %s\n", filePath);
    FILE *htmlFile = fopen(filePath, "w");
    if (!htmlFile)
    {
        perror("Could not create HTML file");
        return;
    }

    fprintf(htmlFile, "%s", response.body);
    fclose(htmlFile);
    printf("HTML file created successfully.\n");
}

// find html file, read it, send it over.
void showHTML(int clientSocket, const char *filePath, const success_response *response)
{
    populateNewHTMLFile(filePath, *response);

    FILE *htmlFile = fopen(filePath, "r");

    fseek(htmlFile, 0, SEEK_END);
    long fileSize = ftell(htmlFile);
    rewind(htmlFile);

    char httpHeader[BUFFER_SIZE];
    snprintf(httpHeader, sizeof(httpHeader),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             fileSize);

    if (send(clientSocket, httpHeader, strlen(httpHeader), 0) == -1)
    {
        perror("Failed to send HTTP header");
    }
    else
    {
        printf("HTTP header sent successfully.\n");
    }

    // Send the file content
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, htmlFile)) > 0)
    {
        if (send(clientSocket, buffer, bytes_read, 0) == -1)
        {
            perror("Failed to send file content");
            break;
        }
    }
    printf("HTML file sent successfully.\n");

    fclose(htmlFile);
}

int server()
{
    int serverSocket;
    int clientSocket;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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