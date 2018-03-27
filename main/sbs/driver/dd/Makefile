


obj-m := btp.o

btp-objs :=	bt_bit.o bt_delay.o bt_diag.o bt_event.o bt_icbr_thread.o \
	bt_isr.o bt_llist.o bt_mem.o bt_mutex.o bt_nbus.o bt_param.o \
	bt_rwlck.o bt_setup.o bt_xfer.o btp_bind.o btp_cfg.o btp_ioctl.o \
	 btp_lowio.o btp_mmap.o btp_open.o btp_rdwr.o btp_version.o

EXTRA_CFLAGS += -g  -DBT1003 -I$(BTDRIVER)/dd -I$(BTDRIVER)/include \
	-fno-strength-reduce -Wall -Wstrict-prototypes -fno-common

