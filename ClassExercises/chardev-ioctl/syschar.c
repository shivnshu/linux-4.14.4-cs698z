#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/mm_types.h>
#include<linux/file.h>
#include<linux/fs.h>
#include<linux/path.h>
#include<linux/dcache.h>
#include<linux/sched.h>
#include<linux/delayed_call.h>
#include<linux/fs_struct.h>

#include "syschar.h"

#define MAX_BUF_SIZE 8192
#define DEVNAME "demo"

static int major;
atomic_t  device_opened;
static unsigned readers;


static int demo_open(struct inode *inode, struct file *file)
{
        atomic_inc(&device_opened);
        try_module_get(THIS_MODULE);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        printk(KERN_INFO "Device closed successfully\n");

        return 0;
}
static ssize_t demo_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset)
{
        printk(KERN_INFO "Sorry, this operation isn't supported.\n");
        return -EINVAL;
}

static ssize_t
demo_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
        printk(KERN_INFO "Sorry, this operation isn't supported.\n");
        return -EINVAL;
}

static long demo_ioctl(struct file *file,
                 unsigned int ioctl_num,
                 unsigned long arg)

{
        int retval = -EINVAL;
        switch(ioctl_num){
              case IOCTL_GET_READERS:
                                 *((unsigned *)(arg)) = readers;
                                 retval = 0;
                                 break;
              case IOCTL_SET_READERS:
                                 readers = (unsigned int) arg;
                                 printk(KERN_INFO "Readers set to %d\n", readers);
                                 retval = 0;
                                 break;
              default:  
                         printk(KERN_INFO "Sorry, this operation isn't supported.\n");
        }
        return retval;
}
static struct file_operations fops = {
        .read = demo_read,
        .write = demo_write,
        .open = demo_open,
        .release = demo_release,
        .unlocked_ioctl = demo_ioctl,
};



int init_module(void)
{
	printk(KERN_INFO "Hello kernel\n");
        major = register_chrdev(DEV_MAJOR, DEVNAME, &fops);
        if (major < 0) {      
          printk(KERN_ALERT "Registering char device failed with %d\n", major);   
          return major;
        }                 
      
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", DEV_MAJOR);                                                              
        printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVNAME, DEV_MAJOR);  
        atomic_set(&device_opened, 0);
	return 0;
}

void cleanup_module(void)
{
        unregister_chrdev(DEV_MAJOR, DEVNAME);
	printk(KERN_INFO "Goodbye kernel\n");
}
MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
