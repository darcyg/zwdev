#include "memory.h"
#include "common.h"
#include "log.h"
#include "hashmap.h"
#include <string.h>

#include "jansson.h"
#include "json_parser.h"

static struct hashmap *hmdevs = NULL;
static struct hashmap  hmdevs_bak;
static int init_flag = 0;

static void *hashmap_alloc_key(const void *_key) {
	const char *key = (const char *)_key;
	int len = strlen(key);
	void *newkey = malloc(len+1);
	if (newkey != NULL) {
		strcpy(newkey, key);
	}
	return newkey;
}

static void hashmap_free_key(void * _key) {
	if (_key == NULL) {
		return;
	}
	free(_key);
}


int memory_init(struct hashmap *_hm) {
	if (init_flag != 0) {
		log_warn("[%s] warning line : %d", __func__, __LINE__);
		return 0;
	}
	init_flag = 1;

	if (_hm != NULL) {
		hmdevs = _hm;
	} else {
		hmdevs= &hmdevs_bak;
	}

	hashmap_init(hmdevs, hashmap_hash_string, hashmap_compare_string, 254);
	hashmap_set_key_alloc_funcs(hmdevs, hashmap_alloc_key, hashmap_free_key);
	return 0;
}

json_t *memory_get_dev(int did) {

	char sdid[32];
	sprintf(sdid, "%d", did);
	void *vhm = hashmap_get(hmdevs, sdid);
	if (vhm == NULL) {
		log_debug("[%s] error line : %d, %d", __func__, __LINE__, did);
		return NULL;
	}
	
	return (json_t*)vhm;
}

int memory_set_dev(int did, json_t *jdev) {
	char sdid[32];
	sprintf(sdid, "%d", did);

	void *vhm = hashmap_get(hmdevs, sdid);
	if (vhm == NULL) {
		if (hashmap_put(hmdevs, sdid, (void*)jdev) == NULL) {
			log_debug("[%s] error line : %d, %d", __func__, __LINE__, did);
			return -1;
		}
	}
	return 0;
}

int memory_del_dev(int did) {
	char sdid[32];
	sprintf(sdid, "%d", did);

	void *old = hashmap_remove((struct hashmap*)hmdevs, sdid);

	if (old != NULL) {
		json_decref(old);
		old = NULL;
	}
	return 0;
}


int memory_test() {
	static int cnt = 0;
	
	memory_init(NULL);
	json_t * jdev = memory_get_dev(1);
	if (jdev == NULL) {
		json_t *jdev = json_object();
		json_object_set_new(jdev, "id", json_integer(1));
		memory_set_dev(1, jdev);
	} else {
		int id; json_get_int(jdev, "id", &id);
		log_debug("dev id is %d", id++);

		json_object_del(jdev, "id");
		json_object_set_new(jdev, "id", json_integer(id));
		memory_set_dev(1, jdev);
	}
	
	if (cnt == 3) {
		memory_del_dev(1);
	}

	if (cnt++ <= 5) {
		log_debug("cnt is %d", cnt);
		memory_test();
	}
	return 0;
}
