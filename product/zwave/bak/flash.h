#ifndef __FLASH_H_
#define __FLASH_H_

#include <jansson.h>


int			flash_init(const char *base);
json_t *flash_load_dev(int did);
int			flash_save_dev(int did, json_t *jdev);
int			flash_remove_dev(int did);

int			flash_test();

#endif
