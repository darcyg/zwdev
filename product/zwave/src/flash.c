#include <stdio.h>
#include <stdlib.h>
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

    log_debug("load string :%s", buf);
    strcpy(value, buf);
    log_debug("load value is %s", buf);
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


