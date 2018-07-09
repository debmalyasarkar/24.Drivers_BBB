#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include "i2c_rtc.h"

#define TIMEOUT_MS 25

/* Private Structure */
struct i2c_rtc_prv 
{
  struct i2c_client client;
  uint32_t reg;
  uint32_t inuse;
};

static struct i2c_rtc_prv *prv = NULL;

static int device_open(struct inode *inode, struct file *file)
{
  pr_info("Open Operation Invoked\r\n");

  if(prv->inuse)
  {
    pr_info("Device Busy %s\r\n",DEVICE_NAME);
    return -EBUSY;
  }
  prv->inuse = 1;

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
  int retval;
  struct i2c_msg msg[2];
  char regAddr[1] = {0};
  char msgbuff[8] = {0};
  unsigned long timeout = 0;
   
  pr_info("Read Operation Invoked\r\n");
 
  /* Dummy Write 1 Bytes To Set OffSet Address to 0x00 */
  msg[0].addr  = prv->client.addr;
  msg[0].buf   = regAddr;
  msg[0].flags = 0;              
  msg[0].len   = 1;

  /* Actual Read Operation To Read All Registers from 0x00 to 0x07 */
  msg[1].addr  = prv->client.addr;
  msg[1].buf   = msgbuff;
  msg[1].flags = 1;
  msg[1].len   = 8;

  /* Set timeout value in terms of jiffies value */
  timeout = jiffies + msecs_to_jiffies(TIMEOUT_MS);

  /* If I2C Transfer fails then Retry until timeout */
  while(time_before(jiffies, timeout))
  {
    retval = i2c_transfer(prv->client.adapter, msg, 2);
    if(retval != 2)
    {
      msleep(1);
      pr_info("I2C Transfer Retry\r\n");
      continue;
    }
    goto process;
  } 
  pr_info("I2C Transfer Timed Out\r\n");  
  return -ETIMEDOUT;

process:
  retval = copy_to_user(buf, msgbuff, sizeof(msgbuff));
  if(retval != 0)
  {
    pr_info("Partial Copy\r\n");
    return -retval;
  }
  pr_info("I2C Transfer Success\r\n");
  return SUCCESS;
}

static ssize_t device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
  int retval;
  struct i2c_msg msg;
  char msgbuff[9] = {0};
  unsigned long timeout = 0;

  pr_info("Write Operation Invoked\r\n");

  /* First Value of Buffer is 0x00 to Set Register Pointer to Base Address */
  /* After this value we put our data for all the 8 registers.
     Once a data byte is taken, then the register pointer is updated 
     automatically. */
  retval = copy_from_user(&msgbuff[1], buf, size);
  if(retval != 0)
  {
    pr_info("Partial Copy\r\n");
    return -retval;
  }

  /* Write Operation to Write To All Registers from 0x00 to 0x07 */
  msg.addr  = prv->client.addr;
  msg.buf   = msgbuff;
  msg.flags = 0;
  msg.len   = 9;
 
  /* Set timeout value in terms of jiffies value */
  timeout = jiffies + msecs_to_jiffies(TIMEOUT_MS);

  /* If I2C Transfer fails then Retry until timeout */
  while(time_before(jiffies, timeout))
  {
    retval = i2c_transfer(prv->client.adapter, &msg, 1);
    if(retval != 1)
    {
      msleep(1);
      pr_info("I2C Transfer Retry\r\n");
      continue;
    }
    goto done;
  }
  pr_info("I2C Transfer Timed Out\r\n");
  return -ETIMEDOUT;

done:
  return SUCCESS;
}

/* Function to Get Value from a Single Register */
static int get_value(uint8_t addr, uint8_t *data)
{
  struct i2c_msg msg[2];
  char msgbuff[2] = {0};
  int retval;
 
  msgbuff[0] = addr;
  
  /* Write 1 Bytes To Set Register Address */
  msg[0].addr  = prv->client.addr;
  msg[0].buf   = &msgbuff[0];
  msg[0].flags = 0;
  msg[0].len   = 1;

  /* Actual Read Operation To Get Register Value */
  msg[1].addr  = prv->client.addr;
  msg[1].buf   = &msgbuff[1];
  msg[1].flags = 1;
  msg[1].len   = 1;

  retval = i2c_transfer(prv->client.adapter, msg, 2);
  if(retval !=  2)
  {
    pr_info("I2C Transfer Failed\r\n");
    return -retval;
  }

  /* Send back the value */
  *data = msgbuff[1];

  return SUCCESS;
}

/* Function to Set Value to a Single Register */
static int set_value(uint8_t addr, uint8_t data)
{
  struct i2c_msg msg;
  char msgbuff[2] = {0};
  int retval;

  msgbuff[0] = addr;
  msgbuff[1] = data;

  /* Write Register Address Followed By Data */
  msg.addr  = prv->client.addr;
  msg.buf   = msgbuff;
  msg.flags = 0;
  msg.len   = 2;

  retval = i2c_transfer(prv->client.adapter, &msg, 1);
  if(retval !=  1)
  {
    pr_info("I2C Transfer Failed\r\n");
    return -retval;
  }
  return SUCCESS;
}

/* Getting and Setting Various Configuration Options of DS1307 */
/* ARM Architecture explicitly does not support direct dereferencing of any
   userspace pointer so use appropriate functions. */
/* put_user and get_user are faster than copy_to_user and copy_from_user 
   function variants. */
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  int err = 0; 
  uint8_t val = 0;
  
  uint8_t *ptr = (uint8_t *)arg;

  /* Check if Magic Number is Matching */
  if(_IOC_TYPE(cmd) != RTC_MAGIC)
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
/* 
  Set() Functions - Receive User Command and Set/Clear the Corresponding 
                    Configuration Bit. 
*/
    case SET_CLOCK_HALT:
      get_user(val, ptr);
      set_value(ADDRESS_SECOND, (val << 7));
      break;

    case SET_MERIDIEM:
      get_user(val, ptr);
      set_value(ADDRESS_HOUR, (val << 5));
      break;

    case SET_HOUR_MODE:
      get_user(val, ptr);
      set_value(ADDRESS_HOUR, (val << 6));
      break;

    case SET_OUT:
      get_user(val, ptr);
      set_value(ADDRESS_CONTROL, (val << 7));
      break;

    case SET_SQWE:
      get_user(val, ptr);
      set_value(ADDRESS_CONTROL, (val << 4));
      break;

    case SET_RS1:
      get_user(val, ptr);
      set_value(ADDRESS_CONTROL, (val << 1));
      break;

    case SET_RS0:
      get_user(val, ptr);
      set_value(ADDRESS_CONTROL, (val << 0));
      break;

/* 
   Get() Functions - Receive User Command and Read the Corresponding 
                     Configuration Bit.
                     If Bit is Set then Return 1 and if it is Cleared 
                     then Return 0 to Userspace.
*/
    case GET_CLOCK_HALT:
      get_value(ADDRESS_SECOND, &val);
      if(val & (1 << 7))
        put_user(1, ptr);
      else
        put_user(0, ptr);
      break;

    case GET_MERIDIEM:
      get_value(ADDRESS_HOUR, &val);
      if(val & (1 << 5))
        put_user(1, ptr);
      else
        put_user(0, ptr);
      break;

    case GET_HOUR_MODE:
      get_value(ADDRESS_HOUR, &val);
      if(val & (1 << 6))
        put_user(1, ptr);
      else
        put_user(0, ptr);
      break;

    case GET_OUT:
      get_value(ADDRESS_CONTROL, &val);
      if(val & (1 << 7))
        put_user(1, ptr);
      else
        put_user(0, ptr);
      break;

    case GET_SQWE:
      get_value(ADDRESS_CONTROL, &val);
      if(val & (1 << 4))
        put_user(1, ptr);
      else
        put_user(0, ptr);
      break;

    case GET_RS1:
      get_value(ADDRESS_MONTH, &val);
      if(val & (1 << 1))
        put_user(1, ptr);
      else
        put_user(0, ptr);
      break;

    case GET_RS0:
      get_value(ADDRESS_YEAR, &val);
      if(val & (1 << 0))
        put_user(1, ptr);
      else
        put_user(0, ptr);
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

static int i2c_rtc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
  int retval;

  pr_info("i2c_rtc.c   : %s\r\n",__func__);

  /* Allocate Private Structure */
  prv = (struct i2c_rtc_prv *)kzalloc(sizeof(struct i2c_rtc_prv), GFP_KERNEL);
  if(prv == NULL)
  {
    pr_info("Requested Memory Allocation Failed\r\n");
    return -ENOMEM;
  }
  /* Instantiate i2c_client structure */
  prv->client = *client;

  /* Read and Print I2C Client Properties from the Device Tree Node */
  if(0 != device_property_read_u32(&client->dev, "reg", &prv->reg))
  {
    pr_info("Missing \"reg\" property\r\n");
    return -ENODEV;
  }

  pr_info("Reg Address   = %d\r\n", prv->reg);

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

static int i2c_rtc_remove(struct i2c_client *client)
{
  pr_info("i2c_rtc.c   : %s\r\n",__func__);
  
  /* Free up the Private Structure */
  kfree(prv);

  pr_info("Device Unregistered : %s with Minor Number : %d\r\n",DEVICE_NAME, device_misc.minor);
  
  /* Unregister the Miscellaneous Device */
  misc_deregister(&device_misc);
  return 0;
}

static const struct i2c_device_id i2c_rtc_ids[] = {
  {"i2c_rtc_ds1307",0x68},
  {},
};

static struct i2c_driver i2c_rtc_drv = {
  .driver = {
    .name       = "I2C_RTC_Driver", 
    .owner	= THIS_MODULE,
  },
  .probe    = i2c_rtc_probe, 
  .remove   = i2c_rtc_remove,
  .id_table = i2c_rtc_ids,
};

/* 
  Helper macro for drivers that don't do anything special in module init/exit.
  Each module may only use this macro once, and calling it replaces 
  module_init() and module_exit()
*/

/* Internally calls i2c_register_driver() to bind with I2C Core */
module_i2c_driver(i2c_rtc_drv);

MODULE_DESCRIPTION("High Level Driver for I2C RTC Device");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

