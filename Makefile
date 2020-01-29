CFLAGS=-g -O2 -W -Wall -Werror
LIBNL_LIBS=`pkg-config libnl-genl-3.0 --libs`
LIBNL_CFLAGS=`pkg-config libnl-genl-3.0 --cflags`
PROGS=quota_monitor_raw quota_monitor_libnl

all: $(PROGS)

quota_monitor_raw: quota_monitor_raw.c
	$(CC) -o $@ $(CFLAGS) $^

quota_monitor_libnl: quota_monitor_libnl.c
	$(CC) -o $@ $(CFLAGS) $(LIBNL_CFLAGS) $^ $(LIBNL_LIBS)

clean:
	rm -f $(PROGS) *~
