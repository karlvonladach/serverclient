#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WINVER
#define WINVER _WIN32_WINNT_WIN7
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

/*****************************************************************************/
/*                             DEFINITIONS                                   */
/*****************************************************************************/

#define DEFAULT_PORT "27015"

/*****************************************************************************/
/*                           GLOBAL VARIABLES                                */
/*****************************************************************************/

/*****************************************************************************/
/*                           STATIC FUNCTIONS                                */
/*****************************************************************************/

/*****************************************************************************/
/*                           PUBLIC FUNCTIONS                                */
/*****************************************************************************/

DWORD WINAPI serverThread(void* args) 
{
  int iResult;
  struct addrinfo *result = NULL, *ptr = NULL, hints;
  SOCKET ListenSocket = INVALID_SOCKET;

  printf("start server...\r\n");

  ZeroMemory(&hints, sizeof (hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags    = AI_PASSIVE;

  // Resolve the local address and port to be used by the server
  iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
  if (iResult != 0) {
    printf("getaddrinfo failed: %d\n", iResult);
    WSACleanup();
    return 1;
  }

  // Create a SOCKET for the server to listen for client connections
  ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (ListenSocket == INVALID_SOCKET) {
    printf("Error at socket(): %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return 1;
  }

  WSACleanup();
  return 0;
}