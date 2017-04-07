include ./make/arch.mk

targets := test
midobjs	:= ./*.o ./*.d ./src/*.o ./src/*.d ./product/src/*.o ./product/src/*.d

include ./make/rules.mk

#$(eval $(call LinkApp, $(targets), main.o ./src/list.o ./src/mutex.o ./src/cond.o ./src/lockqueue.o))
$(eval $(call LinkApp, $(targets), main.o ./src/list.o ./src/mutex.o ./src/cond.o ./src/lockqueue.o ./product/src/serial.o ./product/src/conhandle.o))

scp :
	scp -P 22 ./test root@192.168.10.101:/tmp

