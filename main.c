#include "server.h"
#include <stdio.h>
#include <winsock2.h>
#include "sqlite3.h"

// gcc server.c main.c sqlite3.c -o backend -lws2_32

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    server();

    WSACleanup();
    return 0;
}