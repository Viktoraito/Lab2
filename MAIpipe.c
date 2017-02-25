#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab 2 pipe module");
MODULE_AUTHOR("M. Kofman");

#define DEVICE_NAME "MAIpipe"
#define ROOT_UID 0

static size_t buf_size = 16;

static kuid_t rootUid =
{
	.val = ROOT_UID
};

struct ring_buf {
	kuid_t uid;
	char *buf;
	size_t start;
	size_t end;
};

int getbuf_byid(struct ring_buf *buf, int nbuf, kuid_t byuid) {
	int i=0;
	for(; i<nbuf; i++)
		if(uid_eq(buf[i].uid, byuid))
			return i;
	return -1;
}

int sizeof_buf(struct ring_buf *buf, int nbuf, kuid_t byuid) {
	size_t st=buf[getbuf_byid(buf,nbuf,byuid)].start;
	size_t nd=buf[getbuf_byid(buf,nbuf,byuid)].end;
	if(nd>=st)
		return nd-st;
	else
		return buf_size-(nd-st);
}

char *getbufstr(struct ring_buf *buf, int nbuf, kuid_t byuid) {
	char *ret;
	int sizeof_buf;
	size_t st=buf[getbuf_byid(buf,nbuf,byuid)].start;
	size_t nd=buf[getbuf_byid(buf,nbuf,byuid)].end;
	if(nd>=st)
		sizeof_buf = nd-st+1;
	else
		sizeof_buf = buf_size-(st-nd)+1;

	ret = kmalloc(sizeof_buf,GFP_KERNEL);
	if(nd>=st) {
		strncpy(ret, buf[getbuf_byid(buf,nbuf,byuid)].buf + st, nd - st);
		ret[nd-st] = '\0';
	}
	else {
		strncpy(ret, buf[getbuf_byid(buf,nbuf,byuid)].buf + st, buf_size - st);
		strncat(ret, buf[getbuf_byid(buf,nbuf,byuid)].buf, nd);
		ret[buf_size-(st-nd)] = '\0';
	}
	return ret;
}

void end_dec(struct ring_buf *buf, int nbuf, kuid_t byuid, size_t count) {
	size_t nd=buf[getbuf_byid(buf,nbuf,byuid)].end;
	if(nd-count >= 0)
		buf[getbuf_byid(buf,nbuf,byuid)].end-=count;
	else
		buf[getbuf_byid(buf,nbuf,byuid)].end=buf_size-(count-nd);
}

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
	.owner = THIS_MODULE,
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

	pr_warning("You should create dev file with:\n 'mknod -m 777 /dev/MAIpipe c %d 0'\n", major);
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
		pr_alert("Initialising ring buffer failed at #%d call\n",num_of_bufs-1);
		return -1;		
	}

	pipe_buf[num_of_bufs-1].buf = kmalloc(buf_size*sizeof(char), GFP_KERNEL); //alocate memory for buf inside of ring buf
	if(!pipe_buf[num_of_bufs-1].buf) {
		pr_alert("Allocating buffer failed with %d size\n",buf_size);
		return -1;
	}
	pipe_buf[num_of_bufs-1].uid = get_current_user()->uid;
	pipe_buf[num_of_bufs-1].start=0; pipe_buf[num_of_bufs-1].end=0;
	pr_warning("Initialising #%d ring buffer with %d size\n",num_of_bufs-1,buf_size);
   }

	/*temp:*/
	pipe_buf[num_of_bufs-1].buf="test";
	pipe_buf[num_of_bufs-1].end=4;

	return 0;
}

static int pipe_release(struct inode *i, struct file *f) {
	pr_warning("Releasing #%d buf\n",getbuf_byid(pipe_buf,num_of_bufs,get_current_user()->uid));
	/*kfree(pipe_buf[getbuf_byid(pipe_buf,num_of_bufs,get_current_user()->uid)].buf);
	num_of_bufs--;*/
	kfree(pipe_buf);
	return 1;
}

/**
 * TODO
 * some magic happens with size: it's existing and take part in some processes, but printing always as '0'
 * also some crash happens at kfree() function in commented part
 * should I add sleep procedure already?
 * Also I should add root-warning message
 */


static ssize_t pipe_read(struct file *u_file, char __user *u_buf, size_t count, loff_t *pos) {
	int size = sizeof_buf(pipe_buf,num_of_bufs,get_current_user()->uid);
	//pr_alert("count: %d, pos: %d, size: %d\n",count,*pos,sizeof_buf(pipe_buf,num_of_bufs,get_current_user()->uid));
	if(*pos >= size) {
		//pr_alert("count: %d, pos: %d, size: %d\n",count,*pos,sizeof_buf(pipe_buf,num_of_bufs,get_current_user()->uid));
		return 0;
	}
	if(*pos + count > size) {
		//pr_alert("count: %d, pos: %d, size: %d\n",count,*pos,sizeof_buf(pipe_buf,num_of_bufs,get_current_user()->uid));
		count = size - *pos;
	}
	if(copy_to_user(u_buf, getbufstr(pipe_buf,num_of_bufs,get_current_user()->uid) + *pos, count) != 0 ) {
		//pr_alert("count: %d, pos: %d, size: %d\n",count,*pos,sizeof_buf(pipe_buf,num_of_bufs,get_current_user()->uid));
		return -1;
	}
	//pr_alert("count: %d, pos: %d, size: %d\n",count,*pos,sizeof_buf(pipe_buf,num_of_bufs,get_current_user()->uid));
	end_dec(pipe_buf,num_of_bufs,get_current_user()->uid,count);
	*pos += count;
	//pr_alert("count: %d, pos: %d, size: %d\n",count,*pos,sizeof_buf(pipe_buf,num_of_bufs,get_current_user()->uid));
	return count;
}

/**
 * TODO
 * write into pipe_buf
 * magic with st/nd
 * root-warning message
 */

static ssize_t pipe_write(struct file *f, const char *c, size_t s, loff_t *l) {
	pr_alert("Procedure is not realised yet!\n");
	return -1;
}
