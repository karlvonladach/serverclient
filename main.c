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
#include "client.h"

#pragma comment(lib, "Ws2_32.lib")

/*****************************************************************************/
/*                             DEFINITIONS                                   */
/*****************************************************************************/

#define MAX_NUM_THREADS 2

/*****************************************************************************/
/*                           GLOBAL VARIABLES                                */
/*****************************************************************************/

WSADATA wsaData;

/*****************************************************************************/
/*                           STATIC FUNCTIONS                                */
/*****************************************************************************/

/*****************************************************************************/
/*                           PUBLIC FUNCTIONS                                */
/*****************************************************************************/

int main() {
  int iResult;
  DWORD threadCnt = 0;
  HANDLE threads[MAX_NUM_THREADS];

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0) {
    printf("WSAStartup failed: %d\n", iResult);
    return 1;
  }

  threads[threadCnt++] = CreateThread(
                NULL,             // default security
                0,                // default stack size
                serverThread,     // name of the thread function
                NULL,             // no thread parameters
                0,                // default startup flags
                NULL );

  threads[threadCnt++] = CreateThread( 
                NULL,             // default security
                0,                // default stack size
                clientThread,     // name of the thread function
                NULL,             // no thread parameters
                0,                // default startup flags
                NULL );

  WaitForMultipleObjects(
                threadCnt,        // number of wait objects
                threads,          // array of wait objects
                TRUE,             // wait for all
                INFINITE);        // timeout

  return 0;
}
