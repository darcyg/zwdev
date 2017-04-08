#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "log.h"
#include "timer.h"
#include "file_event.h"
#include "hashmap.h"


struct timer_head th = {
	.first = NULL,
};

struct hashmap hm;

int print_hashmap_item(const void *_key,void *_data, void *_arg) {
	const char *key = _key;
	int value = (int)_data;
	
	log_info("[%s] - [%d]", key, value);
	return 0;
}

void *hashmap_alloc_key(const void *_key) {
	const char *key = (const char *)_key;
	int len = strlen(key);
	void *newkey = malloc(len+1);
	if (newkey != NULL) {
		strcpy(newkey, key);
	}
	return newkey;
}
void hashmap_free_key(void * _key) {
	if (_key == NULL) {
		return;
	}
	free(_key);
}

void timerout_cb(struct timer *t) {
	static int index = 0;

	log_debug("timer out!");
	timer_set(&th, t, 3000);

	
	char keystr[32];
	sprintf(keystr, "%d", index);
	hashmap_put(&hm, keystr, (void*)index);
	index++;

	hashmap_foreach(&hm, print_hashmap_item, NULL);
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

	hashmap_init(&hm, hashmap_hash_string, hashmap_compare_string, 128);
	hashmap_set_key_alloc_funcs(&hm, hashmap_alloc_key, hashmap_free_key);

	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}

	return 0;
}
