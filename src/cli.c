#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WINVER
#define WINVER _WIN32_WINNT_WIN7
#endif

#include <windows.h>
#include <stdio.h>
#include "client.h"
#include "server.h"
#include "cli.h"

/*****************************************************************************/
/*                             DEFINITIONS                                   */
/*****************************************************************************/

typedef struct {
  char* module;
  char* command;
  char* params;
} COMMANDLINE;

typedef struct {
  char* commandName;
  int   commandType;
} COMMAND;

typedef struct {
  char*  moduleName;
  MSG_Q* moduleMsgQueue;
  int    moduleNumCommands;
  COMMAND moduleCommands[];
} MODULE;

/*****************************************************************************/
/*                           GLOBAL VARIABLES                                */
/*****************************************************************************/

static MODULE serverModule = {"server", &serverMsgQueue, 2, {{"listen",  eServerCommandListen},
                                                             {"stop", eServerCommandStop}}};
static MODULE clientModule = {"client", &clientMsgQueue, 2, {{"connect", eClientCommandConnect}, 
                                                             {"stop", eClientCommandStop}}};

static MODULE* modules[] = {&serverModule, &clientModule}; 

/*****************************************************************************/
/*                           STATIC FUNCTIONS                                */
/*****************************************************************************/

/*****************************************************************************/
/*                           PUBLIC FUNCTIONS                                */
/*****************************************************************************/

DWORD WINAPI cliThread(void* args) 
{
  char command[100];
  COMMANDLINE currentCommandLine;
  int numModules = sizeof(modules)/sizeof(MODULE*);

  while (1) {
    printf("> ");
    gets(command);

    if (strcmp(command,"exit") == 0) {
      MsgQueueSendMessage(&serverMsgQueue,eServerCommandStop,0,NULL);
      MsgQueueSendMessage(&clientMsgQueue,eClientCommandStop,0,NULL);

      return 0;
    }

    currentCommandLine.module  = command;
    currentCommandLine.command = NULL;
    currentCommandLine.params  = NULL;
    for (char* p=command; *p; p++) {
      if (*p == '\r' || *p == '\n') {
          *p == '\0';
          break;
      }
      if (*p == ' ' && currentCommandLine.command == NULL) {
          *p = '\0';
          currentCommandLine.command = p+1;
      } 
      else
      if (*p == ' ' && currentCommandLine.params == NULL) {
          *p = '\0';
          currentCommandLine.params = p+1;
      } 
    }

    int i, j;
    for (i=0; i < numModules; i++) {
      if (strcmp(currentCommandLine.module,modules[i]->moduleName) == 0) {
        if (currentCommandLine.command == NULL) {
          break;
        }
        for (j=0; j < modules[i]->moduleNumCommands; j++) {
          if (strcmp(currentCommandLine.command,modules[i]->moduleCommands[j].commandName) == 0) {
            printf("send command #%s to %s\r\n",modules[i]->moduleCommands[j].commandName,modules[i]->moduleName);
            MsgQueueSendMessage(modules[i]->moduleMsgQueue,
                                modules[i]->moduleCommands[j].commandType,
                                (currentCommandLine.params == NULL) ? 0 : strlen(currentCommandLine.params),
                                currentCommandLine.params);
            break;
          }
        }
        if (j == modules[i]->moduleNumCommands) {
          printf("command does not exist\r\n");
        }
        break;
      }
    }
    if (i == numModules) {
      printf("module does not exist\r\n");
    }
  }

  return 0;
}