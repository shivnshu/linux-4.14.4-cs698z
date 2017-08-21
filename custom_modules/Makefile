#obj-m += hello.o
#obj-m += log.o
#obj-m += sys_open_callback.o
obj-m += do_trap_callback.o

all:
	make -C ../ M=$(PWD) modules

clean:
	make -C ../ M=$(PWD) clean
