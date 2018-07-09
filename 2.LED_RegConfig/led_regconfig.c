/* Off board LED connected to GPIO Bank 0 Pin 27 
 * Physicaly connected on the P8 header to Pin 17
 * By default the pin is configured as ACTIVE LOW
 * This driver uses sysfs interface for user interaction
 * This driver makes use of SOC registers and low level bus read/write calls
*/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/sysfs.h>

/* These are register offsets defined in AM335x Technical Reference Manual */
/* Located under the GPIO Registers Section */

/* For Different Banks the Base Addresses will change but the Register Offset
   will be same */

/* The GPIO_OE register is used to enable the pins output capabilities. 
   At reset, all the GPIO related pins are configured as input and output 
   capabilities are disabled. 
   Read Value - 0 [Configured as Output]
   Read Value - 1 [Configured as Input ] */
#define OMAP_GPIO_OE                    0x0134

/* Writing a 1 to a bit in the GPIO_SETDATAOUT register sets to 1 the 
   corresponding bit in the GPIO_DATAOUT register
   Writing a 0 has no effect. 
   A read of the GPIO_SETDATAOUT register returns the value of the data output
   register (GPIO_DATAOUT). */
#define OMAP_GPIO_SETDATAOUT            0x0194

/* Writing a 1 to a bit in the GPIO_CLEARDATAOUT register clears to 0 the 
   corresponding bit in the GPIO_DATAOUT register.
   Writing a 0 has no effect. 
   A read of the GPIO_CLEARDATAOUT register returns the value of the data output 
   register (GPIO_DATAOUT). */
#define OMAP_GPIO_CLEARDATAOUT          0x0190

/* The GPIO_DATAIN register is a read-only register. 
   The input data is sampled synchronously with the interface clock and then 
   captured in the GPIO_DATAIN register synchronously with the interface 
   clock. */
#define OMAP_GPIO_DATAIN                0x0138

struct gpio_pin_bank_t
{
  uint32_t pinNo;
  unsigned int gpioBase;
};

static struct gpio_pin_bank_t *gpbank;

static void __iomem *baseAddr;

static struct kobject *kobj;

static int ledInitDone;

void led_init(void)
{
  unsigned int value = 0;

  value = readl_relaxed(baseAddr + OMAP_GPIO_OE);

  /* Pin 27 Set as Low to Configure In Output Mode */
  value = value & ~(1 << gpbank->pinNo);

  if(value & (1 << gpbank->pinNo))
    pr_info("Direction of Data is Set as Input\r\n");
  else
    pr_info("Direction of Data is Set as Output\r\n");

  writel_relaxed(value, baseAddr + OMAP_GPIO_OE);

  ledInitDone = 1;
}

static ssize_t led_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
  unsigned int value = 0;

  pr_info("%s Invoked From SysFs\r\n",__func__);
  
  if(!ledInitDone)
    led_init();

  value = readl_relaxed(baseAddr + OMAP_GPIO_DATAIN);
  
  /* Checking Bit is 1 or 0 */
  if(value & (1 << gpbank->pinNo))
  {
    pr_info("%s: LED is ON\r\n",__func__);
    return sprintf(buf,"%d\n",1);
  }
  else 
  {
    pr_info("%s: LED is OFF\r\n",__func__);
    return sprintf(buf,"%d\n",0);
  }
}

static ssize_t led_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
  unsigned int data = 0, value = 0, retval;

  pr_info("%s Invoked From SysFs\r\n",__func__);
  
  if (!ledInitDone)
    led_init();

  retval = sscanf(buf,"%d",&value);
  if((retval < 0) || (retval > 1))
    return -EINVAL;
  if((value < 0) || (value > 1))
    return -EINVAL;

  data = readl_relaxed(baseAddr + OMAP_GPIO_DATAIN);

  if(value == 0)
  {
    pr_info("%s: LED Turned OFF\r\n",__func__);
    data = data | (1U << gpbank->pinNo);
    writel_relaxed(data, baseAddr + OMAP_GPIO_CLEARDATAOUT);
  }
  else 
  {
    pr_info("%s: LED Turned ON\r\n",__func__);
    data = data | (1U << gpbank->pinNo);
    writel_relaxed(data, baseAddr + OMAP_GPIO_SETDATAOUT);
  }
  return data;
}

static struct kobj_attribute led_attribute = __ATTR(external_led, 0660, led_show, led_store);

static int extled_probe(struct platform_device *pdev)
{
  struct device_node *pnode;
  
  pr_info("%s : Probe Invoked\r\n",__func__);

  /* gpbank is our private structure to store pin number and base address */
  gpbank = (struct gpio_pin_bank_t *)kmalloc(sizeof(struct gpio_pin_bank_t), GFP_KERNEL);
  
  /*
  * of_get_named_gpio() - Get a GPIO number to use with GPIO API
  * @np:		device node to get GPIO from
  * @propname:	Name of property containing gpio specifier(s)
  * @index:	index of the GPIO
  *
  * Returns GPIO number to use with Linux generic GPIO API, or one of the errno
  * value on the error condition.
  */
  gpbank->pinNo = of_get_named_gpio(pdev->dev.of_node, "gpios", 0);
  pr_info("%s : GPIO Pin Number for LED %d\r\n",__func__,gpbank->pinNo);

  /*
  * of_parse_phandle - Resolve a phandle property to a device_node pointer
  * @np: Pointer to device node holding phandle property
  * @phandle_name: Name of property holding a phandle value
  * @index: For properties holding a table of phandles, this is the index into
  *         the table
  *
  * Returns the device_node pointer with refcount incremented.  Use
  * of_node_put() on it when done.
  */
  pnode = of_parse_phandle(pdev->dev.of_node, "gpios", 0);
  
  /**
  * of_property_read_u32_array - Find and read an array of 32 bit integers
  * from a property.
  *
  * @np:		device node from which the property value is to be read.
  * @propname:	name of the property to be searched.
  * @out_values:	pointer to return value, modified only if return value is 0.
  * @sz:		number of array elements to read
  *
  * Search for a property in a device node and read 32-bit value(s) from
  * it. Returns 0 on success, -EINVAL if the property does not exist,
  * -ENODATA if property does not have a value, and -EOVERFLOW if the
  * property data isn't large enough.
  *
  * The out_values is modified only if a valid u32 value can be decoded.
  */
  of_property_read_u32(pnode, "reg", &gpbank->gpioBase);
  pr_info("%s : GPIO Bank Base Address %x\r\n",__func__,gpbank->gpioBase);

  baseAddr = ioremap(gpbank->gpioBase, 4095);
  if(NULL == baseAddr)
    return -1;
  pr_info("%s : GPIO Bank Base Address As Seen By Kernel %p\r\n",__func__,baseAddr);

  kobj = kobject_create_and_add("external_led", NULL);
  if(!kobj)
    return -ENOMEM;

  if(sysfs_create_file(kobj, &led_attribute.attr))
    kobject_put(kobj);

  return 0;
}

static int extled_remove(struct platform_device *pdev)
{
  pr_info("%s : Remove Invoked\r\n",__func__);
  kobject_put(kobj);
  iounmap(baseAddr);
  kfree(gpbank);
  return 0;
}

/* Used for matching device compatible field in driver */
static const struct of_device_id extled_of_mtable[] = {
  { .compatible = "gpio_extled", },
  {}
};

MODULE_DEVICE_TABLE(of, extled_of_mtable);

static struct platform_driver extled_driver = {
  .driver = {
    .name           = "Off Chip LED Driver", 
    .owner          = THIS_MODULE,
    .of_match_table = extled_of_mtable,
  },
  .probe  = extled_probe,
  .remove = extled_remove,
};

static int __init my_module_init(void)
{
  pr_info("%s : External LED - Driver Inserted\r\n",__func__);
  return(platform_driver_register(&extled_driver)); 
}

static void __exit my_module_exit(void)
{
  pr_info("%s : External LED - Driver Removed\r\n",__func__);
  platform_driver_unregister(&extled_driver);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_DESCRIPTION("Platform Driver for Off Chip External LED Device by Using Registers");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

