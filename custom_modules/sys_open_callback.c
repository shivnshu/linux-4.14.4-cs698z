#include <linux/module.h>
#include <linux/kernel.h>

extern void (*custom_sys_open_callback_function)(const char *path);

char *myPath;
module_param(myPath, charp, 0);

void redefined_func(const char *path){
  /*printk(KERN_INFO "%s %s\n", path, myPath);*/
  int i;
  int l = strlen(myPath);
  if(strlen(path) < l)
    return;
  for(i=0;i<l;++i){
    if(myPath[i] != path[i])
      return;
  }
  printk(KERN_INFO "%s\n", (path+l));
}

int init_module(void){
  custom_sys_open_callback_function = &redefined_func;
  return 0;
}

void cleanup_module(void){
  custom_sys_open_callback_function = NULL;
}
