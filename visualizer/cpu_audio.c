/*
 * Device driver for Visualizer
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
 * insmod visualizer.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree visualizer.c
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
#include "cpu_audio.h" 

#define DRIVER_NAME "cpu_audio"
#define SAMPLENUM 32768

/*
 * Information about our device
 */
struct cpu_audio_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
} dev;

/*
 * write slot_heights[12] to freq_spec.vs 
 */
static void write_freq_mem(freq_slot *slot)
{	
	iowrite16(slot->height, dev.virtbase + slot->addr); 	
}


/*
 * Handle ioctl() calls from userspace:
 * Note extensive error checking of arguments
 */
static long cpu_audio_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    sample_t *samples = (sample_t *) kmalloc(SAMPLENUM * sizeof(sample_t), GFP_KERNEL); 

	switch (cmd) {
	case CPU_AUDIO_READ_SAMPLES:
		if (copy_from_user(samples, (sample_t *) arg, sizeof(sample_t)))
			return -EACCES;
		read_audio_bank(samples); //write dataArray
		if (copy_to_user((sample_t *) arg, samples,
				 sizeof(sample_t) * SAMPLENUM))
			return -EACCES;
		break;

	case CPU_AUDIO_WRITE_SAMPLES:
		if (copy_from_user(&samples, (sample_t *) arg, sizeof(sample_t)))
			return -EACCES;
		write_audio_bank(samples); //write dataArray
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

	/* Register ourselves as a misc device: creates /dev/visualizer */
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
	return platform_driver_probe(&cpu_audio_driver, cpu_audio_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit cpu_audio_exit(void)
{
	platform_driver_unregister(&cpu_audio_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(cpu_audio_init);
module_exit(cpu_audio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MA2799, SS3912");
MODULE_DESCRIPTION("CPU Audio with Equalizer");
