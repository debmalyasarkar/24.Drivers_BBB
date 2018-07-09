/*
 * cli.h
 *
 *  Created on: 14-Feb-2017
 *      Author: dev
 */

#ifndef INC_CLI_H_
#define INC_CLI_H_

#define CMD_MAX_LEN  32

typedef struct cmdFun
{
  char cmd[CMD_MAX_LEN];
  int (*fun) (int,char **);
  char *hlpStr;
} cmdFun_t;

extern cmdFun_t commandTable[7];

void * cliInterface(void *arg);


#endif /* INC_CLI_H_ */
