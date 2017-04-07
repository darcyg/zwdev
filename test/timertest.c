#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "timer.h"
#include "file_event.h"


struct timer_head th = {
	.first = NULL,
};

void timerout_cb(struct timer *t) {
	log_debug("timer out!");
	timer_set(&th, t, 300);
}



int main(int argc, char *argv[]) {
	log_init(argv[0], LOG_OPT_DEBUG | LOG_OPT_CONSOLE_OUT | LOG_OPT_TIMESTAMPS | LOG_OPT_FUNC_NAMES);

	log_debug("init timer...");
	struct timer tr;
	timer_init(&tr, timerout_cb);
	timer_set(&th, &tr, 3000);
	log_debug("init timer complete!");
	log_warn("init timer complete!");
	log_info("init timer complete!");
	log_err("init timer complete!");


	struct file_event_table fet;
	file_event_init(&fet);

	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}

	return 0;
}
