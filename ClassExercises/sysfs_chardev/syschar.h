#ifndef __SYSCHAR_H_
#define __SYSCHAR_H_

#define DEV_MAJOR 243

#define IOCTL_GET_READERS _IOR(DEV_MAJOR, 0, unsigned int *)
#define IOCTL_SET_READERS _IOW(DEV_MAJOR, 1, unsigned int)
#define IOCTL_GET_WRITERS _IOR(DEV_MAJOR, 2, unsigned int *)
#define IOCTL_SET_WRITERS _IOW(DEV_MAJOR, 3, unsigned int)
   
#endif
