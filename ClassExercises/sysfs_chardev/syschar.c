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
#include<linux/mutex.h>
#include<linux/list.h>

#define MAX_BUF_SIZE 8192
#define DEVNAME "demo"

static int major;
atomic_t  device_opened;
static unsigned buf_size = MAX_BUF_SIZE;
static unsigned current_usage;

struct kobject *demodev_kobj;
struct kobject *subdir_kobj;
static char pid[10];
static pid_t cpid;

static unsigned total_bytes_read;
static unsigned total_bytes_written;
char* device_buffer = NULL;

struct mutex dev_mutex;

struct kobj_list {
  struct kobject *kobj;
  struct list_head list;
};

struct kobj_list demodev_kobj_list;
struct kobj_list *tmp;
struct list_head *pos, *q;

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
  kfree(device_buffer);
  device_buffer = (char*)kmalloc(buf_size, GFP_KERNEL);
  return count;
}
static struct kobj_attribute demodev_buf_size_attribute = __ATTR(buffer_size,0644,demodev_buf_size_show, demodev_buf_size_set);

static ssize_t demodev_usage_count_show(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", atomic_read(&device_opened));
}
static struct kobj_attribute demodev_usage_count_attribute = __ATTR(usage_count, 0444,demodev_usage_count_show, NULL);


static ssize_t demodev_current_usage_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
  return sprintf(buf, "%d\n", current_usage);
}
static struct kobj_attribute demodev_current_usage_attribute = __ATTR(current_usage, 0444, demodev_current_usage_show, NULL);


static ssize_t demodev_total_bytes_read_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf){
  return sprintf(buf, "%d\n", total_bytes_read);
}
static struct kobj_attribute demodev_total_bytes_read_attribute = __ATTR(total_bytes_read, 0444, demodev_total_bytes_read_show, NULL);


static ssize_t demodev_total_bytes_written_show(struct kobject *kobj, struct kobj_attribute *attr, char* buf){
  return sprintf(buf, "%d\n", total_bytes_written);
}
static struct kobj_attribute demodev_total_bytes_written_attribute = __ATTR(total_bytes_written, 0444, demodev_total_bytes_written_show, NULL);


static struct attribute *demodev_attrs[] = {
        &demodev_buf_size_attribute.attr,
        &demodev_usage_count_attribute.attr,
        &demodev_current_usage_attribute.attr,
        &demodev_total_bytes_read_attribute.attr,
        &demodev_total_bytes_written_attribute.attr,
        NULL,
};
static struct attribute_group demodev_attr_group = {
        .attrs = demodev_attrs,
        /*.name = "demodev",*/
};



static int demo_open(struct inode *inode, struct file *file)
{
  /*if(atomic_read(&device_opened)){*/
    /*printk(KERN_INFO "Only one process is allowed to open!!\n");*/
    /*return -EINVAL;*/
  /*}*/
  int ret;
  atomic_inc(&device_opened);
  try_module_get(THIS_MODULE);
  printk(KERN_INFO "Device opened successfully\n");
  cpid = current->pid;
  snprintf(pid, 10, "%d", (int)cpid);
  subdir_kobj = kobject_create_and_add(pid, demodev_kobj);
  BUG_ON(!subdir_kobj);
  ret = sysfs_create_group (subdir_kobj, &demodev_attr_group);

  tmp = (struct kobj_list*)kmalloc(sizeof(struct kobj_list), GFP_KERNEL);
  tmp->kobj = subdir_kobj;
  list_add(&(tmp->list), &(demodev_kobj_list.list));

  if(unlikely(ret)){
      printk(KERN_INFO "demodev: can't create sysfs\n");
      BUG_ON(1);
  }
  return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        cpid = current->pid;
        snprintf(pid, 10, "%d", (int)cpid);
        list_for_each_safe(pos, q, &demodev_kobj_list.list){
          tmp = list_entry(pos, struct kobj_list, list);
          if(!strcmp(tmp->kobj->name, pid)){
                subdir_kobj = tmp->kobj;
                list_del(pos);
                kfree(tmp);
              }
        }
        sysfs_remove_group(subdir_kobj, &demodev_attr_group);
        kobject_del(subdir_kobj);
        printk(KERN_INFO "Device closed successfully\n");
        return 0;
}
static ssize_t demo_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset)
{
  if(length > current_usage)
    length = current_usage;
  mutex_lock_interruptible(&dev_mutex);
  printk(KERN_INFO "Read!\n");
  copy_to_user(buffer, device_buffer, length); 
  mutex_unlock(&dev_mutex);
  current_usage = 0;
  /*printk(KERN_INFO "Sorry, this operation isn't supported.\n");*/
  *offset += length;
  total_bytes_read += length;
  return length;
}

static ssize_t
demo_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
  if(len > buf_size)
    len = buf_size;
  mutex_lock_interruptible(&dev_mutex);
  printk(KERN_INFO "Write!\n");
  copy_from_user(device_buffer, buff, len);
  mutex_unlock(&dev_mutex);
  current_usage = len;
  /*printk(KERN_INFO "Sorry, this operation isn't supported.\n");*/
  *off += len;
  total_bytes_written += len;
  return len;
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
  demodev_kobj = kobject_create_and_add("demodev", kernel_kobj);
   /*demodev_kobj = kernel_kobj;*/
   BUG_ON(!demodev_kobj);
   ret = sysfs_create_group (demodev_kobj, &demodev_attr_group);
   if(unlikely(ret))
          printk(KERN_INFO "demodev: can't create sysfs\n");
 
   INIT_LIST_HEAD(&demodev_kobj_list.list);
  mutex_init(&dev_mutex);
  device_buffer = (char*)kmalloc(MAX_BUF_SIZE, GFP_KERNEL);
  memset(device_buffer, 0, MAX_BUF_SIZE);
	return 0;
}

void cleanup_module(void)
{
  unregister_chrdev(major, DEVNAME);
  sysfs_remove_group (kernel_kobj, &demodev_attr_group);
  kfree(device_buffer);
	printk(KERN_INFO "Goodbye kernel\n");
}
MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
