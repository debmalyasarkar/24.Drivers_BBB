#include<stdio.h>
#include<string.h>
#include<appCli.h>
#include<cli.h>

cmdFun_t commandTable[] =
{
  {"o", openFile,          "Open Command"}, 
  {"w", writeFile,         "Write Command"},
  {"r", readFile,          "Read Command"},
  {"c", closeFile,         "Close Command"},
  {"i", ioctlFile,         "IOCTL Command"},
  {"q", quitApp,           "Quit Application"},
  {"h", dispHlp,           "Display User Commands"}
};

void * cliInterface(void *arg)
{
  char *tokens[10];
  int ii,tkncnt;
  char cmd[CMD_MAX_LEN];

  printf("Application CLI v1.00\n");

  while(1)
  {
    printf("cli> ");
    fgets(cmd,CMD_MAX_LEN,stdin);

    tkncnt = 0;
    tokens[tkncnt] = (char *)strtok(cmd, " \n");

    if(!tokens[tkncnt])
      continue;

    while(tokens[tkncnt])
      tokens[++tkncnt] = (char *)strtok(NULL, " \n");

    for(ii = 0; ii < (sizeof(commandTable)/sizeof(cmdFun_t)); ii++)
    {
      if(0 == strcmp(commandTable[ii].cmd,tokens[0]))
        break;
    }

    if(ii == (sizeof(commandTable)/sizeof(cmdFun_t)))
      printf("Invalid Command. Press h for valid commands.\n");
    else
      commandTable[ii].fun(tkncnt,tokens);
  }
}
