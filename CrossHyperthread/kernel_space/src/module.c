#include <evaluation.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>

static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char *output = kzalloc(count, GFP_KERNEL);
	char *tmp;

	if (!output)
		return -1;

	// run test quietly once, because of inconsistent results in the first run after inserting the module.
	tmp = evaluate_hyperthreaded_pcid_support(2);
	kfree(tmp);

	for(int level = 2; level <= 4; level++)
	{
		tmp = evaluate_hyperthreaded_pcid_support(level);
		strcat(output, tmp);
		kfree(tmp);
	}

	if(copy_to_user(buf, output, count))
	{
		kfree(output);
		return -1;
	}

	kfree(output);

	return 0;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return 0;
}

static struct file_operations fops =
{
	.open = device_open,
	.read = device_read,
	.write = device_write,
	.owner = THIS_MODULE,
};

static struct miscdevice misc_dev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "trcrt",
	.fops = &fops,
	.mode = S_IRWXUGO,
};

int init_module(void)
{
	int ret;

	ret = misc_register(&misc_dev);
	if (ret != 0) {
		printk(KERN_ALERT "[-] trcrt: failed to register device with %d\n", ret);
		return -1;
	}

	printk(KERN_INFO "[*] trcrt: initialized.\n");

	return 0;
}

void cleanup_module(void)
{
	misc_deregister(&misc_dev);
	printk(KERN_INFO "[*] trcrt: cleaned up.\n");
}

MODULE_AUTHOR("Philipp Ertmer");
MODULE_DESCRIPTION("Translation Cache Reversing Tool");
MODULE_LICENSE("GPL");
