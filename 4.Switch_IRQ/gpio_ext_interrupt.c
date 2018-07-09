/* Off board Push Switch connected to GPIO Bank 1 Pin 13
 * Configured to Generate Interrupt when Switch is Pressed
 * Physicaly connected on the P8 header to Pin 11
 * The pin is configured as ACTIVE LOW
*/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

static int gpio_obj, gpio_irq;

/* Interrupt Handler */
static irqreturn_t gpio_button_isr(int irq, void *arg)
{
  pr_info("%s : IRQ %d\r\n",__func__,irq);
  return IRQ_HANDLED;
}

static int gpio_button_probe(struct platform_device *pdev)
{
  int ret;

  pr_info("%s : Probe Invoked\r\n",__func__);
  
  gpio_obj = of_get_named_gpio(pdev->dev.of_node, "gpios", 0);
  pr_info("%s : Linux GPIO Number %d\r\n",__func__,gpio_obj);

  ret = gpio_request(gpio_obj, NULL);
  if(ret < 0)
    return ret;

  /* Sets the debounce value to 200 microseconds */
  /* Prevents repeated interrupt generation */
  gpio_set_debounce(gpio_obj, 200);

  /* Resolve the IRQ Line from GPIO */
  gpio_irq = gpio_to_irq(gpio_obj);
  if(gpio_irq < 0)
    pr_info("%s : Couldn't Get IRQ Line\r\n",__func__);

  /* Register Handler with IRQ Line */
  ret = request_threaded_irq(gpio_irq, gpio_button_isr, NULL, IRQF_TRIGGER_RISING, "button_isr", NULL);
  if(ret < 0)
    pr_info("%s : Interrupt Registration Failed\r\n",__func__);

  pr_info("%s : IRQ Line %d\r\n",__func__,gpio_irq);  
  return ret;
}

static int gpio_button_remove(struct platform_device *pdev)
{
  pr_info("%s : Remove Invoked\r\n",__func__);
  free_irq(gpio_irq, NULL);
  gpio_free(gpio_obj);
  return 0;
}

static const struct of_device_id gpio_button_of_mtable[] = {
  { .compatible = "gpio_button_intr", },
  {}
};

MODULE_DEVICE_TABLE(of, gpio_button_of_mtable);

static struct platform_driver gpio_button_driver = {
  .driver = {
    .name           = "GPIO Button Driver", 
    .owner          = THIS_MODULE,
    .of_match_table = gpio_button_of_mtable,
  },
  .probe  = gpio_button_probe,
  .remove = gpio_button_remove,
};

static int __init my_module_init(void)
{
  pr_info("%s : External GPIO Button - Driver Inserted\r\n",__func__);
  return platform_driver_register(&gpio_button_driver);
}

static void __exit my_module_exit(void)
{
  pr_info("%s : External GPIO Button - Driver Removed\r\n",__func__);
  platform_driver_unregister(&gpio_button_driver);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_DESCRIPTION("Platform Driver for External Button on GPIO Configured as Interrupt");
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_LICENSE("GPL");

