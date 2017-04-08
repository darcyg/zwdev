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


struct timer_head th = {
	.first = NULL,
};


void timerout_cb(struct timer *t) {
	log_debug("timer out!");
	timer_set(&th, t, 3000);

	const char *jsonstr = "{\"name\":\"kevne\", \"value\":\"abcdef\", \"key\":35}";
	json_error_t error;
	json_t *jpkt = json_loads(jsonstr, 0, &error);
	if (jpkt == NULL) {
		log_debug("error: on line : %d:%s", error.line, error.text);
		return;
	}

	const char *name = json_get_string(jpkt, "name");
	const char *value = json_get_string(jpkt, "value");
	int key = -1; json_get_int(jpkt, "key", &key);
	if (name == NULL || value == NULL || key == -1) {
		json_decref(jpkt);
		log_debug("error name, value or key item");
		return;
	}
	
	log_info("name:%s, value:%s, key:%d", name, value, key);
	json_decref(jpkt);
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

	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}

	return 0;
}
