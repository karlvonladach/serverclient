#ifndef WINVER
#define WINVER _WIN32_WINNT_WIN7
#endif

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "msg_q.h"

int MsgQueueSendMessage(MSG_Q* msgq, int msgType, int msgLen, void* msgData)
{
  MSG_Q_ELEMENT item;

  if (msgq->BufferSize == 0) {
      return 1;
  }

  //copy data
  item.msgType = msgType;
  item.msgLen  = msgLen;
  item.msgData = NULL;
  if (msgLen > 0) {
    item.msgData = malloc(msgLen);
    if (item.msgData == NULL) {
      return 1;
    }
    memcpy(item.msgData, msgData, msgLen);
  }

  EnterCriticalSection (&msgq->BufferLock);

  while (msgq->QueueSize == msgq->BufferSize)
  {
    // Buffer is full - sleep so consumers can get items.
    SleepConditionVariableCS(&msgq->BufferNotFull, &msgq->BufferLock, INFINITE);
  }

  // Insert the item at the end of the queue and increment size.
  msgq->Buffer[(msgq->QueueStartOffset + msgq->QueueSize) % msgq->BufferSize] = item;
  msgq->QueueSize++;
  msgq->TotalItemsProduced++;

  LeaveCriticalSection (&msgq->BufferLock);

  // If a consumer is waiting, wake it.
  WakeConditionVariable (&msgq->BufferNotEmpty);

  return 0;
}

int MsgQueueWaitMessage(MSG_Q* msgq, int* msgType, int* msgLen, void* msgData)
{
  MSG_Q_ELEMENT item;

  if (msgq->BufferSize == 0) {
      return 1;
  }

  EnterCriticalSection (&msgq->BufferLock);

  while (msgq->QueueSize == 0)
  {
    // Buffer is empty - sleep so producers can create items.
    SleepConditionVariableCS(&msgq->BufferNotEmpty, &msgq->BufferLock, INFINITE);
  }

  // Consume the first available item.
  item = msgq->Buffer[msgq->QueueStartOffset];
  msgq->QueueSize--;
  msgq->QueueStartOffset++;
  msgq->TotalItemsConsumed++;

  if (msgq->QueueStartOffset == msgq->BufferSize)
  {
    msgq->QueueStartOffset = 0;
  }

  //copy data
  *msgType = item.msgType;
  *msgLen  = item.msgLen;
  if (item.msgLen > 0) {
    if (msgData == NULL) {
      return 1;
    } 
    else {
      memcpy(msgData, item.msgData, item.msgLen);
    }
    free(item.msgData);
  }

  LeaveCriticalSection (&msgq->BufferLock);

  // If a producer is waiting, wake it.
  WakeConditionVariable (&msgq->BufferNotFull);

  return 0;
}

int MsgQueueInitialize(MSG_Q* msgq, ULONG bufferSize)
{
  msgq->BufferSize = bufferSize; 

  msgq->Buffer = malloc(bufferSize * sizeof(MSG_Q_ELEMENT));
  if (msgq->Buffer == NULL) {
    return 1;
  }

  msgq->QueueSize = 0;
  msgq->QueueStartOffset = 0;
  msgq->TotalItemsProduced = 0;
  msgq->TotalItemsConsumed = 0;

  InitializeConditionVariable (&msgq->BufferNotEmpty);
  InitializeConditionVariable (&msgq->BufferNotFull);
  InitializeCriticalSection (&msgq->BufferLock);

  return 0;
}

int MsgQueueDestroy(MSG_Q* msgq)
{
  while (msgq->QueueSize > 0) {
    if (msgq->Buffer[msgq->QueueStartOffset].msgData != NULL) {
        free(msgq->Buffer[msgq->QueueStartOffset].msgData);
    }
    msgq->QueueSize--;
    msgq->QueueStartOffset++;
    msgq->TotalItemsConsumed++;
    if (msgq->QueueStartOffset == msgq->BufferSize)
    {
      msgq->QueueStartOffset = 0;
    }
  }
  msgq->BufferSize = 0;
  free(msgq->Buffer);
}