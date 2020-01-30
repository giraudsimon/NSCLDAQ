lib_LTLIBRARIES = libbtp.la
libbtp_la_SOURCES =bt_bind.c \
bt_cas.c \
bt_chke.c \
bt_clre.c \
bt_ctrl.c \
bt_hw_diag.c \
bt_icbr.c \
bt_info.c \
bt_init.c \
bt_ior.c \
bt_iow.c \
bt_locking.c \
bt_mmap.c \
bt_name.c \
bt_open.c \
bt_or_io.c \
bt_perr.c \
bt_rdwr.c \
bt_reset.c \
bt_sema.c \
bt_send_irq.c \
bt_serr.c \
bt_stat.c \
bt_string.c \
bt_tas.c



libbtp_la_LDFLAGS = -version-info 1:0:0  $(THREADLD_FLAGS)
libbtp_la_CPPFLAGS = -I. -I@srcdir@/../include \
	-DBT1003 -DNDEBUG -DBT_INLINE \
	-D_POSIX_C_SOURCE -D_POSIX_THREAD_SEMANTICS \
	-D_XOPEN_SOURCE	\
	-O2 -Wall -Wstrict-prototypes $(THREADCXX_FLAGS)   \
	-std=c99

include_HEADERS =  btpiflib.h

