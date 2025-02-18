#include <experiments.h>
#include <eviction_sets.h>
#include <definitions.h>
#include <tlb.h>
#include <evaluation.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>

#define DEFAULT_TRANSLATION_CACHE {99, 99, NULL}

static long tlb_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	switch(cmd)
	{
		case IOCTL_SET_TLB_INFO:
			if (copy_from_user(tlbs, (struct tlb __user *)arg, sizeof(tlbs))) {
				printk(KERN_DEBUG"[-] Could not receive TLB info from user space\n");
				return -EFAULT;
			}
			printk(KERN_DEBUG"[*] Received TLB info from user space\n");
			break;
		default:
			return -ENOTTY;
	}
	return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct translation_cache translation_caches[5] = {{-1, -1, NULL}, {-1, -1, NULL}, DEFAULT_TRANSLATION_CACHE, DEFAULT_TRANSLATION_CACHE, DEFAULT_TRANSLATION_CACHE};

	char *output = kzalloc(count, GFP_KERNEL);
	char *tmp;

	if (!output)
		return -1;

	for(int level = 2; level <= 4; level++)
	{
		tmp = evaluate_cache_existence(level);
		strcat(output, tmp);
		kfree(tmp);
	}

	for(int level = 2; level <= 4; level++)
	{
		tmp = evaluate_cache_hierarchy(level);
		strcat(output, tmp);
		kfree(tmp);
	}

	for(int level = 2; level <= 4; level++)
	{
		tmp = evaluate_cache_structure(translation_caches, level);
		strcat(output, tmp);
		kfree(tmp);
	}

	for(int level = 2; level <= 4; level++)
	{
		tmp = evaluate_replacement_policy(translation_caches, level);
		strcat(output, tmp);
		kfree(tmp);
	}

	for(int level = 2; level <= 3; level++)
	{
		tmp = evaluate_nesting(translation_caches, level);
		strcat(output, tmp);
		kfree(tmp);
	}

	for(int level = 2; level <= 3; level++)
	{
		tmp = evaluate_huge_TLB(level);
		strcat(output, tmp);
		kfree(tmp);
	}

	for(int level = 2; level <= 3; level++)
	{
		test_huge_page_eviction(level);
	}

	for(int level = 2; level <= 4; level++)
	{
		tmp = evaluate_supported_pcids(level);
		strcat(output, tmp);
		kfree(tmp);
	}

	for(int level = 2; level <= 4; level++)
	{
		tmp = evaluate_pcid_respect(level);
		strcat(output, tmp);
		kfree(tmp);
	}

	if (copy_to_user(buf, output, count))
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
	.unlocked_ioctl = tlb_ioctl,
	.owner = THIS_MODULE,
};

static struct miscdevice misc_dev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "talbot",
	.fops = &fops,
	.mode = S_IRWXUGO,
};

int init_module(void)
{
	int ret;

	ret = misc_register(&misc_dev);
	if (ret != 0) {
		printk(KERN_ALERT "[-] talbot: failed to register device with %d\n", ret);
		return -1;
	}

	printk(KERN_INFO "[*] talbot: initialized.\n");

	return 0;
}

void cleanup_module(void)
{
	misc_deregister(&misc_dev);
	printk(KERN_INFO "[*] talbot: cleaned up.\n");
}

MODULE_AUTHOR("Philipp Ertmer");
MODULE_DESCRIPTION("Translation Cache Reversing Tool");
MODULE_LICENSE("GPL");
