#ifndef SERVER_H
#define SERVER_H

#include <windows.h>

#define MAX_CONNECTIONS 10

DWORD WINAPI serverThread(void* args);

#endif