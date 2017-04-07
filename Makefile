include ./make/arch.mk

targets := main testlockqueue testlog testtimer

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

midobjs	:= 
midobjs += $(objs)					$(objs:%.o=%.d) 
midobjs += $(testobjs)			$(testobjs:%.o=%.d) 
midobjs += $(testlogobjs)		$(testlogobjs:%.o=%.d)
midobjs += $(testtimerobjs) $(testtimerobjs:%.o=%.d) 

include ./make/rules.mk

$(eval $(call LinkApp, main, $(objs)))
$(eval $(call LinkApp, testlockqueue, $(testobjs)))
$(eval $(call LinkApp, testlog, $(testlogobjs)))
$(eval $(call LinkApp, testtimer, $(testtimerobjs)))

scp :
	scp -P 22 ./main root@192.168.10.101:/tmp

