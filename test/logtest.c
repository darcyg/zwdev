#include <stdlib.h>
#include <stdio.h>

#include "log.h"


int main(int argc, char *argv[]) {
	log_init(argv[0], LOG_OPT_DEBUG | LOG_OPT_CONSOLE_OUT | LOG_OPT_TIMESTAMPS | LOG_OPT_FUNC_NAMES);

	while (1) {
		log_debug("hello world");
	}

	return 0;
}
