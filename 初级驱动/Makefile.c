---------------------------------------------------

## 编译应用程序.c 、.cpp ##

 1. 单个文件编译

```
CC = gcc
CFLAGFS = -Wall -g -O0
OBJS = test
SRC = test.c linkqueue.c
$(OBJS) : $(SRC)
	    $(CC) $(CFLAGFS) -o $@ $^

clean:
	$(RM) $(OBJS) .*.sw?

```

 2. 全部文件编译
```
SRC = ${wildcard *.c}
BIN = ${patsubst %.c, %, $(SRC)}
CFLAGS = -g -Wall

all:$(BIN)

$(BIN):%:%.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(BIN) .*.sw?

.PHONY:all clean

```

---------------------------------------------------
## 编译驱动.ko ##
```
ROOTFS_DIR = /opt/fs210/rootfs/helloc_arm	#复制生成.ko文件到的路径
CROSS_COMPILE=arm-none-linux-gnueabi-
#APP_NAME = key_test
KO_NAME = mybus
KO_NAME2 = mydev
KO_NAME3 = mydrv

ifeq ($(KERNELRELEASE), ) 
KERNEL_DIR =/home/farsight/linux-3.0.8		#内核路径
CUR_DIR = $(shell pwd)
all :
	make -C $(KERNEL_DIR) M=$(CUR_DIR) modules
#	$(CROSS_COMPILE)gcc -g -Wall $(APP_NAME).c -o $(APP_NAME)
clean :
	make -C $(KERNEL_DIR) M=$(CUR_DIR) clean
	rm -rf $(APP_NAME)

install:
	cp -raf *.ko $(APP_NAME)  $(ROOTFS_DIR)/drv_module
	
else
obj-m += $(KO_NAME).o
obj-m += $(KO_NAME2).o
obj-m += $(KO_NAME3).o
endif
```
