
/* I2C RTC is DS 1307 */
#define DEVICE_NAME   "ds1307"

#define MAX_IOCTL 14

#define SUCCESS 0

/* Address of RTC Registers */
#define ADDRESS_SECOND     0x00
#define ADDRESS_MINUTE     0x01
#define ADDRESS_HOUR       0x02
#define ADDRESS_DAY        0x03
#define ADDRESS_DATE       0x04
#define ADDRESS_MONTH      0x05
#define ADDRESS_YEAR       0x06
#define ADDRESS_CONTROL    0x07

/* RTC Private Structure */
typedef struct _rtc_t
{
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t date;
  uint8_t month;
  uint8_t year;
  uint8_t control;
  
  uint8_t ch;
  uint8_t mode;
  uint8_t meridiem;
  uint8_t out;
  uint8_t sqwe;
  uint8_t rs;
}rtc_t;

/* IOCTL Macros for RTC Configuration Operations */
#define RTC_MAGIC 'D'

#define SET_CLOCK_HALT _IOW(RTC_MAGIC,1,uint8_t)
#define SET_MERIDIEM   _IOW(RTC_MAGIC,2,uint8_t)
#define SET_HOUR_MODE  _IOW(RTC_MAGIC,3,uint8_t)
#define SET_OUT        _IOW(RTC_MAGIC,4,uint8_t)
#define SET_SQWE       _IOW(RTC_MAGIC,5,uint8_t)
#define SET_RS1        _IOW(RTC_MAGIC,6,uint8_t)
#define SET_RS0        _IOW(RTC_MAGIC,7,uint8_t)

#define GET_CLOCK_HALT _IOR(RTC_MAGIC,8 ,uint8_t)
#define GET_MERIDIEM   _IOR(RTC_MAGIC,9 ,uint8_t)
#define GET_HOUR_MODE  _IOR(RTC_MAGIC,10,uint8_t)
#define GET_OUT        _IOR(RTC_MAGIC,11,uint8_t)
#define GET_SQWE       _IOR(RTC_MAGIC,12,uint8_t)
#define GET_RS1        _IOR(RTC_MAGIC,13,uint8_t)
#define GET_RS0        _IOR(RTC_MAGIC,14,uint8_t)

