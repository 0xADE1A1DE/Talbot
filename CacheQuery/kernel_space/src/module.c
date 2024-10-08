#include <definitions.h>

#include <query.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/fs.h>

static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char *query = kmalloc(sizeof(char) * (count + 1), GFP_KERNEL);
	char *final_cache_state;

	if(copy_from_user((void *)query, (void *)buf, count + 1) != 0)
	{
		return -1;
	}

	final_cache_state = query_cache(query, count);

	if (copy_to_user(buf, final_cache_state, count))
	{
		kfree(final_cache_state);
		return -1;
	}

	kfree(query);
	kfree(final_cache_state);

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
