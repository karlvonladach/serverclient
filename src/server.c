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
#include "server.h"

#pragma comment(lib, "Ws2_32.lib")

/*****************************************************************************/
/*                             DEFINITIONS                                   */
/*****************************************************************************/

#define DEFAULT_BUFLEN 512

/*****************************************************************************/
/*                           GLOBAL VARIABLES                                */
/*****************************************************************************/

MSG_Q  serverMsgQueue;
SOCKET gClientSockets[MAX_CONNECTIONS];

/*****************************************************************************/
/*                           STATIC FUNCTIONS                                */
/*****************************************************************************/

DWORD WINAPI serverConnectionThread(void* args) 
{
  int iResult;
  SOCKET ClientSocket = *((SOCKET*)args);
  char recvbuf[DEFAULT_BUFLEN];
  int iSendResult;
  int recvbuflen = DEFAULT_BUFLEN;

  printf("New connection opened\r\n");

  // Receive until the peer shuts down the connection
  do {
    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        printf("Bytes received: %d\n", iResult);

        // Echo the buffer back to the sender
        iSendResult = send(ClientSocket, recvbuf, iResult, 0);
        if (iSendResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        printf("Bytes sent: %d\n", iSendResult);
    } 
    else if (iResult == 0) {
        printf("Connection closing...\n");
    } 
    else {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
  } while (iResult > 0);

  // shutdown the send half of the connection since no more data will be sent
  iResult = shutdown(ClientSocket, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    printf("shutdown failed: %d\n", WSAGetLastError());
    closesocket(ClientSocket);
    WSACleanup();
    return 1;
  }

  // cleanup
  closesocket(ClientSocket);

  // WSACleanup(); // needed here?

  printf("Connection closed\r\n");

  return 0;

}

/*****************************************************************************/
/*                           PUBLIC FUNCTIONS                                */
/*****************************************************************************/

DWORD WINAPI serverThread(void* args) 
{
  int iResult;
  struct addrinfo *result = NULL, *ptr = NULL, hints;
  SOCKET ListenSocket = INVALID_SOCKET;
  char* listenPort = (char*)args;
  int numActiveConnections = 0;
  HANDLE connectionThreads[MAX_CONNECTIONS];
  DWORD dwWaitResult;

  printf("start server...\r\n");

  ZeroMemory(&hints, sizeof (hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags    = AI_PASSIVE;

  // Resolve the local address and port to be used by the server
  iResult = getaddrinfo(NULL, listenPort, &hints, &result);
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

  // Setup the TCP listening socket
  iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }

  freeaddrinfo(result);

  // Start listening on socket
  if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
    printf( "Listen failed with error: %ld\n", WSAGetLastError() );
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }

  printf("Server waiting for connections...\r\n");

  while (1) {
    if (numActiveConnections < MAX_CONNECTIONS) {
      gClientSockets[numActiveConnections] = INVALID_SOCKET;
      // Accept a client socket
      gClientSockets[numActiveConnections] = accept(ListenSocket, NULL, NULL);
      if (gClientSockets[numActiveConnections] == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
      }
      // Start new thread for the connection
      connectionThreads[numActiveConnections] = CreateThread(
              NULL,
              0,
              serverConnectionThread,
              (void*)&gClientSockets[numActiveConnections],  
              0,
              NULL );
      numActiveConnections++;
    } 
    else {
      // Wait for any open thread to finish
      dwWaitResult = WaitForMultipleObjects(
              numActiveConnections,
              connectionThreads,
              FALSE,  // wait for any
              INFINITE);
      // Remove thread from the list
      if ( dwWaitResult >= WAIT_OBJECT_0 && dwWaitResult < (WAIT_OBJECT_0 + numActiveConnections) ) {
        int index = dwWaitResult - WAIT_OBJECT_0;
        CloseHandle(connectionThreads[index]);
        numActiveConnections--;
        for (int i=index; i < numActiveConnections; i++) {
          connectionThreads[i] = connectionThreads[i+1];
          gClientSockets[i]    = gClientSockets[i+1];
        }
      }
      else {
        printf("Wait failed\r\n");
      }
    }
  }

  WSACleanup();
  return 0;
}