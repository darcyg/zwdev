include ./make/arch.mk

targets := main testlockqueue testlog testtimer hashmap

objs							:= ./main.o

testobjs					:= ./test/test.o
testobjs					+= ./src/list.o
testobjs					+= ./src/mutex.o
testobjs					+= ./src/cond.o
testobjs					+= ./src/lockqueue.o

testlogobjs				:= ./test/logtest.o
testlogobjs				+= ./src/ayla/log.o
testlogobjs				+= ./src/ayla/lookup_by_name.o

testtimerobjs			:=
testtimerobjs			+= ./test/timertest.o
testtimerobjs			+= ./src/ayla/log.o
testtimerobjs			+= ./src/ayla/lookup_by_name.o
testtimerobjs			+= ./src/ayla/timer.o
testtimerobjs			+= ./src/ayla/time_utils.o
testtimerobjs			+= ./src/ayla/assert.o
testtimerobjs			+= ./src/ayla/file_event.o

hashmapobjs				:= 
hashmapobjs				+= ./test/hashmaptest.o
hashmapobjs				+= ./src/ayla/log.o
hashmapobjs				+= ./src/ayla/lookup_by_name.o
hashmapobjs				+= ./src/ayla/timer.o
hashmapobjs				+= ./src/ayla/time_utils.o
hashmapobjs				+= ./src/ayla/assert.o
hashmapobjs				+= ./src/ayla/file_event.o
hashmapobjs				+= ./src/ayla/hashmap.o





midobjs	:= 
midobjs += $(objs)					$(objs:%.o=%.d) 
midobjs += $(testobjs)			$(testobjs:%.o=%.d) 
midobjs += $(testlogobjs)		$(testlogobjs:%.o=%.d)
midobjs += $(testtimerobjs) $(testtimerobjs:%.o=%.d) 
midobjs += $(hashmapobjs)		$(hashmapobjs:%.o=%.d) 

include ./make/rules.mk

$(eval $(call LinkApp, main, $(objs)))
$(eval $(call LinkApp, testlockqueue, $(testobjs)))
$(eval $(call LinkApp, testlog, $(testlogobjs)))
$(eval $(call LinkApp, testtimer, $(testtimerobjs)))
$(eval $(call LinkApp, hashmap, $(hashmapobjs)))

scp :
	scp -P 22 ./main root@192.168.10.101:/tmp

