#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

/*
  i2c_adapter is the structure used to identify a physical i2c bus along
  with the access algorithms necessary to access it.
*/
static struct i2c_adapter *adapter;

/* This function is called for both read and write from client */
/* We can implement our implementation here in case of real drivers */
static int omap_i2c_xfer_msg(struct i2c_adapter *adapter, struct i2c_msg *msg, int stop, int flag)
{
  char *buffer = "Dark Side of The Moon by Pink Floyd";

  pr_info("i2c_virtual_host.c     : %s\r\n",__func__);

  pr_info("Addr = %#02x, Len = %d, Flag = %x\r\n",msg->addr, msg->len, msg->flags);
  
  if(msg->len == 0)
    return -EINVAL;

  if(msg->flags == 0)
  {
    pr_info("Write Request\r\n");
    pr_info("%s\r\n", msg->buf);
  }
  else
  {
    pr_info("Read Request\r\n");
    memcpy(msg->buf, buffer, msg->len);
  }
  return 0;
}

/* Issue a set of i2c transactions to the given I2C adapter defined by the msgs
   array, with num messages available to transfer via the adapter */
static int omap_i2c_xfer(struct i2c_adapter *adapter, struct i2c_msg msgs[], int num)
{
  int ii, retval;

  pr_info("i2c_virtual_host.c     : %s\r\n",__func__);
  
  pr_info("Number of Messages = %d\r\n", num);
  
  for(ii = 0; ii < num; ii++)
  {
    retval = omap_i2c_xfer_msg(adapter, &msgs[ii], (ii == (num - 1)), (num == 1));
    if(0 != retval)
      return retval;
  }
  return num;
}

/* Return the flags that this algorithm/adapter pair supports from the 
   I2C_FUNC_* flags. */
static u32 omap_i2c_func(struct i2c_adapter *adapter)
{
  pr_info("i2c_virtual_host.c     : %s\r\n",__func__);

  return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

/* struct i2c_algorithm - represent I2C transfer method */
static const struct i2c_algorithm i2c_algo = {
  .master_xfer   = omap_i2c_xfer,
  .functionality = omap_i2c_func,
};

static int i2c_vhost_probe(struct platform_device *pdev)
{
  pr_info("i2c_virtual_host.c     : %s\r\n",__func__);
  
  /* Instantiate a new i2c_adapter structure */
  /* Automatically freed on driver detachment */
  adapter = devm_kzalloc(&pdev->dev, sizeof(*adapter), GFP_KERNEL);
  if(NULL == adapter)
  {
    pr_info("I2C Adapter Allocation Failed\r\n");
    return -ENOMEM;
  }
  adapter->owner       = THIS_MODULE;
  /* Classes for which probing is allowed */
  adapter->class       = I2C_CLASS_DEPRECATED;
  /* Algorithm to access the bus */
  adapter->algo        = &i2c_algo;
  adapter->dev.parent  = &pdev->dev;
  adapter->dev.of_node = pdev->dev.of_node;
  /* Bus ID */
  adapter->nr          = pdev->id;
  strlcpy(adapter->name, "I2C Virtual Adapter", sizeof(adapter->name));

  /* Declare i2c_adapter and dynamically allocate a new bus number.
     When this returns zero, a new bus number was allocated and stored
     in adap->nr, and the specified adapter became available for clients.
     Otherwise, a negative errno value is returned.
  */
  if(0 != i2c_add_adapter(adapter))
  {
    pr_info("I2C Adapter Registration Failed\r\n");
    return -ENODEV;
  }
  return 0;
}

static int i2c_vhost_remove(struct platform_device *pdev)
{
  pr_info("i2c_virtual_host.c     : %s\r\n",__func__);

  /* Unregister the i2c_adapter structure */
  i2c_del_adapter(adapter);
  return 0;
}

/* Compatible field must match with device tree */
static struct of_device_id i2c_vhost_mtable[] = {
  {.compatible = "ti, i2c_virtual_host"},
  {},
};

MODULE_DEVICE_TABLE(of, i2c_vhost_mtable);

static struct platform_driver i2c_vhost_drv = {
  .driver = {
    .name           = "I2C_vHost_Driver", 
    .of_match_table = i2c_vhost_mtable,
  },
  .probe  = i2c_vhost_probe, 
  .remove = i2c_vhost_remove,
};

/* 
  Helper macro for drivers that don't do anything special in module init/exit.
  Each module may only use this macro once, and calling it replaces 
  module_init() and module_exit()
*/
/* Register the adapter with i2c_core */
module_platform_driver(i2c_vhost_drv);

MODULE_DESCRIPTION("Platform Driver for Virtual I2C Host Controller");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

