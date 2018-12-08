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
#include "client.h"

#pragma comment(lib, "Ws2_32.lib")

/*****************************************************************************/
/*                             DEFINITIONS                                   */
/*****************************************************************************/

#define DEFAULT_BUFLEN 512

/*****************************************************************************/
/*                           GLOBAL VARIABLES                                */
/*****************************************************************************/

/*****************************************************************************/
/*                           STATIC FUNCTIONS                                */
/*****************************************************************************/

/*****************************************************************************/
/*                           PUBLIC FUNCTIONS                                */
/*****************************************************************************/

DWORD WINAPI clientThread(void* args) 
{
  struct addrinfo *result = NULL, *ptr = NULL, hints;
  char* connectPort = (char*)args;
  SOCKET ConnectSocket = INVALID_SOCKET;
  int recvbuflen = DEFAULT_BUFLEN;
  char *sendbuf = "this is a test";
  char recvbuf[DEFAULT_BUFLEN];
  int iResult;

  printf("start client...\r\n");

  getchar();
 
  ZeroMemory( &hints, sizeof(hints) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Resolve the server address and port
  iResult = getaddrinfo("127.0.0.1", connectPort, &hints, &result);
  if (iResult != 0) {
    printf("getaddrinfo failed: %d\n", iResult);
    WSACleanup();
    return 1;
  }

  // Attempt to connect to the first address returned by
  // the call to getaddrinfo
  ptr=result;

  // Create a SOCKET for connecting to server
  ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (ConnectSocket == INVALID_SOCKET) {
    printf("Error at socket(): %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return 1;
  }

  // Connect to server.
  iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    closesocket(ConnectSocket);
    ConnectSocket = INVALID_SOCKET;
  }

  // Should really try the next address returned by getaddrinfo
  // if the connect call failed
  // But for this simple example we just free the resources
  // returned by getaddrinfo and print an error message

  freeaddrinfo(result);

  if (ConnectSocket == INVALID_SOCKET) {
    printf("Unable to connect to server!\n");
    WSACleanup();
    return 1;
  }

  printf("Connected to server\r\n");

  // Send an initial buffer
  iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
  if (iResult == SOCKET_ERROR) {
    printf("send failed: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
  }

  printf("Bytes Sent: %ld\n", iResult);

  // shutdown the connection for sending since no more data will be sent
  // the client can still use the ConnectSocket for receiving data
  iResult = shutdown(ConnectSocket, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    printf("shutdown failed: %d\n", WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    return 1;
  }

  // Receive data until the server closes the connection
  do {
    iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0)
        printf("Bytes received: %d\n", iResult);
    else if (iResult == 0)
        printf("Connection closed\n");
    else
        printf("recv failed: %d\n", WSAGetLastError());
  } while (iResult > 0);

  // cleanup
  closesocket(ConnectSocket);
  // WSACleanup(); // needed here?

  return 0;
}