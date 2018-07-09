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
#include "../../i2c_rtc.h"

#define DEVICE_FILE_NAME "/dev/ds1307" 

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
  int retval;
  uint8_t rdbuff[8] = {0};
  char str[12] = {0};  

  retval = read(fd, rdbuff, 8);
  if(retval < 0)
  {
    perror("Read Failed : ");
    return FAILURE;
  }
  printf("File Read Success\n");

  sprintf(str, "%02x:%02x:%02x %02x/%02x/%02x", 
    rdbuff[2], rdbuff[1], rdbuff[0], rdbuff[4], rdbuff[5], rdbuff[6]);
  
  printf("Current Time = %s\r\n",str); 

  return SUCCESS;
}

int parseRtcStr(char *tmDtStr, uint8_t *buff)
{
  int ii;
  uint8_t tmdt[6] = {0};

  while(*tmDtStr == ' ')
    tmDtStr++;

  /* Check if all characters are within valid range */
  for(ii = 0; ii < 12; ii++)
  {
    if((tmDtStr[ii] < '0') || (tmDtStr[ii] > '9'))
    {
      puts("Invalid time format\r\n");
      return FAILURE;
    }
  }
  /* HH_MM_SS_DD_MM_YY Format */ 
  tmdt[0] = ((tmDtStr[0]  - '0') << 4) | (tmDtStr[1]  - '0');
  tmdt[1] = ((tmDtStr[2]  - '0') << 4) | (tmDtStr[3]  - '0');
  tmdt[2] = ((tmDtStr[4]  - '0') << 4) | (tmDtStr[5]  - '0');
  tmdt[3] = ((tmDtStr[6]  - '0') << 4) | (tmDtStr[7]  - '0');
  tmdt[4] = ((tmDtStr[8]  - '0') << 4) | (tmDtStr[9]  - '0');
  tmdt[5] = ((tmDtStr[10] - '0') << 4) | (tmDtStr[11] - '0');

  /* Rearrange to SS_MM_HH_DAY_DD_MM_YY_CTRL Format as per Device Registers */
  buff[0] = tmdt[2];
  buff[1] = tmdt[1];
  buff[2] = tmdt[0];
  buff[3] = 1;
  buff[4] = tmdt[3];
  buff[5] = tmdt[4];
  buff[6] = tmdt[5];
  buff[7] = 0;

  return SUCCESS;
}

int writeFile(int argc,char *argv[])
{
  int retval;
  uint8_t wrbuff[8] = {0};

  if(argc < 2)
  {
    printf("Enter Time & Date in <hhmmssddmmyy> Format\r\n");
    return FAILURE;
  }

  if(SUCCESS != parseRtcStr(argv[1], wrbuff))
    return FAILURE;

  retval = write(fd, wrbuff, sizeof(wrbuff));
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
    printf("1   = SET_CLOCK_HALT\n");
    printf("2   = SET_MERIDIEM\n");
    printf("3   = SET_HOUR_MODE\n");
    printf("4   = SET_OUT\n");
    printf("5   = SET_SQWE\n");
    printf("6   = SET_RS1\n");
    printf("7   = SET_RS0\n");

    printf("8   = GET_CLOCK_HALT\n");
    printf("9   = GET_MERIDIEM\n");
    printf("10  = GET_HOUR_MODE\n");
    printf("11  = GET_OUT\n");
    printf("12  = GET_SQWE\n");
    printf("13  = GET_RS1\n");
    printf("14  = GET_RS0\n");

    return FAILURE;
  }

  sscanf(argv[1],"%d",&option);
  if((option < 1) || (option > 14))
  {
    printf("Invalid Option\n");
    return FAILURE;
  }

  if(option < 8)
  {
    if(argc != 3)
    {
      printf("Missing Value to Set\r\n");
      return FAILURE;
    }
    sscanf(argv[2], "%d", &val);
    if((val != 0) && (val != 1))
    {
      printf("Value can be only 1 or 0\r\n");
      return FAILURE;
    }
  }

  switch(option)
  {
/* Set() */
    case 1:
      cmd = SET_CLOCK_HALT;
      break;
    case 2:
      cmd = SET_MERIDIEM;
      break;
    case 3:
      cmd = SET_HOUR_MODE;
      break;
    case 4:
      cmd = SET_OUT;
      break;
    case 5:
      cmd = SET_SQWE;
      break;
    case 6:
      cmd = SET_RS1;
      break;
    case 7:
      cmd = SET_RS0;
      break;
/* Get() */
    case 8:
      cmd = GET_CLOCK_HALT;
      break;
    case 9:
      cmd = GET_MERIDIEM;
      break;
    case 10:
      cmd = GET_HOUR_MODE;
      break;
    case 11:
      cmd = GET_OUT;
      break;
    case 12:
      cmd = GET_SQWE;
      break;
    case 13:
      cmd = GET_RS1;
      break;
    case 14:
      cmd = GET_RS0;
      break;
  }

  if(0 > ioctl(fd, cmd, &val))
  {
    perror("IOCTL Failed : ");
    return FAILURE;
  }

  if(option > 7)
    printf("Current Value %u\r\n",val);
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
