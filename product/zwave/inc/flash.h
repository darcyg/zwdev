#ifndef __FLASH_H_
#define __FLASH_H_



int flash_init(const char *base);
int flash_load_device(int did, char **sdev);
int flash_save_device(int did, const char *sdev);
int flash_remove_device(int did);

#endif
