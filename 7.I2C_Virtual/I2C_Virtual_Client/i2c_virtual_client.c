#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>

struct i2c_vclient_prv 
{
  struct i2c_client client;
  unsigned int size;
  unsigned int pagesize;
  unsigned int address_width;
};

struct i2c_vclient_prv *prv = NULL;

static int i2c_vclient_read(struct i2c_client *client)
{
  struct i2c_msg msg;
  char *msgbuf = NULL;
  int rdlen = 0;

  msgbuf = kzalloc(4096, GFP_KERNEL);
  if(msgbuf == NULL)
  {
    pr_info("Buffer Allocation Failed\r\n");
    return -ENOMEM;
  }

  msg.addr  = client->addr;
  msg.buf   = msgbuf;
  /* 1 in this field signifies read operation */
  msg.flags = 1;
  msg.len   = 15;

  /* Read I2C Message and Print the same */
  rdlen = i2c_transfer(client->adapter, &msg, 1);

  pr_info("Read %s\r\n",msgbuf);

  kfree(msgbuf);

  return rdlen;
}

static int i2c_vclient_write(struct i2c_client *client)
{
  /* This is the main structure used to send/recv a message over i2c bus */
  struct i2c_msg msg;
  char *msgbuf = NULL;
  int wrlen = 0;

  msgbuf = kzalloc(4096, GFP_KERNEL);
  if(msgbuf == NULL)
  {
    pr_info("Buffer Allocation Failed\r\n");
    return -ENOMEM;
  }  
  memcpy(msgbuf, "Hello I am Debmalya", 20);
 
  msg.addr  = client->addr;
  msg.buf   = msgbuf;
  /* 0 in this field signifies write operation */
  msg.flags = 0;
  msg.len   = 20;

  /* Send I2C Message */
  wrlen = i2c_transfer(client->adapter, &msg, 1);
  
  kfree(msgbuf);

  return wrlen;
}

static int i2c_vclient_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
  pr_info("i2c_virtual_client.c   : %s\r\n",__func__);

  /* Allocate Private Structure */
  prv = (struct i2c_vclient_prv *)kzalloc(sizeof(struct i2c_vclient_prv), GFP_KERNEL);
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
  pr_info("Address Width = %d bits\r\n", prv->address_width);

  /* Test I2C Write */
  i2c_vclient_write(client);

  /* Test I2C Read */
  i2c_vclient_read(client);
 
  /* Ideally we should add a sysfs or ioctl interface here to expose the device 
     functionalities to user space */
  return 0;
}

static int i2c_vclient_remove(struct i2c_client *client)
{
  pr_info("i2c_virtual_client.c   : %s\r\n",__func__);
  
  /* Free up the Private Structure */
  kfree(prv);
  return 0;
}

static const struct i2c_device_id i2c_vclient_ids[] = {
  {"i2c_virtual_client",0x77},
  {},
};

static struct i2c_driver i2c_vclient_drv = {
  .driver = {
    .name       = "I2C_vClient_Driver", 
    .owner	= THIS_MODULE,
  },
  .probe  = i2c_vclient_probe, 
  .remove = i2c_vclient_remove,
  .id_table = i2c_vclient_ids,
};

/* 
  Helper macro for drivers that don't do anything special in module init/exit.
  Each module may only use this macro once, and calling it replaces 
  module_init() and module_exit()
*/

/* Internally calls i2c_register_driver() to bind with I2C Core */
module_i2c_driver(i2c_vclient_drv);

MODULE_DESCRIPTION("High Level Driver for Virtual I2C Client Device");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

