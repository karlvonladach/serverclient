#ifndef MSG_Q_H
#define MSG_Q_H

#include <windows.h>

typedef struct {
  int msgType;
  int msgLen;
  void *msgData;
} MSG_Q_ELEMENT;

typedef struct {
  ULONG BufferSize;
  MSG_Q_ELEMENT *Buffer;
  ULONG QueueSize;
  ULONG QueueStartOffset;
  ULONG TotalItemsProduced;
  ULONG TotalItemsConsumed;
  CONDITION_VARIABLE BufferNotEmpty;
  CONDITION_VARIABLE BufferNotFull;
  CRITICAL_SECTION   BufferLock;
} MSG_Q;

int MsgQueueSendMessage(MSG_Q* msgq, int msgType,  int msgLen,  void* msgData);
int MsgQueueWaitMessage(MSG_Q* msgq, int* msgType, int* msgLen, void* msgData);
int MsgQueueInitialize(MSG_Q* msgq, ULONG bufferSize);
int MsgQueueDestroy(MSG_Q* msgq);

#endif