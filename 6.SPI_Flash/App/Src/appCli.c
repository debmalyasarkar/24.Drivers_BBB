#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cli.h>
#include <errno.h>
#include <appCli.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../../spi_flash.h"

#define DEVICE_FILE_NAME "/dev/at45db161d" 

#define PAGE_SIZE 512

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
  int ii, retval, count  = 1, size = 0;
  uint8_t rdbuff[PAGE_SIZE] = {0};

  if(argc != 2)
  {
    printf("Usage <CMD> <BytesToRead>\r\n");
    return FAILURE;
  }
  sscanf(argv[1], "%d", &size);
  if((size < 0) || (size > PAGE_SIZE))
  {
    printf("Size is Out of Range\r\n");
    return FAILURE;
  }

  retval = read(fd, rdbuff, size);
  if(retval < 0)
  {
    perror("Read Failed : ");
    return FAILURE;
  }
  printf("File Read Success\n");

  /* Print 16 Values and Go To Next Line */
  for(ii = 0; ii < size; ii++, count++)
  {
    printf("%02x ", rdbuff[ii]);
    if(count == 16)
    {
      printf("\r\n");
      count  = 0;
    }
  }
  printf("\r\n");
  printf("%s\r\n",rdbuff);

  return SUCCESS;
}

int writeFile(int argc,char *argv[])
{
  int retval, size = 0;
  char wrbuff[PAGE_SIZE] = {0};

  if(argc != 2)
  {
    printf("Usage <CMD> <MessageToWrite>\r\n");
    return FAILURE;
  }
  strcpy(wrbuff, argv[1]);

  size = strlen(wrbuff);

  retval = write(fd, wrbuff, size);
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

/* Setting or Getting Configuration Bits */
int ioctlFile(int argc,char *argv[])
{
  unsigned int  option = 0, cmd = 0, val = 0;
 
  if(argc < 2)
  {
    printf("Usage <Cmd> <Option>\n");
    printf("<Option>\n");
    printf("1   = GET_DEVICE_ID\n");
    printf("2   = GET_MAX_PAGES\n");
    printf("3   = GET_PAGE_OFFSET\n");
    printf("4   = SET_PAGE_OFFSET\n");
    printf("5   = ERASE_PAGE\n");
    printf("6   = ERASE_SECTOR\n");
    printf("7   = ERASE_CHIP\n");

    return FAILURE;
  }

  sscanf(argv[1],"%d",&option);
  if((option < 1) || (option > MAX_IOCTL))
  {
    printf("Invalid Option\n");
    return FAILURE;
  }

  if((option == 4) || (option == 5) || (option == 6))
  {
    if(argc != 3)
    {
      printf("Missing Value to Set\r\n");
      return FAILURE;
    }
    sscanf(argv[2], "%u", &val);
  }

  switch(option)
  {
    case 1:
      cmd = GET_DEVICE_ID;
      break;

    case 2:
      cmd = GET_MAX_PAGES;
      break;

    case 3:
      cmd = GET_PAGE_OFFSET;
      break;

    case 4:
      cmd = SET_PAGE_OFFSET;
      if((val < 0) || (val >= 8192))
      {
        printf("Page Number is Out of Range\r\n");
        return FAILURE;
      }
      break;

    case 5:
      cmd = ERASE_PAGE;
      if((val < 0) || (val >= 8192))
      {
        printf("Page Number is Out of Range\r\n");
        return FAILURE;
      }
      break;

    case 6:
      cmd = ERASE_SECTOR;
      if((val < 0) || (val >= 16))
      {
        printf("Sector Number is Out of Range\r\n");
        return FAILURE;
      }
      break;

    case 7:
      cmd = ERASE_CHIP;
      break;
  }

  if(0 > ioctl(fd, cmd, &val))
  {
    perror("IOCTL Failed : ");
    return FAILURE;
  }

  if(option == 1)
    printf("Device ID %x\r\n", val);
  else if(option == 2)
    printf("Max Pages %d\r\n", val);
  else if(option == 3)
    printf("Current Page %d\r\n", val);
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
