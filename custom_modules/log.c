#include <linux/module.h>
#include <linux/kernel.h>

int myInt;
module_param(myInt, int, 0);

int init_module(void){
  if(myInt < 0)
    printk(KERN_INFO "negative\n");
  else if(myInt == 0)
    printk(KERN_INFO "zero\n");
  else
    printk(KERN_INFO "positive\n");
  return 0;
}

void cleanup_module(void){
  printk(KERN_INFO "GoodBye from log module\n");
}
