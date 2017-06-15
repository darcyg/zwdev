#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "flash.h"
#include <string.h>
#include "log.h"
#include "file_io.h"
#include "common.h"

static char *inventory_dir = "/etc/config/dusun/zwave";

int flash_init(const char *base) {
	if (base != NULL) {
		inventory_dir = base;
	}
	
	log_debug("flash module init at : %s", inventory_dir);
  if (!file_is_dir(inventory_dir)) {
    file_create_dir(inventory_dir, 755);
  }
	return 0;
}

int flash_load_attr(int did, const char *attr, char *value) {
  char f[256];

  sprintf(f, "%s/%d/%s", inventory_dir, did, attr);
  FILE *fp = fopen(f, "r");
  if (fp != NULL) {
    char buf[64+1];
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    size = size > sizeof(buf) ? sizeof(buf) : size;
    int ret = fread(buf, size, 1, fp);
    buf[ret*size] = 0;
    fclose(fp);

    strcpy(value, buf);
    log_debug("load attr %s --> value is %s", attr, buf);
		return 0;
  }
  *value = 0;
  return -1;
}
int flash_save_attr(int did, const char *attr, char *value) {
  char f[256];
  sprintf(f, "%s/%d", inventory_dir, did);
  if (!file_is_dir(f)) {
    file_create_dir(f, 755);
  }

  strcat(f, "/");
  strcat(f, attr);
  FILE *fp = fopen(f, "w");
  if (fp != NULL) {
    char buf[32];
    sprintf(buf, "%s", value);
    fwrite(buf, strlen(buf), 1, fp);
    fclose(fp);
  }
	return 0;
}

int flash_save_class(int did, char *cls, int cnt) {
  char f[256];
  sprintf(f, "%s/%d", inventory_dir, did);
  if (!file_is_dir(f)) {
    file_create_dir(f, 755);
  }

  strcat(f, "/");
  strcat(f, ":class");
  FILE *fp = fopen(f, "w");
  if (fp != NULL) {
    fwrite(cls, cnt, 1, fp);
    fclose(fp);
  }
	return 0;
}

int flash_load_class(int did, char *cls, int *cnt) {
  char f[256];

  sprintf(f, "%s/%d/%s", inventory_dir, did, ":class");
  FILE *fp = fopen(f, "r");
  if (fp != NULL) {
    char buf[64+1];
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    size = size > sizeof(buf) ? sizeof(buf) : size;
    int ret = fread(buf, size, 1, fp);
    fclose(fp);

    log_debug("load [%02x] - ", did&0xff);
		log_debug_hex(":class:", buf, ret*size);
		memcpy(cls, buf, ret*size);
		*cnt = ret*size;
		return 0;
  }
	*cnt = 0;
	return -1;
}

int flash_save_basic_generic_specific(int did, char b, char g, char s) {
  char f[256];
  sprintf(f, "%s/%d", inventory_dir, did);
  if (!file_is_dir(f)) {
    file_create_dir(f, 755);
  }

  strcat(f, "/");
  strcat(f, ":basic_generic_sepecific");
  FILE *fp = fopen(f, "w");
  if (fp != NULL) {
		char buf[3] = {b, g, s};
    fwrite(buf, sizeof(buf), 1, fp);
    fclose(fp);
  }
	return 0;

}
int flash_load_basic_generic_specific(int did, char *b, char *g, char *s) {
  char f[256];

  sprintf(f, "%s/%d/%s", inventory_dir, did, ":basic_generic_sepecific");
  FILE *fp = fopen(f, "r");
	if (fp == NULL) {	
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (size != 3) {
		fclose(fp);
		return -2;
	}

	char buf[3];
	int ret = fread(buf, size, 1, fp);
	fclose(fp);
	if (ret != 1) {
		return -3;
	}

	*b = buf[0]; *g = buf[1]; *s = buf[2];
	log_debug("load [%02x] basic:%02x, generic:%02x, specific:%02x.", did&0xff, *b, *g, *s);

	return 0;
}


int flash_remove_device(int did) {
  char f[256];
  sprintf(f, "%s/%d", inventory_dir, did);
	if (access(f, F_OK) == 0) {
		log_debug("remove device : %02x, %s", did&0xff, f);
		char buf[128];
		sprintf(buf, "rm -rf %s", f);
		int ret = system(buf);
		ret = ret;
		//unlink(f);
  }
	return 0;
}

