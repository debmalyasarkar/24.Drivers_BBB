#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<cli.h>
#include<appCli.h>

int quitFlag;
pthread_t cliThread;

int main(void)
{
  pthread_create(&cliThread,NULL,cliInterface,NULL);
  while(!quitFlag);
  return 0;
}
