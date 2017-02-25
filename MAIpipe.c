#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab 2 pipe module");
MODULE_AUTHOR("M. Kofman");

#define DEVICE_NAME "MAIpipe"

static int buf_size = 0;
module_param(buf_size, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(buf_size, "Size of buffer");

static int pipe_open(struct inode *, struct file *);
static int pipe_release(struct inode *, struct file *);
static ssize_t pipe_read(struct file *, char *, size_t, loff_t *);
static ssize_t pipe_write(struct file *, const char *, size_t, loff_t *);

static int major;

static struct file_operations fops =
{
	.open = pipe_open,
	.release = pipe_release,
	.read = pipe_read,
	.write = pipe_write
};

static int __init pipe_init(void) {
	pr_warning("Initialising buffer with %d size\n",buf_size);
	/*initiate buffer*/
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if(major < 0) {
		pr_alert("Registering the character device failed with %d\n", major);
		return -1;
	}
	pr_warning("MAIpipe module is loaded\n");
	pr_warning("You should create dev file with 'mknod /dev/MAIpipe c %d0'\n", major);
	return 0;
}

static void __exit pipe_exit(void) {
	unregister_chrdev(major, DEVICE_NAME);
	pr_alert("MAIpipe module is unloaded\n");
}

module_init(pipe_init);
module_exit(pipe_exit);

static int pipe_open(struct inode *inode, struct file *file) {
	
}
