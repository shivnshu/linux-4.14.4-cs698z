#ifndef __SYSCHAR_H_
#define __SYSCHAR_H_

#define DEV_MAJOR 246

#define IOCTL_GET_READERS _IOR(DEV_MAJOR, 0, unsigned int *)
#define IOCTL_SET_READERS _IOW(DEV_MAJOR, 1, unsigned int)
   
#endif
