#include <linux/module.h>

extern int (*custom_do_trap_callback_function)(struct pt_regs *regs);

int redefined_func(struct pt_regs *regs){
  regs->ip += 1;
  return 0;
}

int init_module(void){
  custom_do_trap_callback_function = &redefined_func;
  return 0;
}

void cleanup_module(void){
  custom_do_trap_callback_function = NULL;
}
