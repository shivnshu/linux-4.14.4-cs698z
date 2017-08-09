#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/path.h>
#include <linux/fs_struct.h>
#include <linux/dcache.h>

extern void (*custom_sys_open_callback_function)(const char *path);

char *myPath;
module_param(myPath, charp, 0);

void redefined_func(const char *path){
  /*printk(KERN_INFO "%s %s\n", path, myPath);*/
 char abs_path[1000] = {0};
 const char *rel_path;
 int i=0, l;
 if(path[0] == '/'){
    rel_path = path;
  }
  else {
    char *dir_names[100];
    char d_slash[] = {'/', '\0'};
    struct path pwd;
    pwd = current->fs->pwd;
    struct dentry *dir_dentry;
    dir_dentry = pwd.dentry;
    while(strlen(dir_dentry->d_iname) != 1 || dir_dentry->d_iname[0] != '/'){
      /*printk("Module: %s", dir_dentry->d_iname);*/
      dir_names[i] = dir_dentry->d_iname;
      i++;
      dir_dentry = dir_dentry->d_parent;
    }
    for(;i>0;--i){
      strcat(abs_path, d_slash);
      strcat(abs_path, dir_names[i-1]);
    }
    strcat(abs_path, d_slash);
    strcat(abs_path, path);
    rel_path = abs_path;
  }
  /*printk(KERN_INFO "ABS: %s", rel_path);*/

  l = strlen(myPath);
  if(strlen(rel_path) < l)
    return;
  for(i=0;i<l;++i){
    if(myPath[i] != rel_path[i])
      return;
  }
  printk(KERN_INFO "%s\n", (rel_path+l));
 
}

int init_module(void){
  custom_sys_open_callback_function = &redefined_func;
  return 0;
}

void cleanup_module(void){
  custom_sys_open_callback_function = NULL;
}
