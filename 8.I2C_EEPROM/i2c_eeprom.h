
/* IOCTL Macros for EEPROM Operations */
#define EEPROM_MAGIC 'D'

#define GET_PAGE_OFFSET _IOR(EEPROM_MAGIC,1,uint8_t)
#define SET_PAGE_OFFSET _IOW(EEPROM_MAGIC,2,uint8_t)
