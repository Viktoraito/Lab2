#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab 2 pipe module");
MODULE_AUTHOR("M. Kofman");

#define DEVICE_NAME "MAIpipe"
#define ROOT_UID 0

static kuid_t rootUid =
{
	.val = ROOT_UID
};

struct ring_buf {
	kuid_t uid;
	char *buf;
	int start;
	int end;
};

int getbuf_byid(struct ring_buf *buf, int nbuf, kuid_t byuid) {
	int i=0;
	for(; i<nbuf; i++)
		if(uid_eq(buf[i].uid, byuid))
			return i;
	return -1;
}

static int buf_size = 16;
module_param(buf_size, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(buf_size, "Size of buffer");

static int pipe_open(struct inode *, struct file *);
static int pipe_release(struct inode *, struct file *);
static ssize_t pipe_read(struct file *, char *, size_t, loff_t *);
static ssize_t pipe_write(struct file *, const char *, size_t, loff_t *);

static int major;
struct ring_buf *pipe_buf;
int num_of_bufs=0;

static struct file_operations fops =
{
	.open = pipe_open,
	.release = pipe_release,
	.read = pipe_read,
	.write = pipe_write
};

static int __init pipe_init(void) {
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if(major < 0) {
		pr_alert("Registering the character device failed with %d\n", major);
		return -1;
	}
	pr_warning("MAIpipe module is loaded\n");

	pr_warning("You should create dev file with 'mknod -m 777 /dev/MAIpipe c %d 0'\n", major);
	return 0;
}

static void __exit pipe_exit(void) {
	kfree(pipe_buf);
	unregister_chrdev(major, DEVICE_NAME);
	pr_warning("MAIpipe module is unloaded\n");
}

module_init(pipe_init);
module_exit(pipe_exit);

static int pipe_open(struct inode *i, struct file *f) {

   if(getbuf_byid(pipe_buf,num_of_bufs,get_current_user()->uid)==-1) {
	num_of_bufs++;

	pipe_buf=krealloc(pipe_buf, num_of_bufs*sizeof(*pipe_buf), GFP_KERNEL); //allocate memory for new ring buf
	if(!pipe_buf) {
		pr_alert("Initialising ring buffer failed at #%d call",num_of_bufs-1);
		return -1;		
	}

	pipe_buf[num_of_bufs-1].buf = kmalloc(buf_size*sizeof(char), GFP_KERNEL); //alocate memory for buf inside of ring buf
	if(!pipe_buf[num_of_bufs-1].buf) {
		pr_alert("Allocating buffer failed with %d size",buf_size);
		return -1;
	}
	pipe_buf[num_of_bufs-1].uid = get_current_user()->uid;
	pipe_buf[num_of_bufs-1].start=0; pipe_buf[num_of_bufs-1].end=0;
	pr_warning("Initialising #%d ring buffer with %d size\n",num_of_bufs-1,buf_size);
   }
	return 0;
}

static int pipe_release(struct inode *i, struct file *f) {
	pr_warning("Releasing #%d buf\n",getbuf_byid(pipe_buf,num_of_bufs,get_current_user()->uid));
	kfree(pipe_buf[getbuf_byid(pipe_buf,num_of_bufs,get_current_user()->uid)].buf);
	num_of_bufs--;
	return 1;
}

static ssize_t pipe_read(struct file *f, char *c, size_t s, loff_t *l) {
	pr_alert("Procedure is not realised yet!\n");
	return -1;
}

static ssize_t pipe_write(struct file *f, const char *c, size_t s, loff_t *l) {
	pr_alert("Procedure is not realised yet!\n");
	return -1;
}
