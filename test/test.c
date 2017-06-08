#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "mutex.h"
#include "cond.h"
#include "lockqueue.h"

stList_t l;
stMutex_t lock;
stCond_t cnd;
stLockQueue_t queue;

void *wait_thread(void *);

int main(int argc, char *argv[]) {


	list_init(&l);
	mutex_init(&lock);
	cond_init(&cnd);
	lockqueue_init(&queue);

	pthread_t pid;
	pthread_create(&pid, NULL, wait_thread, (void*)&cnd);

	int value = 0;
	while (1) {
		int i;
		for (i = 0; i < 100; i++) {
			mutex_lock(&lock);
			list_push_back(&l, (void*)(rand()%1000));
			mutex_unlock(&lock);
		}

		while (!list_is_empty(&l)) {
			int x;
			mutex_lock(&lock);
			if (list_pop_back(&l, (void **)&x)) {
			}
			mutex_unlock(&lock);
		}


		lockqueue_push(&queue, (void *)(value++));
		cond_wake(&cnd);

	}

	list_destroy(&l, NULL);

	return 0;
}


void *wait_thread(void *arg) {
	stCond_t *cnd = (stCond_t*)arg;
	while (true) {
		cond_wait(cnd);
		while (!lockqueue_empty(&queue)) {
			int value;
			if (lockqueue_pop(&queue, (void **)&value)) {
				DEBUG_PRINT("value is %d\n", value);
			}
		}
	}
}
