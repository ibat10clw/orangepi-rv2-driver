obj-m += k1-crng.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	dtc -@ -I dts -O dtb -o k1-crng.dtbo k1-crng.dtso
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
