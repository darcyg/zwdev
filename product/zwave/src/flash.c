#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "log.h"
#include "file_io.h"
#include "common.h"

#include "flash.h"
#include "jansson.h"
#include "json_parser.h"

static char *inventory_dir = "/etc/config/dusun/zwave";
static int init_flag = 0;

int flash_init(const char *base) {
	if (init_flag != 0) {
		log_debug("[%s] warning line : %d", __func__, __LINE__);
		return 0;
	}

	if (base != NULL) {
		inventory_dir = (char*)base;
	}
	
	log_debug("flash module init at : %s", inventory_dir);
  if (!file_is_dir(inventory_dir)) {
    file_create_dir(inventory_dir, 755);
  }
	return 0;
}

json_t *flash_load_dev(int did) {
  char f[256];
  sprintf(f, "%s/%d", inventory_dir, did);
  FILE *fp = fopen(f, "r");
	if (fp == NULL) {
		log_debug("[%s] error line : %d, %d", __func__, __LINE__, did);
		goto err_1;
	}

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

	char *buf = MALLOC(size+1);
	if (buf == NULL) {
		log_debug("[%s] error line : %d, %d", __func__, __LINE__, did);
		goto err_2;
	}

  int ret = fread(buf, size, 1, fp);
  fclose(fp);
	fp = NULL;
	if (ret != 1) {
		log_debug("[%s] error line : %d, %d", __func__, __LINE__, did);
		goto err_3;
	}

  buf[ret*size] = 0;


	//log_debug("[%s] success : %d, %d : %s" , __func__, __LINE__, did, *sdev);

	json_error_t err;
	json_t *jret = json_loads(buf, 0, &err);
	FREE(buf);
	buf = NULL;
	if (jret == NULL) {
		log_debug("[%s] error line : %d, %d : %s", __func__, __LINE__, did, err.text);
		goto err_4;
	}
	return jret;
err_4:
err_3:
	if (buf != NULL) {
		FREE(buf);
		buf = NULL;
	}
err_2:
err_1:
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	return NULL;
}


int flash_save_dev(int did, json_t *jdev) {
  char f[256];
	int ret = 0;

  sprintf(f, "%s/%d", inventory_dir, did);
	/*
  if (!file_is_dir(f)) {
    file_create_dir(f, 755);
  }
	*/

  FILE *fp = fopen(f, "w");
  if (fp == NULL) {
		log_debug("[%s] error line : %d, %d : %s", __func__, __LINE__, did, f);
		ret = -1;
		goto err_1;
	}

	char *sdev = json_dumps(jdev, 0);
	if (sdev == NULL) {
		log_debug("[%s] error line : %d, %d, %s", __func__, __LINE__, did, sdev);
		ret = -2;	
		goto err_2;
	}

	int len = strlen(sdev);
  ret = fwrite(sdev, len, 1, fp);
  fclose(fp);
	fp = NULL;
	FREE(sdev);
	sdev = NULL;
	if (ret != 1) {
		log_debug("[%s] error line : %d, %d, %s", __func__, __LINE__, did, sdev);
		ret = -3;
		goto err_3;
	}

	//log_debug("[%s] success : %d, %d : %s" , __func__, __LINE__, did, sdev);

	return 0;
err_3:
err_2:
	if (fp != NULL) {
		fclose(fp); 
		fp = NULL;
	}
err_1:
	return ret;
}

int flash_remove_dev(int did) {
  char f[256];
  sprintf(f, "%s/%d", inventory_dir, did);
	if (access(f, F_OK) == 0) {
		char buf[128];
		sprintf(buf, "rm -rf %s", f);
		int ret = system(buf);
		ret = ret;
		//unlink(f);
  }
	log_debug("[%s] success : %d, %d" , __func__, __LINE__, did);
	return 0;
}


int flash_test() {
	flash_init(NULL);
	json_t * jdev = flash_load_dev(1);
	if (jdev == NULL) {
		json_t *jdev = json_object();
		json_object_set_new(jdev, "id", json_integer(1));
		flash_save_dev(1, jdev);
	} else {
		int id; json_get_int(jdev, "id", &id);
		log_debug("dev id is %d", id++);

		json_object_del(jdev, "id");
		json_object_set_new(jdev, "id", json_integer(id));
		flash_save_dev(1, jdev);
	}
	return 0;
}


