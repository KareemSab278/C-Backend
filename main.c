#include "server.h"
#include <stdio.h>
#include <winsock2.h>

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