#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include "msg_q.h"

#define MAX_CONNECTIONS 10

extern MSG_Q serverMsgQueue;

enum serverCommands {
  eServerCommandListen,
  eServerCommandStop
};

DWORD WINAPI serverThread(void* args);

#endif