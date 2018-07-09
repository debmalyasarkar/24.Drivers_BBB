/* Off board LED connected to GPIO Bank 0 Pin 27 
 * Physicaly connected on the P8 header to Pin 17
 * By default the pin is configured as ACTIVE LOW
 * This driver uses sysfs interface for user interaction
 * This driver makes use of the Linux API's for GPIO Handling
*/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/gpio.h>

#define GPIO_BANK 0
#define GPIO_PIN  27

/* Global parameter which is passed to all GPIO APIs */
static unsigned int gpio_obj;

static struct kobject *kobj;

static ssize_t led_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
  int ret;

  pr_info("%s : SysFs Invoked\r\n",__func__);

  /* Get the value from GPIO */
  ret = gpio_get_value(gpio_obj);
  if(ret)
  {
    pr_info("%s : LED is ON\r\n",__func__);
    return sprintf(buf,"%d\n",(char)ret);
  }
  else
  {
    pr_info("%s : LED is OFF\r\n",__func__);
    return sprintf(buf,"%d\n",(char)ret);
  }
}

static ssize_t led_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
  int value = 0, retval = 0;

  pr_info("%s : SysFs Invoked\r\n",__func__);

  retval = sscanf(buf,"%d",&value);
  if((retval < 0) && (retval > 1))
    return -EINVAL;
  if((value < 0) && (value > 1))
    return -EINVAL;
  
  /* Set the value to GPIO */
  if(value == 0)
  {
    gpio_set_value(gpio_obj, value);
    pr_info("%s : LED Turned OFF\r\n",__func__);
  }
  else if(value == 1)
  {
    gpio_set_value(gpio_obj, value);
    pr_info("%s : LED Turned ON\r\n",__func__);
  }
  /* Returning 0 causes this function to be invoked in endless loop */
  return count;
}

static struct kobj_attribute led_attribute = __ATTR(external_led, 0660, led_show, led_store);

static int extled_probe(struct platform_device *pdev)
{
  int ret;

  pr_info("%s : Probe Invoked\r\n",__func__);

  /* GPIO's numbers are linearly calculated as per SOC */
  gpio_obj = (32 * GPIO_BANK) + GPIO_PIN;
  pr_info("%s : Linux GPIO Number = %u, GPIO_BANK = %d, GPIO_PIN = %d\r\n",__func__,gpio_obj, GPIO_BANK, GPIO_PIN);

  ret = gpio_request(gpio_obj, NULL);
  if(ret < 0)
    return ret;

  ret = gpio_direction_output(gpio_obj, 0);
  if(ret < 0)
  {
    gpio_free(gpio_obj);
    return ret;
  }
  
  kobj = kobject_create_and_add("external_led", NULL);
  if(!kobj)
  {
    gpio_free(gpio_obj);
    return -ENOMEM;
  }

  ret = sysfs_create_file(kobj, &led_attribute.attr);
  if(ret < 0)
  { 
    gpio_free(gpio_obj); 
    kobject_put(kobj);
    return ret;
  }
  return ret;
}

static int extled_remove(struct platform_device *pdev)
{
  pr_info("%s : Remove Invoked\r\n",__func__);
  gpio_free(gpio_obj);
  kobject_put(kobj);
  return 0;
}

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
  return platform_driver_register(&extled_driver);
}

static void __exit my_module_exit(void)
{
  pr_info("%s : External LED - Driver Removed\r\n",__func__);
  platform_driver_unregister(&extled_driver);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_DESCRIPTION("Platform Driver for Off Chip External LED Device using Linux Calls");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

