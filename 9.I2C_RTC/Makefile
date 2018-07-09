obj-m := i2c_rtc.o

ARCH = arm

CROSS_COMPILE = arm-linux-

KDIR = /usr/ksrc/linux-4.12

PWD := $(shell pwd)

default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	@rm -rf a.out
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

# Generic Makefile should not contain preset values for CROSS_COMPILE and ARCH
# The syntax to invoke make command to build for any architecture using any cross toolchain \
  should be specified from command line by user:                                            \
  Example : For ARM architecture and using arm-linux- cross toolchain :                     \
            $make ARCH=arm CROSS_COMPILE=arm-linux-

# Since here we use Beaglebone Black which is ARM architecture and our cross toolchain is   \
  created using buildroot called arm-linux- so we are presetting them in the Makefile.      \
  This saves time as now we only need to call $make                                         \
  However it builds for ARM by default and any unaware user may not realize this issue and  \
  think that it generated code for x86 system by default.


# Though we have preset values for ARCH and CROSS_COMPILE, we can specify them from command \
  line to override the preset values.

