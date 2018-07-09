/* SPI Flash Memory is AT45DB161D */
#define DEVICE_NAME   "at45db161d"

#define MAX_IOCTL 7

#define SUCCESS 0

#define DUMMY 0xA5

/* Commands */
#define FLASH_CONTINUOUS_ARRAY_READ_LEGACY               0xE8
#define FLASH_CONTINUOUS_ARRAY_READ_HF                   0x0B
#define FLASH_CONTINUOUS_ARRAY_READ_LF                   0x03
#define FLASH_MAIN_MEMORY_PAGE_READ                      0xD2
#define FLASH_BUFFER1_READ                               0XD4
#define FLASH_BUFFER1_READ_LF                            0XD1
#define FLASH_BUFFER2_READ                               0XD3
#define FLASH_BUFFER2_READ_LF                            0XD6
#define FLASH_BUFFER1_WRITE                              0x84
#define FLASH_BUFFER2_WRITE                              0x87
#define FLASH_BUFFER1_TO_MAIN_MEMORY_WRITE_WITH_ERASE    0x83
#define FLASH_BUFFER2_TO_MAIN_MEMORY_WRITE_WITH_ERASE    0x86
#define FLASH_BUFFER1_TO_MAIN_MEMORY_WRITE_WITHOUT_ERASE 0x88
#define FLASH_BUFFER2_TO_MAIN_MEMORY_WRITE_WITHOUT_ERASE 0x89
#define FLASH_PAGE_ERASE                                 0x81
#define FLASH_BLOCK_ERASE                                0x50
#define FLASH_SECTOR_ERASE                               0x7C
#define FLASH_BULK_ERASE1                                0xC7
#define FLASH_BULK_ERASE2                                0x94
#define FLASH_BULK_ERASE3                                0x80
#define FLASH_BULK_ERASE4                                0x9A
#define FLASH_MAIN_MEMORY_PAGE_PROGRAM_THROUGH_BUFFER1   0x82
#define FLASH_MAIN_MEMORY_PAGE_PROGRAM_THROUGH_BUFFER2   0x85
#define FLASH_ENABLE_SECTOR_PROTECT1                     0x3D
#define FLASH_ENABLE_SECTOR_PROTECT2                     0x2A
#define FLASH_ENABLE_SECTOR_PROTECT3                     0x7F
#define FLASH_ENABLE_SECTOR_PROTECT4                     0xA9
#define FLASH_DISABLE_SECTOR_PROTECT1                    0x3D
#define FLASH_DISABLE_SECTOR_PROTECT2                    0x2A
#define FLASH_DISABLE_SECTOR_PROTECT3                    0x7F
#define FLASH_DISABLE_SECTOR_PROTECT4                    0x9A
#define FLASH_ERASE_SECTOR_PROTECT_REGISTER1             0x3D
#define FLASH_ERASE_SECTOR_PROTECT_REGISTER2             0x2A
#define FLASH_ERASE_SECTOR_PROTECT_REGISTER3             0x7F
#define FLASH_ERASE_SECTOR_PROTECT_REGISTER4             0xCF
#define FLASH_PROGRAM_SECTOR_PROTECT_REGISTER1           0x3D
#define FLASH_PROGRAM_SECTOR_PROTECT_REGISTER2           0x2A
#define FLASH_PROGRAM_SECTOR_PROTECT_REGISTER3           0x7F
#define FLASH_PROGRAM_SECTOR_PROTECT_REGISTER4           0xFC
#define FLASH_READ_SECTOR_PROTECT_REGISTER1              0x32
#define FLASH_READ_SECTOR_PROTECT_REGISTER2              DUMMY
#define FLASH_READ_SECTOR_PROTECT_REGISTER3              DUMMY
#define FLASH_READ_SECTOR_PROTECT_REGISTER4              DUMMY
#define FLASH_SECTOR_LOCKDOWN1                           0x3D
#define FLASH_SECTOR_LOCKDOWN2                           0x2A
#define FLASH_SECTOR_LOCKDOWN3                           0x7F
#define FLASH_SECTOR_LOCKDOWN4                           0x30
#define FLASH_READ_SECTOR_LOCKDOWN_REGISTER1             0x35
#define FLASH_READ_SECTOR_LOCKDOWN_REGISTER2             DUMMY
#define FLASH_READ_SECTOR_LOCKDOWN_REGISTER3             DUMMY
#define FLASH_READ_SECTOR_LOCKDOWN_REGISTER4             DUMMY
#define FLASH_PROGRAM_SECURITY_REGISTER1                 0x9B
#define FLASH_PROGRAM_SECURITY_REGISTER2                 0x00
#define FLASH_PROGRAM_SECURITY_REGISTER3                 0x00
#define FLASH_PROGRAM_SECURITY_REGISTER4                 0x00
#define FLASH_READ_SECURITY_REGISTER1                    0x77
#define FLASH_READ_SECURITY_REGISTER2                    DUMMY
#define FLASH_READ_SECURITY_REGISTER3                    DUMMY
#define FLASH_READ_SECURITY_REGISTER4                    DUMMY
#define FLASH_TRANSFER_MAIN_MEMORY_PAGE_TO_BUFFER1       0x53
#define FLASH_TRANSFER_MAIN_MEMORY_PAGE_TO_BUFFER2       0x55
#define FLASH_COMPARE_MAIN_MEMORY_PAGE_WITH_BUFFER1      0x60
#define FLASH_COMPARE_MAIN_MEMORY_PAGE_WITH_BUFFER2      0x61
#define FLASH_AUTO_PAGE_REWRITE_BUFFER1                  0x58
#define FLASH_AUTO_PAGE_REWRITE_BUFERF2                  0x59
#define FLASH_STATUS_REGISTER_READ                       0xD7
#define FLASH_DEEP_POWER_DOWN                            0xB9
#define FLASH_DEEP_POWER_DOWN_RESUME                     0xAB
#define FLASH_POWER_OF_TWO_PAGE_SIZE1                    0x3D
#define FLASH_POWER_OF_TWO_PAGE_SIZE2                    0x2A
#define FLASH_POWER_OF_TWO_PAGE_SIZE3                    0x80
#define FLASH_POWER_OF_TWO_PAGE_SIZE4                    0xA6
#define FLASH_MANUFACTURER_DEVICE_ID_READ                0x9F

/* IOCTL Macros for RTC Configuration Operations */
#define SPI_MAGIC 'D'

#define GET_DEVICE_ID   _IOR(SPI_MAGIC,1,uint8_t)
#define GET_MAX_PAGES   _IOR(SPI_MAGIC,2,uint8_t)
#define GET_PAGE_OFFSET _IOR(SPI_MAGIC,3,uint8_t)
#define SET_PAGE_OFFSET _IOW(SPI_MAGIC,4,uint8_t)
#define ERASE_PAGE      _IOW(SPI_MAGIC,5,uint8_t)
#define ERASE_SECTOR    _IOW(SPI_MAGIC,6,uint8_t)
#define ERASE_CHIP      _IOW(SPI_MAGIC,7,uint8_t)

