quota_monitor: quota_monitor.c
	$(CC) -o $@ -g -O2 -W -Wall -Werror $^

clean:
	rm -f quota_monitor *~
