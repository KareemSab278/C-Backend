// gcc server.c main.c -o backend -lws2_32
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <winsock2.h> // has to be like this on windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
#include <unistd.h>

#define PORT 6969        // def port
#define BUFFER_SIZE 1024 // how many bytes to handle at once

// need to change the way this works. instead of html i want to connect to a DB and serve it to another page. ill need to set endpoints. for now itll just do a simple read.
// dont want to serve any files. ill fun server.c and get the data i need from mysql then send it to the frontend via an endpoint.

void httpServer()
{
}
// find html file, read it, send it over.
void showHTML(int clientSocket, const char *filePath)
{
    printf("Opening HTML file: %s\n", filePath);
    FILE *htmlFile = fopen(filePath, "r");
    if (!htmlFile)
    {
        perror("Could not find the HTML file");
        return;
    }

    // Calculate the file size
    fseek(htmlFile, 0, SEEK_END);
    long fileSize = ftell(htmlFile);
    rewind(htmlFile);

    // Create the HTTP header with Content-Length
    char httpHeader[BUFFER_SIZE];
    snprintf(httpHeader, sizeof(httpHeader),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
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
        showHTML(clientSocket, "index.html");
    }
    close(clientSocket);
    printf("Disconnected\n");

    close(serverSocket);
    return 0;
}