/* Beaglebone Black has 2 SPI Controllers namely spi0 and spi1 */
/* We have created a virtual controller attached to the spi interface
   called spi2 */
/* This low level driver hooks to the platform_core at the lower level 
   and spi_core at the higher level */
/* The Corresponding Functions are called when a client accesses this 
   host controller and this is indicated using print statements */
/* In real spi low level driver these functions will directly access 
   the hardware registers for a specific task. */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>

/* Setup mode and clock, etc (spi driver may call many times) */
static int spi_vhost_setup(struct spi_device *spi)
{
  pr_info("spi_virtual_host.c   : %s\r\n",__func__);
  return 0;
}


/* These hooks are for drivers that want to use the generic  master 
   transfer queueing mechanism */
static int spi_vhost_prepare_message(struct spi_master *master, 
                              struct spi_message *message)
{
  pr_info("spi_virtual_host.c   : %s\r\n",__func__);
  return 0;
}

/* These hooks are for drivers that use a generic implementation 
   of transfer_one_message() provied by the core */
static int spi_vhost_transfer_one(struct spi_master *master, 
                           struct spi_device *spi, 
                           struct spi_transfer *transfer)
{
  pr_info("spi_virtual_host.c   : %s\r\n",__func__);
  return 0;
}

/* Called on release() to free memory provided by spi_master */
static void spi_vhost_cleanup(struct spi_device *spi)
{
  pr_info("spi_virtual_host.c   : %s\r\n",__func__);
}

static int spi_vhost_probe(struct platform_device *pdev)
{
  struct spi_master *master = NULL;

  pr_info("spi_virtual_host.c   : %s\r\n",__func__);

  /* Instantiate a spi_master structure for the host controller */
  /* @dev : The controller, possibly using the platform_bus.
     @size: How much zeroed driver-private data to allocate; 
            The pointer to this memory is in the driver_data field of the 
            returned device, accessible with spi_master_get_devdata().
  */
  /* So we commonly pass private structure size as the second argument */
  master = spi_alloc_master(&pdev->dev, 4096);
  if(NULL == master)
  {
    pr_info("SPI Master Allocation Failed\r\n");
    return -ENOMEM;
  }
  /* Allocate the function pointers which directly act on the registers when
     they are invoked */
  master->setup           = spi_vhost_setup;
  master->prepare_message = spi_vhost_prepare_message;
  master->transfer_one    = spi_vhost_transfer_one;
  master->cleanup         = spi_vhost_cleanup;
  master->dev.of_node     = pdev->dev.of_node;
 
  /* As our spi_master structure is ready so we now register the structure with
     the spi core (mid layer) */
  if(0 > devm_spi_register_master(&pdev->dev, master))
  {
    pr_info("SPI Master Registration Failed\r\n");
    return -ENOMEM;
  }
  return 0;
}

static int spi_vhost_remove(struct platform_device *pdev)
{
  pr_info("spi_virtual_host.c   : %s\r\n",__func__);
  return 0;
}

static struct of_device_id spi_vhost_mtable[] = {
  {.compatible = "ti, spi_virtual_host"},
  {},
};
 
MODULE_DEVICE_TABLE(of, spi_vhost_mtable);

static struct platform_driver spi_vhost_drv = {
  .driver = {
    .name           = "SPI_vHost_Driver",
    .of_match_table = spi_vhost_mtable,
  },
  .probe  = spi_vhost_probe,
  .remove = spi_vhost_remove,
};

/* 
  Helper macro for drivers that don't do anything special in module init/exit.
  Each module may only use this macro once, and calling it replaces 
  module_init() and module_exit()
*/
module_platform_driver(spi_vhost_drv);

MODULE_DESCRIPTION("Platform Driver for Virtual SPI Host Controller");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");
