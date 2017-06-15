#include "memory.h"
#include "common.h"
#include "log.h"
#include "hashmap.h"
#include <string.h>
#include "app.h"

static struct hashmap *hmdevs = NULL;
static struct hashmap  hmdevs_bak;

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
		if (hashmap_put(hmdevs, sdid, vhm) == NULL) {
			log_debug("[%s] error line : %d, %d", __func__, __LINE__, did);
			return -1;
		}
	}
	return 0;
}

int memory_del_device(int did) {
	char sdid[32];
	sprintf(sdid, "%d", did);

	void *old = hashmap_remove((struct hashmap*)hmdevs, sdid);

	if (old != NULL) {
		json_decref(old);
		old = NULL;
	}
	return 0;
}
