include ./make/arch.mk

targets := main
midobjs	:= ./*.o ./*.d ./src/*.o ./src/*.d ./test/*.o ./test/*.d ./product/src/*.o ./product/src/*.d

include ./make/rules.mk



# test
#$(eval $(call LinkApp, $(targets), ./test/test.o ./src/list.o ./src/mutex.o ./src/cond.o ./src/lockqueue.o ./src/ayla/log.o ./src/ayla/lookup_by_name.o))
$(eval $(call LinkApp, $(targets), ./test/logtest.o ./src/ayla/log.o ./src/ayla/lookup_by_name.o))

# app
#$(eval $(call LinkApp, $(targets), ./test/test.o ./src/list.o ./src/mutex.o ./src/cond.o ./src/lockqueue.o))

scp :
	scp -P 22 ./main root@192.168.10.101:/tmp

