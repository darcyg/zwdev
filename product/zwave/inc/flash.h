#ifndef __FLASH_H_
#define __FLASH_H_



int flash_init(const char *base);
int flash_load_attr(int did, const char *attr, char *value);
int flash_save_attr(int did, const char *attr, char *value);


#endif
