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
 * insmod cpu_audio.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree cpu_audio.c
 */

#include <linux/module.h>
#include <linux/delay.h>
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
#include "cpu_audio.h"

#define DRIVER_NAME "cpu_audio"

/*
 * Information about our device
 */
struct cpu_audio_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
} dev;

static struct sample *smpArr;

// Read the whole transform length from the fft
static int readAudio(struct sample *smpArr)
{
	int i;
	// First, get the number of elements
	*((unsigned int *) smpArr) = ioread32(dev.virtbase);
	// If there aren't enough samples, return
	// if ((uint16_t) smpArr[0].left % 10 == 0)
	// 	printk("Samples read %u, %u\n", (uint16_t) smpArr[0].left, (uint16_t) smpArr[0].right);
	if ((uint16_t) smpArr[0].left < SAMPLENUM || (uint16_t) smpArr[0].right < SAMPLENUM) {
		// printk("Only %u samples.\n", (uint16_t) smpArr[0].left);
		return 1;
	}
	// if ((uint16_t) smpArr[0].left % 10 == 0)
	// 	printk("Copying: %u samples\n", (uint16_t) smpArr[0].left);
	// printk("size of struct: %lu, unsigned int: %d\n", sizeof(struct sample), sizeof(unsigned int));
	ioread32_rep(dev.virtbase + 4, smpArr, SAMPLENUM);
	// for (i = 0; i < SAMPLENUM; i++) {
	// 	((unsigned int *) smpArr)[i] = ioread32(dev.virtbase + 4);
	// 	// printk("Left: %d, Right %d.\n", smpArr[i].left, smpArr[i].right);
	// }
	return 0;
}

static void writeAudio(struct sample *smpArr)
{
	// First, determine how empty the buffer is
	struct sample buffer;
	int pos = 0, i;
	unsigned int current_write_size;
	printk("Called\n");
	do {
		// address 3 indicates the number of words in the write FIFO
		// for (i = 0; i < 1000; i++)
		*((unsigned int *) &buffer) = ioread32(dev.virtbase + 8);
		printk("Value read: 0x%x\n", *((unsigned int *) &buffer));
		// Assuming they're roughly equal
		current_write_size = min(BUFFER_SIZE - (u16) buffer.left, BUFFER_SIZE - (u16) buffer.right);
		// If we can't write anything skip
		printk("Write: %d, current: %d\n", current_write_size, max((u16) buffer.left, (u16) buffer.right));
		if (current_write_size < SAMPLENUM * 2 - 1) continue; // only write full bursts;
		// // Write the appropriate number
		iowrite32_rep(dev.virtbase, smpArr, SAMPLENUM);
		// for (i = 0; i < SAMPLENUM; i++) {
		// 	iowrite32(((unsigned int *) smpArr)[pos++], dev.virtbase);
		// }
		break;
	} while (1);
}

/*
 * Handle ioctl() calls from userspace:
 * Note extensive error checking of arguments
 */
static long cpu_audio_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	// int i;
	char *to, *from;
    to = (char *) arg;
    from = (char *) smpArr;
    if (!smpArr)
    	return -ENOMEM;

	switch (cmd) {
	case CPU_AUDIO_READ_SAMPLES:
		if (copy_from_user(smpArr, (struct sample *) arg,
				   sizeof(struct sample) * SAMPLENUM)) {
			return -EACCES;
		}
		// If the returns is non zero, then tell the user to try again
		if (readAudio(smpArr)) {
			return -EAGAIN;
		}
		// for (i = 0; i < SAMPLENUM * sizeof(struct sample); i++) {
		// 	if (copy_from_user(from + i,  to + i, 1) != 0)
		// 		break;
		// }
		// printk("Bytes read: %d, expected size: %d\n", i, sizeof(struct sample) * SAMPLENUM);
		if (copy_to_user((struct sample *) arg, smpArr,
				 sizeof(struct sample) * SAMPLENUM)) {
			return -EACCES;
		}
        printk("user location: %p\n", (void *) arg);
		break;

	case CPU_AUDIO_WRITE_SAMPLES:
		if (copy_from_user(smpArr, (struct sample *) arg,
				   sizeof(struct sample) * SAMPLENUM)) {
			return -EACCES;
		}
		writeAudio(smpArr);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations cpu_audio_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = cpu_audio_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice cpu_audio_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &cpu_audio_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init cpu_audio_probe(struct platform_device *pdev)
{
	int ret;

	/* Register ourselves as a misc device: creates /dev/cpu_audio */
	ret = misc_register(&cpu_audio_misc_device);

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
	misc_deregister(&cpu_audio_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int cpu_audio_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&cpu_audio_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id cpu_audio_of_match[] = {
	{ .compatible = "altr,cpu_audio" },
	{},
};
MODULE_DEVICE_TABLE(of, cpu_audio_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver cpu_audio_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(cpu_audio_of_match),
	},
	.remove	= __exit_p(cpu_audio_remove),
};

/* Calball when the module is loaded: set things up */
static int __init cpu_audio_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	smpArr = kmalloc(SAMPLENUM * sizeof(struct sample), GFP_KERNEL);
	return platform_driver_probe(&cpu_audio_driver, cpu_audio_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit cpu_audio_exit(void)
{
	platform_driver_unregister(&cpu_audio_driver);
	kfree(smpArr);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(cpu_audio_init);
module_exit(cpu_audio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MA2799, SS3912");
MODULE_DESCRIPTION("CPU Audio for MuseBox");

