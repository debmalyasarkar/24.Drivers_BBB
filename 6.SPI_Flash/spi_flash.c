/* Driver for Interfacing Atmel AT45DB161D SPI DataFlash 
   with spi0 SPI Host Controller Interface of BeagleBone Black */
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/property.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include "spi_flash.h"

/* 
   This SPI Driver Supports the 16Mb and 32 MB SPI FLASH memories.
   16Mb = 16MegaBit = 2MegaByte Organized Into 4096 Pages as follows  
   1 Chip   = 16 Sectors
   1 Sector = 32  Blocks
   1 Block  = 8   Pages
   1 Page   = 512 Bytes
     
   32Mb = 32MegaBit = 4MegaByte Organized Into 8192 Pages as follows
   1 Chip   = 16 Sectors
   1 Sector = 64  Blocks
   1 Block  = 8   Pages
   1 Page   = 512 Bytes
*/
struct spi_flash_prv
{
  struct spi_device *spidev;
  unsigned int inuse;
  unsigned int reg;
  unsigned int page_size;
  unsigned int address_width;
  unsigned int spi_max_frequency;
  unsigned int device_id;
  unsigned int max_pages;
  unsigned int current_page;
};

struct spi_flash_prv *prv = NULL;

static int get_device_properties(void)
{
  /* Read and Print SPI Device Properties from the Device Tree Node */
  if(0 != device_property_read_u32(&prv->spidev->dev, "reg", &prv->reg))
  {
    pr_info("Missing \"reg\" property\r\n");
    return -ENODEV;
  }
  if(0 != device_property_read_u32(&prv->spidev->dev, "pagesize", &prv->page_size))
  {
    pr_info("Missing \"pagesize\" property\r\n");
    return -ENODEV;
  }
  if(0 != device_property_read_u32(&prv->spidev->dev, "address-width", &prv->address_width))
  {
    pr_info("Missing \"address-width\" property\r\n");
    return -ENODEV;
  } 
  if(0 != device_property_read_u32(&prv->spidev->dev, "spi-max-frequency", &prv->spi_max_frequency))
  {
    pr_info("Missing \"spi-max-frequency\" property\r\n");
    return -ENODEV;
  }

  pr_info("Reg Address   = %d\r\n", prv->reg);
  pr_info("Page Size     = %d bytes\r\n", prv->page_size);
  pr_info("Address Width = %d bits\r\n", prv->address_width);
  pr_info("Max Frequency = %d Hz\r\n", prv->spi_max_frequency);

  return SUCCESS;
}

/* Read Manufacturer Device ID and Get Chip Information */
static unsigned int get_device_id(void)
{
  uint8_t cmd[1];
  uint8_t devInfo[4] = {0}, capacity = 0;  

  cmd[0] =  FLASH_MANUFACTURER_DEVICE_ID_READ;

  spi_write_then_read(prv->spidev, cmd, 1, devInfo, 4);

  if(devInfo[0] == 0x1F &&(devInfo[1] & 0xE0) == 0x20)
  {
    pr_info("Valid SPI Flash\r\n");	
    capacity=devInfo[1] & 0x1F;
    if(capacity == 0x06)      //16Mbit
    {
      pr_info("16Mbit Capacity\r\n");
      prv->max_pages = 4096;
    }
    else if(capacity == 0x07) //32Mbit
    {
      pr_info("32Mbit Capacity\r\n");
      prv->max_pages = 8192;
    }
    else
    {	
      pr_info("Invalid SPI Flash Capacity\r\n");
      return -EINVAL;
    }
  }

  prv->device_id = (devInfo[0] << 24) | (devInfo[1] << 16) | 
                   (devInfo[2] << 8) | (devInfo[3] << 0);
  pr_info("Device ID %x\n", prv->device_id);

  return SUCCESS;
}

/* Setting Page Size as 512 */
static unsigned int set_page_size(void)
{
  unsigned int status;
  uint8_t buff[4];

  buff[0] = FLASH_POWER_OF_TWO_PAGE_SIZE1;
  buff[1] = FLASH_POWER_OF_TWO_PAGE_SIZE2;
  buff[2] = FLASH_POWER_OF_TWO_PAGE_SIZE3; 
  buff[3] = FLASH_POWER_OF_TWO_PAGE_SIZE4;
 
  status = spi_write(prv->spidev, buff, 4);

  return status;
}

/* Single Page Erase */
static unsigned int erase_page(unsigned int page_no)
{
  uint32_t addr, retval;
  uint8_t  cmd[4] = {0};
  
  cmd[0] = FLASH_PAGE_ERASE;
  /* Bits 0 - 8  (9  Bits) --> Address 512  bytes in a page */
  /* Bits 9 - 21 (13 Bits) --> Address 8192 pages */
  addr = page_no << 9;
  cmd[1] = ((addr >> 16) & 0xFF);
  cmd[2] = ((addr >> 8)  & 0xFF);
  cmd[3] = ((addr >> 0)  & 0xFF);

  retval = spi_write(prv->spidev, cmd, 4);
  if(0 != retval)
  {
    pr_info("SPI Failed\r\n");
    return retval;
  }
  return SUCCESS;
}

/* Sector Erase */
static unsigned int erase_sector(unsigned int sector_no)
{
  uint32_t addr, retval;
  uint8_t  cmd[4] = {0};

  cmd[0] = FLASH_SECTOR_ERASE;
  /* Bits 0 - 8  (9  Bits) --> Address 512  bytes in a page */
  /* Bits 9 - 21 (13 Bits) --> Address 8192 pages */
  addr = sector_no << 9;
  cmd[1] = ((addr >> 16) & 0xFF);
  cmd[2] = ((addr >> 8)  & 0xFF);
  cmd[3] = ((addr >> 0)  & 0xFF);

  retval = spi_write(prv->spidev, cmd, 4);
  if(0 != retval)
  {
    pr_info("SPI Failed\r\n");
    return retval;
  }
  return SUCCESS;
}

/* Full Chip Erase */
static unsigned int erase_chip(void)
{
  uint32_t retval;
  uint8_t  cmd[4] = {0};

  cmd[0] = FLASH_BULK_ERASE1;
  cmd[1] = FLASH_BULK_ERASE2;
  cmd[2] = FLASH_BULK_ERASE3;
  cmd[3] = FLASH_BULK_ERASE4;

  retval = spi_write(prv->spidev, cmd, 4);
  if(0 != retval)
  {
    pr_info("SPI Failed\r\n");
    return retval;
  }
  return SUCCESS;

}

static int device_open(struct inode *inode, struct file *file)
{
  pr_info("Open Operation Invoked\r\n");

  if(prv->inuse)
  {
    pr_info("Device Busy %s\r\n",DEVICE_NAME);
    return -EBUSY;
  }
  prv->inuse = 1;
  
  /* Check Device ID */
  if(SUCCESS != get_device_id())
    return -EINVAL;

  /* Set Page Size to 512 */
  if(SUCCESS != set_page_size())
  {
    pr_info("Page Size Setting Failed\r\n");
    return -EINVAL;
  }
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
  pr_info("Release Operation Invoked\r\n");

  prv->inuse = 0;

  return SUCCESS;
}

static ssize_t device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
  uint32_t addr, retval;
  uint8_t  cmd[8]   = {0};
  uint8_t  tmp[512] = {0};

  struct spi_transfer t[2];
  struct spi_message  m;

  pr_info("Read Operation Invoked\r\n");

  cmd[0] = FLASH_MAIN_MEMORY_PAGE_READ;

  /* Bits 0 - 8  (9  Bits) --> Address 512  bytes in a page */
  /* Bits 9 - 21 (13 Bits) --> Address 8192 pages */
  addr = prv->current_page << 9;
 
  cmd[1] = ((addr >> 16) & 0xFF);
  cmd[2] = ((addr >> 8)  & 0xFF);
  cmd[3] = ((addr >> 0)  & 0xFF);
  
  /* Initialie spi_message list */
  spi_message_init(&m);  

  /* Initialize spi_transfer structure with 0 */
  memset(t, 0, sizeof(t));

  /* Four dummy bytes are to be needed after address so we write 8 bytes */
  t[0].tx_buf = cmd;
  t[0].len    = sizeof(cmd);
  spi_message_add_tail(&t[0], &m);

  /* We read the data into a local buffer */
  t[1].rx_buf = tmp;
  t[1].len    = size;
  spi_message_add_tail(&t[1], &m);

  /* Initiate SPI Transfer */
  retval = spi_sync(prv->spidev, &m);
  if(0 != retval)
  {
    pr_info("SPI Failed\r\n");
    return retval;
  }
  /* Note: We could have used spi_write_then_read() variant and skip using 
           spi_message and spi_transfer structures but that method is not 
           portable and should not be used to transfer more than 32 bytes */
  
  /* Safely copy data from temporary buffer to the user buffer */
  retval = copy_to_user(buf, tmp, size);
  if(0 != retval)
  {
    pr_info("Partial Copy\r\n");
    return retval;
  }
  return SUCCESS;
}

static ssize_t device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
  uint32_t addr, retval;
  uint8_t  cmd[516] = {0};
  struct spi_transfer t;
  struct spi_message  m;

  pr_info("Write Operation Invoked\r\n");

/* We are using Internal Buffer 2 */
/* S1 : Read from Main Memory to Buffer 2 */
  cmd[0] = FLASH_TRANSFER_MAIN_MEMORY_PAGE_TO_BUFFER2;

  /* Bits 0 - 8  (9  Bits) --> Address 512  bytes in a page */
  /* Bits 9 - 21 (13 Bits) --> Address 8192 pages */
  addr = prv->current_page << 9;

  cmd[1] = ((addr >> 16) & 0xFF);
  cmd[2] = ((addr >> 8)  & 0xFF);
  cmd[3] = ((addr >> 0)  & 0xFF);

  retval = spi_write(prv->spidev, cmd, 4);
  if(0 != retval)
  {
    pr_info("SPI Failed\r\n");
    return retval;
  }

/* S2 : Write to Buffer 2 */
  /* Initialize spi_message list */
  spi_message_init(&m);

  /* Initialize spi_transfer structure with 0 */
  memset(&t, 0, sizeof(t));
   
  cmd[0] = FLASH_BUFFER2_WRITE;

  /* Address Offset Fixed to 0 so we write from starting of page */
  addr = 0x00;
  cmd[1] = ((addr >> 16) & 0xFF);
  cmd[2] = ((addr >> 8)  & 0xFF);
  cmd[3] = ((addr >> 0)  & 0xFF);

  /* Safely Copy User Buffer Data to Temporary Buffer */
  retval = copy_from_user(&cmd[4], buf, size);
  if(retval != 0)
  {
    pr_info("Partial Copy\r\n");
    return retval;
  }

  t.tx_buf = cmd;
  t.len    = size + 4;
  spi_message_add_tail(&t, &m);

  retval = spi_sync(prv->spidev, &m);
  if(0 != retval)
  {
    pr_info("SPI Failed\r\n");
    return retval;
  }

/* S3 : Write to SPI Flash from Buffer 2 */
  cmd[0] = FLASH_BUFFER2_TO_MAIN_MEMORY_WRITE_WITH_ERASE;	

  addr = prv->current_page << 9; 

  cmd[1] = ((addr >> 16) & 0xFF);
  cmd[2] = ((addr >> 8)  & 0xFF);
  cmd[3] = ((addr >> 0)  & 0xFF);

  /* Four Dummy Bytes are needed so size is 8 */
  retval = spi_write(prv->spidev, cmd, 8);
  if(0 != retval)
  {
    pr_info("SPI Failed\r\n");
    return retval;
  }
  
  return SUCCESS;
}

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  int err = 0;
  unsigned int val = 0;

  unsigned int *ptr = (unsigned int *)arg;

  pr_info("IOCTL Operation Invoked\r\n");
 
  /* Check if Magic Number is Matching */
  if(_IOC_TYPE(cmd) != SPI_MAGIC)
    return -ENOTTY;

  /* Check if Command is within Range */
  if(_IOC_NR(cmd) > MAX_IOCTL)
    return -ENOTTY;

  /* Check that the user space pointer is accessible */
  if(_IOC_DIR(cmd) & _IOC_READ)
    err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

  else if(_IOC_DIR(cmd) & _IOC_WRITE)
    err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

  if(err)
    return -EFAULT;

  switch(cmd)
  {
    case GET_DEVICE_ID:
      put_user(prv->device_id, ptr);
      break;

    case GET_MAX_PAGES:
      put_user(prv->max_pages, ptr);
      break;
 
    case GET_PAGE_OFFSET:
      put_user(prv->current_page, ptr);
      break;

    case SET_PAGE_OFFSET:
      get_user(prv->current_page, ptr);
      break;

    case ERASE_PAGE:
      get_user(val, ptr);
      erase_page(val);
      break;

    case ERASE_SECTOR:
      get_user(val, ptr);
      erase_sector(val);
      break;
  
    case ERASE_CHIP:
      erase_chip();
      break;
  }
  return SUCCESS;
}

static struct file_operations device_fops = {
  .owner          = THIS_MODULE,
  .open           = device_open,
  .release        = device_release,
  .read           = device_read,
  .write          = device_write,
  .unlocked_ioctl = device_ioctl,
};

static struct miscdevice device_misc = {
  .minor = MISC_DYNAMIC_MINOR,
  .name  = DEVICE_NAME,
  .fops  = &device_fops,
};

static int spi_flash_probe(struct spi_device *spidev)
{
  int retval = 0;

  pr_info("spi_flash.c : %s\r\n",__func__);

  /* Allocate Private Structure */
  prv = (struct spi_flash_prv *)kzalloc(sizeof(struct spi_flash_prv), GFP_KERNEL);
  if(prv == NULL)
  {
    pr_info("Requested Memory Allocation Failed\r\n");
    return -ENOMEM;
  }
  
  /* Save spi_device reference in private structure */
  prv->spidev = spidev;

  /* Get Device Properties */
  get_device_properties();

  /* Using Character Driver Interface but we may also use Sysfs Interface */

  /* Register a Miscellaneous Device */
  retval = misc_register(&device_misc);
  if(retval < 0)
  {
    pr_err("Device Registration Failed with Minor Number %d\r\n",device_misc.minor);
    return retval;
  }
  pr_info("Device Registered : %s with Minor Number : %d\r\n",DEVICE_NAME, device_misc.minor);

  return SUCCESS;
}

static int spi_flash_remove(struct spi_device *spidev)
{
  pr_info("spi_flash.c : %s\r\n",__func__);

  /* Free up the Private Structure */
  kfree(prv);

  pr_info("Device Unregistered : %s with Minor Number : %d\r\n",DEVICE_NAME, device_misc.minor);

  /* Unregister the Miscellaneous Device */
  misc_deregister(&device_misc);

  return SUCCESS;
}

static struct of_device_id spi_flash_mtable[] = {
  {.compatible = "atmel,at45db161d"},
  {},
};
 
MODULE_DEVICE_TABLE(of, spi_flash_mtable);

static struct spi_driver spi_flash_drv = {
  .driver = {
    .name           = "SPI_Flash_Driver",
    .of_match_table = spi_flash_mtable,
  },
  .probe  = spi_flash_probe,
  .remove = spi_flash_remove,
};

/* 
  Helper macro for drivers that don't do anything special in module init/exit.
  Each module may only use this macro once, and calling it replaces 
  module_init() and module_exit()
*/
/* Internally calls spi_register_driver() to bind with SPI Core */
module_spi_driver(spi_flash_drv);

MODULE_DESCRIPTION("High Level Driver for Atmel SPI Data Flash Device AT45DB161D");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");
