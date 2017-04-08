#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "log.h"
#include "timer.h"
#include "file_event.h"
#include "hashmap.h"
#include "filesystem_monitor.h"
#include "file_io.h"


struct timer_head th = {
	.first = NULL,
};


void timerout_cb(struct timer *t) {
	log_debug("timer out!");
	timer_set(&th, t, 3000);
}


#define MONITOR_FILE "/tmp/monitorred"



void file_monitor_cb(const char *path, uint32_t event_mask, const char *name) {
	if (event_mask & IN_ACCESS) { 
		log_warn("%s/%s was changed : %s", path, name ? name : "null", "access");
	} 
	if (event_mask & IN_MODIFY) {
		log_warn("%s/%s was changed : %s", path, name ? name : "null", "modified");
	}
}


int main(int argc, char *argv[]) {
	log_init(argv[0], LOG_OPT_DEBUG | LOG_OPT_CONSOLE_OUT | LOG_OPT_TIMESTAMPS | LOG_OPT_FUNC_NAMES);

	log_info("init timer...");
	struct timer tr;
	timer_init(&tr, timerout_cb);
	timer_set(&th, &tr, 3000);
	log_info("init timer complete!");


	struct file_event_table fet;
	file_event_init(&fet);

	file_touch(MONITOR_FILE);
	fs_monitor_init(IN_NONBLOCK);
	//fs_monitor_init(0);
	/*
           IN_ACCESS         File was accessed (read) (*).
           IN_ATTRIB         Metadata changed, e.g., permissions, timestamps, extended attributes, link count (since Linux 2.6.25), UID, GID, etc. (*).
           IN_CLOSE_WRITE    File opened for writing was closed (*).
           IN_CLOSE_NOWRITE  File not opened for writing was closed (*).
           IN_CREATE         File/directory created in watched directory (*).
           IN_DELETE         File/directory deleted from watched directory (*).
           IN_DELETE_SELF    Watched file/directory was itself deleted.
           IN_MODIFY         File was modified (*).
           IN_MOVE_SELF      Watched file/directory was itself moved.
           IN_MOVED_FROM     File moved out of watched directory (*).
           IN_MOVED_TO       File moved into watched directory (*).
           IN_OPEN           File was opened (*).
	*/
	fs_monitor_add_watcher(MONITOR_FILE, file_monitor_cb, IN_MODIFY | IN_ACCESS);

	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
		if (fs_monitor_task() < 0) {
			log_warn("file monitor error: %m");
		}
	}

	return 0;
}
