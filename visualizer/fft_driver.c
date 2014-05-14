/*
 * Device driver for FFT
 *
 * A Platform device implemented using the misc subsystem
 *
 * Stephen A. Edwards
 * Columbia University
 * edited by ma2799 and ss3912
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 * "make" to build
 * insmod fft_driver.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree fft_driver.c
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "fft_driver.h"

#define DRIVER_NAME "fft_driver"

/*
 * Information about our device
 */
struct fft_driver_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
} dev;

// Read the whole transform length from the fft
static void readTransform(complex_num *dataArray)
{
	int amountRead = 0;
	s16 *cnum = kmalloc(2 * sizeof(s16), GFP_KERNEL);
	s16 *ack = kmalloc(2 * sizeof(s16), GFP_KERNEL);
	// Keep reading while we haven't retrieved all values
	while (amountRead <= SAMPLENUM) {
    	*((unsigned int *) cnum) = ioread32(dev.virtbase);
    	*((unsigned int *) ack) = ioread32(dev.virtbase + 4);
    	// If the data was good
    	if (ack[1]) {
    		if (amountRead != SAMPLENUM) {
    			dataArray[amountRead].real = cnum[0];
    			dataArray[amountRead].imag = cnum[1];
    		} else {
    			dataArray[amountRead].real = ack[0];
    			dataArray[amountRead].imag = ack[1];
    		}
    		amountRead++;
    	}
    }
    kfree(cnum);
    kfree(ack);
}

/*
 * Handle ioctl() calls from userspace:
 * Note extensive error checking of arguments
 */
static long fft_driver_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    struct complex_num *dataArray = kmalloc(SAMPLENUM * sizeof(complex_num), GFP_KERNEL); //allocating space for data array
    
	switch (cmd) {
	case FFT_DRIVER_READ_TRANSFORM:
		if (copy_from_user(dataArray, (complex_num *) arg,
				   sizeof(complex_num) * SAMPLENUM + 1)) {
			kfree(dataArray);
			return -EACCES;
		}
		readTransform(dataArray); //read into dataArray
		if (copy_to_user((complex_num *) arg, dataArray,
				 sizeof(complex_num) * SAMPLENUM + 1)) {
			kfree(dataArray);
			return -EACCES;
		}
		break;

	default:
		kfree(dataArray);
		return -EINVAL;
	}

	kfree(dataArray);
	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations fft_driver_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = fft_driver_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice fft_driver_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &fft_driver_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init fft_driver_probe(struct platform_device *pdev)
{
	int ret;

	/* Register ourselves as a misc device: creates /dev/fft_driver */
	ret = misc_register(&fft_driver_misc_device);

	/* Get the address of our registers from the device tree */
	ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	if (ret) {
		ret = -ENOENT;
		goto out_deregister;
	}

	/* Make sure we can use these registers */
	if (request_mem_region(dev.res.start, resource_size(&dev.res),
			       DRIVER_NAME) == NULL) {
		ret = -EBUSY;
		goto out_deregister;
	}

	/* Arrange access to our registers */
	dev.virtbase = of_iomap(pdev->dev.of_node, 0);
	if (dev.virtbase == NULL) {
		ret = -ENOMEM;
		goto out_release_mem_region;
	}


	return 0;

out_release_mem_region:
	release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
	misc_deregister(&fft_driver_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int fft_driver_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&fft_driver_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id fft_driver_of_match[] = {
	{ .compatible = "altr,aud_to_fft" },
	{},
};
MODULE_DEVICE_TABLE(of, fft_driver_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver fft_driver_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(fft_driver_of_match),
	},
	.remove	= __exit_p(fft_driver_remove),
};

/* Calball when the module is loaded: set things up */
static int __init fft_driver_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	return platform_driver_probe(&fft_driver_driver, fft_driver_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit fft_driver_exit(void)
{
	platform_driver_unregister(&fft_driver_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(fft_driver_init);
module_exit(fft_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MA2799, SS3912");
MODULE_DESCRIPTION("FFT Driver for MuseBox");

