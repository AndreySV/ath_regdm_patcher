# if KERNELRELEASE is defined,
# make've been invoked from kernel build system
MODULE=ath_regdm_patcher
LOG=test.log
SRC := $(MODULE).c
OBJ := $(SRC:%.c=%.o)
CC  :=/usr/bin/ccache gcc

ifneq ($(KERNELRELEASE),)
	obj-m	:= $(OBJ)

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all: $(SRC) checkpatch
	$(MAKE) C=2 -C $(KERNELDIR) M=$(PWD) modules

kernel_clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

clean: kernel_clean
	rm -rf Module.symvers modules.order

checkpatch: $(SRC)
	for file in $^; do \
		checkpatch.pl --emacs --terse --file $$file; \
	done

modules_install: all
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a

endif
