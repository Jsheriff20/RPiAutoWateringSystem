
#ifndef plantWatering_H
#define plantWatering_H

#include <linux/ioctl.h>


typedef struct gpio_pin {
	char desc[16];
	unsigned int pin;
	int value;
	char opt;
} gpio_pin;

#define IOCTL_WRITE 0x68

#define  DEVICE_NAME "moistureDev"
#define  CLASS_NAME  "moistureclass"

#endif
