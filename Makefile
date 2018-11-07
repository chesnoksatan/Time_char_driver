obj-m += set_time.o
TARGET = set_time

CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/kernel/$(MDIR)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c

install: all
	sudo insmod $(TARGET).ko
	sudo cp -v $(TARGET).ko $(DEST) && sudo /sbin/depmod -a
	sudo /sbin/depmod -a
	sudo cp -f 71-set_time.rules /etc/udev/rules.d
	sudo dmesg -c

unload:
	sudo rmmod $(TARGET).ko
	sudo dmesg -c
