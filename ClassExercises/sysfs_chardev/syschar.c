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
#include<linux/uaccess.h>
#include<linux/slab.h>

#define MAX_BUF_SIZE 8192
#define DEVNAME "demo"

static int major;
atomic_t  device_opened;
static unsigned buf_size = MAX_BUF_SIZE;
static unsigned current_usage = 0;

static char msg[MAX_BUF_SIZE] = {0};

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
  if(current_usage){
    printk("Only one device allowed to read!!\n");
    return -EINVAL; 
  }
  current_usage = 1;
  copy_to_user(buffer, msg, buf_size); 
  current_usage = 0;
  /*printk(KERN_INFO "Sorry, this operation isn't supported.\n");*/
  return buf_size;
}

static ssize_t
demo_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
  if(current_usage){
    printk("Only one device is allowed to write!!\n");
    return -EINVAL;
  }
  current_usage = 1;
  copy_from_user(msg, buff, buf_size);
  current_usage = 0;
  /*printk(KERN_INFO "Sorry, this operation isn't supported.\n");*/
  return buf_size;
}

static long demo_ioctl(struct file *file,
                 unsigned int ioctl_num,
                 unsigned long arg)

{
   printk(KERN_INFO "Sorry, this operation isn't supported.\n");
   return -EINVAL;
}
static struct file_operations fops = {
        .read = demo_read,
        .write = demo_write,
        .open = demo_open,
        .release = demo_release,
        .unlocked_ioctl = demo_ioctl,
};


static ssize_t demodev_buf_size_show(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", buf_size);
}

static ssize_t demodev_buf_size_set(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
        int err;
        unsigned long mode;

        if(current_usage){
            printk(KERN_INFO "Can not change size while buf being used\n");
            return -EINVAL;
        }
        err = kstrtoul(buf, 10, &mode);
        if (err || mode < 0 || mode > MAX_BUF_SIZE )
                return -EINVAL;

        buf_size = mode;
        return count;
}

static struct kobj_attribute demodev_buf_size_attribute = __ATTR(buffer_size,0644,demodev_buf_size_show, demodev_buf_size_set);

static ssize_t demodev_usage_count_show(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", atomic_read(&device_opened));
}
static struct kobj_attribute demodev_usage_count_attribute = __ATTR(usage_count, 0444,demodev_usage_count_show, NULL);

static struct attribute *demodev_attrs[] = {
        &demodev_buf_size_attribute.attr,
        &demodev_usage_count_attribute.attr,
        NULL,
};
static struct attribute_group demodev_attr_group = {
        .attrs = demodev_attrs,
        .name = "demodev",
};

int init_module(void)
{
  int ret;
  printk(KERN_INFO "Hello kernel\n");
  major = register_chrdev(0, DEVNAME, &fops);
  if (major < 0) {      
    printk(KERN_ALERT "Registering char device failed with %d\n", major);   
    return major;
  }                 

  printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
  printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVNAME, major);  
  atomic_set(&device_opened, 0);

  /*sysfs creation*/
   ret = sysfs_create_group (kernel_kobj, &demodev_attr_group);
   if(unlikely(ret))
          printk(KERN_INFO "demodev: can't create sysfs\n");
 
	return 0;
}

void cleanup_module(void)
{
  unregister_chrdev(major, DEVNAME);
  sysfs_remove_group (kernel_kobj, &demodev_attr_group);
	printk(KERN_INFO "Goodbye kernel\n");
}
MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
