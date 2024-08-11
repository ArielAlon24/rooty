SRC_DIR = src

obj-m += $(SRC_DIR)/rooty.o

all: clean build insert

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insert:
	sudo insmod src/rooty.ko

remove:
	sudo rmmod rooty
