#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <cli.h>
#include <errno.h>
#include <appCli.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../../i2c_eeprom.h"

#define DEVICE_FILE_NAME "/dev/at24c32" 

#define PAGE_SIZE 32
#define MAX_PAGES 128

int fd;

extern int quitFlag;

int openFile(int argc,char *argv[])
{
  fd = open(DEVICE_FILE_NAME, O_RDWR);
  if(fd < 0)
  {
    perror("Open Failed : ");
    return FAILURE;
  }
  printf("File Open Success\n");
  return SUCCESS;
}

int readFile(int argc,char *argv[])
{
  int retval,ii;
  uint8_t rdbuff[PAGE_SIZE] = {0};

  retval = read(fd, rdbuff, PAGE_SIZE);
  if(retval < 0)
  {
    perror("Read Failed : ");
    return FAILURE;
  }
  printf("File Read Success\n");

  for(ii = 0; ii < PAGE_SIZE; ii++)
    printf("%d ",rdbuff[ii]);
  printf("\r\n");

  return SUCCESS;
}

int writeFile(int argc,char *argv[])
{
  int retval,ii;
  uint8_t wrbuff[PAGE_SIZE] = {0};

  for(ii = 0; ii < PAGE_SIZE; ii++)
    wrbuff[ii] = ii;

  retval = write(fd, wrbuff, PAGE_SIZE);
  if(retval < 0)
  {
    perror("Write Failed : ");
    return FAILURE;
  }
  printf("File Write Success\n");

  return SUCCESS;
}

int closeFile(int argc,char *argv[])
{
  if(0 > close(fd))
  {
    perror("Close Failed : ");
    return FAILURE;
  }
  printf("File Close Success\n");
  return SUCCESS;
}

int ioctlFile(int argc,char *argv[])
{
  unsigned int  option = 0, val = 0;
 
  if(argc < 3)
  {
    printf("Usage <Cmd> <Option> <ValueToSet>\n");
    printf("<Option>\n");
    printf("1  = GET_PAGE_OFFSET\n");
    printf("2  = SET_PAGE_OFFSET\n");

    printf("<ValueToSet>\n");
    printf("GET_PAGE_OFFSET - Pass 0\n");
    printf("SET_PAGE_OFFSET - Pass Value\n"); 
    return FAILURE;
  }
  sscanf(argv[1],"%u",&option);
  if(option > 2)
  {
    printf("Invalid Number\n");
    return FAILURE;
  }

  sscanf(argv[2],"%u",&val);
  if(val > MAX_PAGES)
  {
    printf("Out of Range %d\n", MAX_PAGES);
    return FAILURE;
  }  

  if(option == 1)
  {
    if(0 > ioctl(fd, GET_PAGE_OFFSET, &val))
    {
      perror("IOCTL Failed : ");
      return FAILURE;
    }
    printf("Current Page Number %d\r\n",val);
  }
  else if(option == 2)
  {
    if(0 > ioctl(fd, SET_PAGE_OFFSET, &val))
    {
      perror("IOCTL Failed : ");
      return FAILURE;
    }
  }
  return SUCCESS;
}

int quitApp(int argc,char *argv[])
{
  quitFlag = 1;
  return SUCCESS;
}

int dispHlp(int argc,char *argv[])
{
  int ii;
  for(ii = 0; ii < (sizeof(commandTable)/sizeof(cmdFun_t)); ii++)
    printf("%s\t : %s\n",commandTable[ii].cmd,commandTable[ii].hlpStr);
  printf("\n");
  return 0;
}
