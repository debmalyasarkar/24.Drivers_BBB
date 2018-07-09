/* Beaglebone Black has 2 SPI Controllers namely spi0 and spi1 */
/* We have created a virtual controller attached to the spi interface
   called spi2 */
/* This high level driver hooks to the spi_core at the lower level 
   and sysfs interface at the higher level */
/* Since this driver is attached to the host controller at spi2 so 
   when any functions of this driver is invoked, it causes the 
   corresponding functions of the host controller driver to be invoked */

#include <linux/module.h>
#include <linux/spi/spi.h>

#define TEST_COMMAND 0x01

static int spi_vclient_probe(struct spi_device *spidev)
{
  pr_info("spi_virtual_client.c : %s\r\n",__func__);
 
  /* Write a byte for testing the SPI transfer */ 
  /* Results in SPI Host Functions to be invoked to indicate successful binding */
  if(0 > spi_w8r8(spidev, TEST_COMMAND))
    pr_info("Transfer Error\r\n");

  /* We may use API's here to create sysfs entries for binding the driver 
     to sysfs interface */
  return 0;
}

static int spi_vclient_remove(struct spi_device *spidev)
{
  pr_info("spi_virtual_client.c : %s\r\n",__func__);
  return 0;
}

static struct of_device_id spi_vclient_mtable[] = {
  {.compatible = "spi_virtual_client"},
  {},
};
 
MODULE_DEVICE_TABLE(of, spi_vclient_mtable);

static struct spi_driver spi_vclient_drv = {
  .driver = {
    .name           = "SPI_vClient_Driver",
    .of_match_table = spi_vclient_mtable,
  },
  .probe  = spi_vclient_probe,
  .remove = spi_vclient_remove,
};

/* 
  Helper macro for drivers that don't do anything special in module init/exit.
  Each module may only use this macro once, and calling it replaces 
  module_init() and module_exit()
*/
/* Internally calls spi_register_driver() to bind with SPI Core */
module_spi_driver(spi_vclient_drv);

MODULE_DESCRIPTION("High Level Driver for Virtual SPI Client Device");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");
