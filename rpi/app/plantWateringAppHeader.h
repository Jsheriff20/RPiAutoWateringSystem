
/*
 *  chardev.h - the header file with the ioctl definitions.
 *
 *  The declarations here have to be in a header file, because
 *  they need to be known both to the kernel module
 *  (in chardev.c) and the process calling ioctl (ioctl.c)
 */

#ifndef PIIO_H
#define PIIO_H

#include <linux/ioctl.h>

typedef struct gpio_pin {
	char desc[16];
	unsigned int pin;
	int value;
	char opt;
} gpio_pin;

#define IOCTL_READ 0x67
#define IOCTL_WRITE 0x68


#endif
