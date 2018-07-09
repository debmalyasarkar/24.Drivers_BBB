#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include "i2c_eeprom.h"

#define SUCCESS 0

#define DEVICE_NAME "at24c32"

#define RDWR_LIMIT 32

struct i2c_eeprom_prv 
{
  struct i2c_client client;
  uint32_t size;
  uint32_t pagesize;
  uint32_t address_width;
  uint32_t inuse;
  uint32_t page_no;
};

static struct i2c_eeprom_prv *prv = NULL;

static int device_open(struct inode *inode, struct file *file)
{
  if(prv->inuse)
  {
    pr_info("Device Busy %s\r\n",DEVICE_NAME);
    return -EBUSY;
  }
  prv->inuse = 1;
  pr_info("Open Operation Invoked\r\n");

  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
  prv->inuse = 0;
  pr_info("Release Operation Invoked\r\n");
  return SUCCESS;
}

static ssize_t device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
  int offset, retval;
  struct i2c_msg msg[2];
  char msgbuf[34] = {0};
  
  if(size > RDWR_LIMIT)
  {
    pr_info("Read Limit Exceeeded, Setting MAX Limit\r\n");
    size = RDWR_LIMIT;
  }

  offset = prv->page_no * 32;

  msgbuf[0] = offset >> 8;
  msgbuf[1] = offset;
  
  /* Dummy Write 2 Bytes OffSet Address */
  msg[0].addr  = prv->client.addr;
  msg[0].buf   = msgbuf;
  msg[0].flags = 0;              
  msg[0].len   = 2;

  /* Actual Read */
  msg[1].addr  = prv->client.addr;
  msg[1].buf   = &msgbuf[2];
  msg[1].flags = 1;
  msg[1].len   = size;

  retval = i2c_transfer(prv->client.adapter, msg, 2);
  if(retval !=  2)
  {
    pr_info("I2C Transfer Failed\r\n");
    return -retval;
  }

  retval = copy_to_user(buf, &msgbuf[2], size);
  if(retval != 0)
  {
    pr_info("Partial Copy\r\n");
    return -retval;
  }
   
  pr_info("Read Operation Invoked\r\n");
  return SUCCESS;
}

static ssize_t device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
  int offset, retval;
  struct i2c_msg msg;
  char msgbuf[34] = {0};
 
  if(size > RDWR_LIMIT)
  {
    pr_info("Write Limit Exceeded, Setting MAX Limit\r\n");
    size = RDWR_LIMIT;
  }
  
  offset = prv->page_no * 32;

  msgbuf[0] = offset >> 8;
  msgbuf[1] = offset;
 
  /* First two bytes are offset addresses, followed by data to write */ 
  retval = copy_from_user(&msgbuf[2], buf, size);
  if(retval != 0)
  {
    pr_info("Partial Copy\r\n");
    return -retval;
  }

  msg.addr  = prv->client.addr;
  msg.buf   = msgbuf;
  msg.flags = 0;
  msg.len   = size + 2;

  retval = i2c_transfer(prv->client.adapter, &msg, 1);
  if(retval !=  1)
  {
    pr_info("I2C Transfer Failed\r\n");
    return -retval;
  }

  pr_info("Write Operation Invoked\r\n");
  return SUCCESS;
}

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  int *ptr = (int *)arg;

  switch(cmd)
  {
    case GET_PAGE_OFFSET:
      if(0 != copy_to_user(ptr, &prv->page_no, sizeof(prv->page_no)))
        pr_info("Partial Copy\r\n");
      break;

    case SET_PAGE_OFFSET:
      if(0 != copy_from_user(&prv->page_no, ptr, sizeof(prv->page_no)))
        pr_info("Partial Copy\r\n");
      break;
  }
  pr_info("IOCTL Operation Invoked\r\n");
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

static int i2c_eeprom_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
  int retval;

  pr_info("i2c_eeprom.c   : %s\r\n",__func__);

  /* Allocate Private Structure */
  prv = (struct i2c_eeprom_prv *)kzalloc(sizeof(struct i2c_eeprom_prv), GFP_KERNEL);
  if(prv == NULL)
  {
    pr_info("Requested Memory Allocation Failed\r\n");
    return -ENOMEM;
  }
  /* Instantiate i2c_client structure */
  prv->client = *client;

  /* Read and Print I2C Client Properties from the Device Tree Node */
  if(0 != device_property_read_u32(&client->dev, "size", &prv->size))
  {
    pr_info("Missing \"size\" property\r\n");
    return -ENODEV;
  }
  if(0 != device_property_read_u32(&client->dev, "pagesize", &prv->pagesize))
  {
    pr_info("Missing \"pagesize\" property\r\n");
    return -ENODEV;
  }
  if(0 != device_property_read_u32(&client->dev, "address-width", &prv->address_width))
  {
    pr_info("Missing \"address-width\" property\r\n");
    return -ENODEV;
  }

  pr_info("Size          = %d bytes\r\n", prv->size);
  pr_info("Page Size     = %d bytes\r\n", prv->pagesize);
  pr_info("Address Width = %d bits\r\n",  prv->address_width);

  /* Using Character Driver Interface but we may also use Sysfs Interface */
  /* Register a Miscellaneous Device */
  retval = misc_register(&device_misc);
  if(retval < 0)
  {  
    pr_err("Device Registration Failed with Minor Number %d\r\n",device_misc.minor);
    return retval;
  }
  pr_info("Device Registered : %s with Minor Number : %d\r\n",DEVICE_NAME, device_misc.minor);
  return 0;
}

static int i2c_eeprom_remove(struct i2c_client *client)
{
  pr_info("i2c_eeprom.c   : %s\r\n",__func__);
  
  /* Free up the Private Structure */
  kfree(prv);

  pr_info("Device Unregistered : %s with Minor Number : %d\r\n",DEVICE_NAME, device_misc.minor);
  
  /* Unregister the Miscellaneous Device */
  misc_deregister(&device_misc);
  return 0;
}

static const struct i2c_device_id i2c_eeprom_ids[] = {
  {"i2c_eeprom_at24c32",0x50},
  {},
};

static struct i2c_driver i2c_eeprom_drv = {
  .driver = {
    .name       = "I2C_EEPROM_Driver", 
    .owner	= THIS_MODULE,
  },
  .probe    = i2c_eeprom_probe, 
  .remove   = i2c_eeprom_remove,
  .id_table = i2c_eeprom_ids,
};

/* 
  Helper macro for drivers that don't do anything special in module init/exit.
  Each module may only use this macro once, and calling it replaces 
  module_init() and module_exit()
*/

/* Internally calls i2c_register_driver() to bind with I2C Core */
module_i2c_driver(i2c_eeprom_drv);

MODULE_DESCRIPTION("High Level Driver for I2C EEPROM Device");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

/*
  Notes
  ------
  The chip supports the following two operations:
  1. Read from current position.
  2. Write to specified position.

  Read from specified position is not supported directly. 
  For this we do a dummy write that begins by setting the 
  address.
  Then we dont send any data or I2C stop.
  Instead we perform an I2C start again and perform a normal read.

  We can read as much data as required.

  We can write up to 32 bytes at a time. 
  There is no need to start at the beginning of the page but regardless of 
  where we start in a page, if we continue to write after reaching the end of 
  the page we will wrap back to the start of it and continue writing there.
 
  In this driver we simply limit read and write to 32 bytes but for a fully
  featured driver we need to keep track of how much we are writing and where
  the next page boundary is.
*/

/* 
  Pseudo Code for Retry Mechanism of I2C Transfer
  ------------------------------------------------
  Find Timeout Value In Terms of Jiffies
    -->timeout = jiffies + msecs_to_jiffies(TIMEOUT_MSECS);
  Check Current Jiffie Value With Timeout Value in Loop Condition
    -->while(time_before(jiffies, timeout))
  If I2C Transfer Is Successful then Break Loop
    -->i2c_transfer()
  Otherwise Sleep For 1 millisec And Continue Loop
    -->msleep(1)
  Return -ETIMEDOUT If Transfer Fails
*/

