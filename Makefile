include ./make/arch.mk

targets := main
midobjs	:= ./*.o ./*.d ./src/*.o ./src/*.d ./test/*.o ./product/src/*.o ./product/src/*.d

include ./make/rules.mk

$(eval $(call LinkApp, $(targets), ./test/test.o ./src/list.o ./src/mutex.o ./src/cond.o ./src/lockqueue.o ./product/src/serial.o ./product/src/conhandle.o))

scp :
	scp -P 22 ./main root@192.168.10.101:/tmp

