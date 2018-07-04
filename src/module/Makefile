MDANAME := usbflashtrig
MODULE := $(MDANAME).ko
obj-m := $(MDANAME).o
flashtrig-objs := \
		usbflashtrig.o

KREL := $(shell uname -r)
KDIR := /lib/modules/$(KREL)/build

PWD  := $(shell pwd)

all:
	$(MAKE) build

$(MODULE):
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

build:
	make KREL=$(KREL) clean
	make KREL=$(KREL) $(MODULE)

load:	$(MODULE)
	sync; sync
	sudo insmod $(MODULE)

unload:
	sync; sync
	sudo rmmod $(MODULE)

install: $(MODULE)
	sudo install --mode=0644 --owner=root --group=root $(MODULE) /lib/modules/`uname -r`/kernel/drivers/misc
	sudo /sbin/depmod -a
	sudo /sbin/modprobe $(MDANAME)

uninstall:
	sudo /sbin/modprobe -rq $(MDANAME)
	sudo rm -f /lib/modules/`uname -r`/kernel/drivers/misc/$(MODULE)
	sudo /sbin/depmod -a

