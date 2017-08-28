#include<stdio.h>
#include<stdlib.h>
#include<sys/fcntl.h>
#include<sys/ioctl.h>
#include "syschar.h"

main()
{
   int fd = open("/dev/demo",O_RDWR);
   unsigned readers;
   if(fd < 0){
       perror("open");
       exit(-1);
   }

   if(ioctl(fd, IOCTL_GET_READERS, &readers) < 0) {
         perror("ioctl");
   }
   printf("readers=%d\n", readers);
   if(ioctl(fd, IOCTL_SET_READERS, 10) < 0) {
         perror("ioctl");
   }
   if(ioctl(fd, IOCTL_GET_READERS, &readers) < 0) {
         perror("ioctl");
   }
   printf("readers=%d\n", readers);
   close(fd);
}
