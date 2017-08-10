#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/path.h>
#include <linux/fs_struct.h>
#include <linux/dcache.h>
#include <linux/slab.h>

extern void (*custom_sys_open_callback_function)(const char *path);

char *myPath;
module_param(myPath, charp, 0);

void redefined_func(const char *path){
  /*printk(KERN_INFO "%s %s\n", path, myPath);*/
 char abs_path[1000] = {0};
 char rel_path[1000];
 char *dir_names[100];
 char d_slash[] = {'/', '\0'};
 int i=0, j, k, l;
 if(path[0] == '/'){
    strcpy(rel_path, path);
  }
  else {
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
    strcpy(rel_path, abs_path);
  }

  char tmp_names[100][100];
  l = strlen(rel_path);
  j = 0;
  k = 0;
  int index;
  for(i=1;i<l;++i){
   if(rel_path[i] == '/'){
    index = 0;
    while(j<i-1){
     tmp_names[k][index] = rel_path[j+1]; 
     index++;
     j++;
    }
    tmp_names[k][index] = '\0';
    k++;
    j = i;
   }
  }
  if(j<l){
    strcpy(tmp_names[k], (rel_path+j+1));
    k++;
  }

  for(i=0;i<k;++i){
    if(strlen(tmp_names[i]) == 1 && tmp_names[i][0] == '.')
      tmp_names[i][0] = '\0';
    else if(strlen(tmp_names[i]) == 2 && tmp_names[i][0] == '.' && tmp_names[i][1] == '.'){
      tmp_names[i][0] = '\0';
      j = i-1;
      while(j>=0 && tmp_names[j][0] == '\0')
        j--;
      if(j<0)
        continue;
      tmp_names[j][0] = '\0';

    }
  }

  rel_path[0] = '\0';
  for(i=0;i<k;++i){
    if(strlen(tmp_names[i]) > 0){
      strcat(rel_path, d_slash);
      strcat(rel_path, tmp_names[i]);
      }
  }

  l = strlen(myPath);
  if(strlen(rel_path) < l)
    return;
  for(i=0;i<l;++i){
    if(myPath[i] != rel_path[i])
      return;
  }
  if(myPath[l-1] != '/')
    l++;
  printk(KERN_INFO "%s\n", (rel_path+l));

}

int init_module(void){
  custom_sys_open_callback_function = &redefined_func;
  return 0;
}

void cleanup_module(void){
  custom_sys_open_callback_function = NULL;
}
