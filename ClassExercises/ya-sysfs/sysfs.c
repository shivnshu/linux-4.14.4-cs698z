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

#define MAX_BUF_SIZE 8192
atomic_t device_opened;
static unsigned buf_size;
static unsigned current_usage;
struct kobject *demodev_kobj;
struct kobject *subdir_kobj;

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
};

int init_module(void)
{
        int ret;
	printk(KERN_INFO "Hello kernel\n");

        /*sysfs creation*/
        demodev_kobj = kobject_create_and_add("demodev", kernel_kobj);      
        BUG_ON(!demodev_kobj);                         
        ret = sysfs_create_group (demodev_kobj, &demodev_attr_group);
        if(unlikely(ret)){
                printk(KERN_INFO "demodev: can't create sysfs\n");
                BUG_ON(1);
         }
        subdir_kobj = kobject_create_and_add("level1", demodev_kobj);      
        BUG_ON(!subdir_kobj);                         
        ret = sysfs_create_group (subdir_kobj, &demodev_attr_group);
        if(unlikely(ret)){
                printk(KERN_INFO "demodev: can't create sysfs\n");
                BUG_ON(1);
         }
	return 0;
}

void cleanup_module(void)
{
        sysfs_remove_group (subdir_kobj, &demodev_attr_group);
        kobject_del(subdir_kobj);
        sysfs_remove_group (demodev_kobj, &demodev_attr_group);
        kobject_del(demodev_kobj);
         
	printk(KERN_INFO "Goodbye kernel\n");
}
MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
