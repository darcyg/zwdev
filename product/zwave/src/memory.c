#include "memory.h"
#include "common.h"
#include "log.h"
#include "hashmap.h"

static struct hashmap *hmattrs = NULL;

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


int memory_init() {
	hmattrs = &(app_get_inventory()->hmattrs);

	hashmap_init(hmattrs, hashmap_hash_string, hashmap_compare_string, 254);
	hashmap_set_key_alloc_funcs(hmattrs, hashmap_alloc_key, hashmap_free_key);
	return 0;
}

int memory_get_attr(int did, const char *attr, char *value) {
	//log_debug("memory_get_attr: %d, %s", did, attr);

	char sdid[32];
	sprintf(sdid, "%d", did);
	void *vhm = hashmap_get(hmattrs, sdid);
	if (vhm == NULL) {
		value[0] = 0;
		return -1;
	}
	
	const char *v = hashmap_get((struct hashmap *)vhm, attr);
	if (v == NULL) {
		value[0] = 0;
		return -2;
	}
	
	strcpy(value, v);

	return 0;
}

int memory_set_attr(int did, const char *attr, char *value) {
	char sdid[32];
	sprintf(sdid, "%d", did);

	//log_debug("memory_set_attr: %d, %s, %s", did, attr, value);

	void *vhm = hashmap_get(hmattrs, sdid);
	if (vhm == NULL) {
		struct hashmap *hm = MALLOC(sizeof(struct hashmap));
		if (hm == NULL) {
			log_debug("out of memory!");
			return -1;
		} 
		hashmap_init(hm, hashmap_hash_string, hashmap_compare_string, 16);
		hashmap_set_key_alloc_funcs(hm, hashmap_alloc_key, hashmap_free_key);
		vhm = hm;

		if (hashmap_put(hmattrs, sdid, vhm) == NULL) {
			log_debug("error : %s %d", __func__, __LINE__);
			return -2;
		}
	}

	char *tmp_value = (char*)MALLOC(strlen(value)+1);
	if (tmp_value == NULL) {
		log_debug("out of memory!");
		return -3;
	}
	strcpy(tmp_value, value);

	void *old = hashmap_remove((struct hashmap*)vhm, attr);
	if (old != NULL) {
		FREE(old);
	}

	if (hashmap_put((struct hashmap*)vhm, attr, tmp_value) == NULL) {
		return -3;
	}

	return 0;
}

