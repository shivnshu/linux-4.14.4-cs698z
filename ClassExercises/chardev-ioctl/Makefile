obj-m = syschar.o

KVERSION = $(shell uname -r)

all:
	make -C ../../ M=$(PWD) modules

clean:
	make -C ../../ M=$(PWD) clean

