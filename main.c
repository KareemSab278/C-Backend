// #include <stdio.h>
// #include <winsock2.h>
// #include "sqlite3.h"
// #include "server.h"

// int main()
// {
//     WSADATA wsaData;
//     if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
//     {
//         perror("WSAStartup failed");
//         exit(EXIT_FAILURE);
//     }

//     server();

//     WSACleanup();
//     return 0;
// }

// linux:
#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "server.h"

int main()
{
    server();
    return 0;
}