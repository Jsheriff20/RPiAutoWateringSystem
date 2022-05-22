#include "plantWateringHeader.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/seq_file.h>

static int DevBusy = 0;
static int MajorNum = 100;
static struct class*  ClassName  = NULL;
static struct device* DeviceName = NULL;

gpio_pin apin;

static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "plantWateringDriver: device_open(%p)\n", file);

	if (DevBusy)
		return -EBUSY;

	DevBusy++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "plantWateringDriver: device_release(%p)\n", file);
	DevBusy--;

	module_put(THIS_MODULE);
	return 0;
}

static int device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

	switch (cmd) {	
		case IOCTL_WRITE:
			/*copy data from the user space format: (where its going to be set (destination address in kernal space), where its coming from (address in user space), number of byes to copy  */
			copy_from_user(&apin, (gpio_pin *)arg, sizeof(gpio_pin));
			
			/*allocate GPIO */ 
			gpio_request(apin.pin, apin.desc);
			
			/*specify the pin (apin.pin) to be used for output   */
			gpio_direction_output(apin.pin, 0);
			
			/*set the value of the pin */
			gpio_set_value(apin.pin, apin.value);
			printk("pi:%u - val:%i - desc:%s\n" , apin.pin , apin.value , apin.desc);
			break;
		default:
			printk("command not found\n");
	}
	
	
	return 0;
}

struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

static int __init plantWateringDriver_init(void){
	int ret_val;
	ret_val = 0;

	printk(KERN_INFO "plantWateringDriver: Initializing the plantWateringDriver\n");
	/* create and register a cdev occupying a range of minors */
	MajorNum = register_chrdev(0, DEVICE_NAME, &Fops);
	    if (MajorNum<0){
	        printk(KERN_ALERT "plantWateringDriver: failed to register a major number\n");
	        return MajorNum;
	    }
	printk(KERN_INFO "plantWateringDriver: registered with major number %d\n", MajorNum);

	ClassName = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(ClassName)){
	    unregister_chrdev(MajorNum, DEVICE_NAME);
	    printk(KERN_ALERT "plantWateringDriver: Failed to register device class\n");
	    return PTR_ERR(ClassName);
	}
	printk(KERN_INFO "plantWateringDriver: device class registered\n");

	DeviceName = device_create(ClassName, NULL, MKDEV(MajorNum, 0), NULL, DEVICE_NAME);
	if (IS_ERR(DeviceName)){
	    class_destroy(ClassName);
	    unregister_chrdev(MajorNum, DEVICE_NAME);
	    printk(KERN_ALERT "plantWateringDriver: Failed to create the device\n");
	    return PTR_ERR(DeviceName);
	}
printk(KERN_INFO "plantWateringDriver: device class created\n");

	return 0;
}

static void __exit plantWateringDriver_exit(void){
	   device_destroy(ClassName, MKDEV(MajorNum, 0));
	   class_unregister(ClassName);
	   class_destroy(ClassName);
	   unregister_chrdev(MajorNum, DEVICE_NAME);
	   gpio_free(apin.pin);
	   printk(KERN_INFO "plantWateringDriver: Module removed\n");
}
module_init(plantWateringDriver_init);
module_exit(plantWateringDriver_exit);
MODULE_AUTHOR("Jack Sheriff");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RPi driver to control a water pump and soil moisutre sensor ");
MODULE_VERSION("0.1");
