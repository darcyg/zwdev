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
#include "json_parser.h"
#include "classcmd.h"


struct timer_head th = {
	.first = NULL,
};


void timerout_cb(struct timer *t) {
	log_debug("timer out!");
	timer_set(&th, t, 10000);

	json_t *jattrs = json_object();
	if (jattrs != NULL) {
		emClass_t classes[] = {0x01, 0x02, 0x03, 0x04};
		class_cmd_to_attrs(1, classes, sizeof(classes)/sizeof(classes[0]), jattrs);
		char *jattr_str = json_dumps(jattrs, 0);
		if (jattr_str != NULL) {
			log_debug("attrs is [%s]", jattr_str);
			free(jattr_str);
		}
		json_decref(jattrs);
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

	memory_module_init();
	flash_module_init();
	device_module_init();



	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}

	return 0;
}
