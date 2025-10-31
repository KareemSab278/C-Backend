// gcc server.c main.c -o backend -lws2_32
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <winsock2.h> // has to be like this on windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
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
        htmlFile = fopen(filePath, "r");

        if (htmlFile == NULL)
        {
            printf("Existing HTML file deleted successfully.\n");
        }
        return;
    }
}

void createHTMLFile(const char *filePath, const struct success_response response)
{
    deleteHTMLFile(filePath);

    printf("Creating HTML file: %s\n", filePath);
    FILE *htmlFile = fopen(filePath, "w");
    if (!htmlFile)
    {
        perror("Could not create HTML file");
        return;
    }

    // Write the response body to the HTML file
    fprintf(htmlFile, "%s", response.body);
    fclose(htmlFile);
    printf("HTML file created successfully.\n");
}

// find html file, read it, send it over.
void showHTML(int clientSocket, const char *filePath, const success_response *response)
{

    createHTMLFile(filePath, *response);

    FILE *htmlFile = fopen(filePath, "r");

    // Calculate the file size
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

    // Send the HTTP header
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