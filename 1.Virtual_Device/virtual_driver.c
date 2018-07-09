#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

/* Drivers can access device tree data through interface API's provided 
   in the kernel header /include/linux/of.h */
static int virtdev_probe(struct platform_device *platformDevice)
{
  void __iomem *baseAddr;
  struct resource *resource;
  
  pr_info("%s : Probe Invoked\r\n",__func__);
  
  /* The name of the driver which is invoked for this device */
  /* This was originally specified in the platform driver structure */
  pr_info("%s : Driver Invoked\r\n",platformDevice->name);
  
  /* Device Resource structure filled from device tree node properties */
  resource = platform_get_resource(platformDevice, IORESOURCE_MEM, 0);
  
  /* Check, Request Region, and Ioremap Resource */
  /* Arg1 = Generic device to handle the resource for */
  /* Arg2 = Resource to be handled */
  
  /* Internally calls devm_ioremap - Managed ioremap() */
  /* Map is automatically unmapped on driver detach. */
  baseAddr = devm_ioremap_resource(&platformDevice->dev, resource);
  if(IS_ERR(baseAddr))
  { 
    pr_info("Failed to remap device registers\r\n");
    return PTR_ERR(baseAddr);
  }
  /* Access Device Tree Node Properties from Driver using Resource Structure */
  pr_info("Device Start Address = %x\r\n",resource->start);
  pr_info("Device End Address   = %x\r\n",resource->end);
  pr_info("Device Name          = %s\r\n",resource->name);
  pr_info("Base Address as seen from kernel = %p\r\n",baseAddr);
  return 0;
}

static int virtdev_remove(struct platform_device *pdev)
{
  pr_info("%s : Remove Invoked\r\n",__func__);
  return 0;
}

/* Driver uses the specific device details (compatible property) here to match
   device and bind with itself */
/* The compatible property must be exactly the same as specified in the device
   tree node,so always copy it from there and do not manually type this field */
static const struct of_device_id virtdev_of_mtable[] = {
  { .compatible = "virtual_vendor,virtual_device", },
  {}
};

/* Declare the ID table as a device table to enable autoloading of the module
   as the device tree is instantiated */
/* Kernel autoloads driver using modprobe when this device is found */
MODULE_DEVICE_TABLE(of, virtdev_of_mtable);

/* We are specifiying the device and corresponding driver information in the
   platform_driver structure.
   In this the of_match_table field has an array inside which the device 
   compatible field is specified in same way as in device tree node */
static struct platform_driver virtdev_driver = {
  .driver = {
    .name           = "chrdrv_virtual_device",
    .owner          = THIS_MODULE,
    .of_match_table = virtdev_of_mtable,
  },
  .probe  = virtdev_probe,
  .remove = virtdev_remove,
};

static int __init my_module_init(void)
{
  pr_info("%s : Virtual Node - Driver Inserted\r\n",__func__);
  return platform_driver_register(&virtdev_driver);
}

static void __exit my_module_exit(void)
{
  pr_info("%s : Virtual Node - Driver Removed\r\n",__func__);
  platform_driver_unregister(&virtdev_driver);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_DESCRIPTION("Platform Driver for the Virtual Node at Address 0x4800A000");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

