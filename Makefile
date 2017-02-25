obj-m := MAIpipe.o
KDIR := /usr/src/linux

all:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	rm -rf built-in.o MAIpipe.ko MAIpipe.mod.* MAIpipe.o Module.symvers modules.order .MAIpipe.ko.cmd .MAIpipe.mod.o.cmd .MAIpipe.o.cmd .built-in.o.cmd .tmp_versions/

