#include<stdio.h>
#include<stdlib.h>
#include<sys/fcntl.h>

main()
{
   char buf[10];
   int fd = open("/dev/demo",O_RDWR);
   if(fd < 0){
       perror("open");
       exit(-1);
   }
   write(fd, "Xcvb\0", 10);
   ssize_t size = read(fd, buf, 10);
   printf("Read byte: %s\n", buf);
   sleep(10);
   close(fd);
}
