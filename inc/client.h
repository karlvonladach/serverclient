#ifndef CLIENT_H
#define CLIENT_H

#include <windows.h>
#include "msg_q.h"

extern MSG_Q clientMsgQueue;

DWORD WINAPI clientThread(void* args);

#endif