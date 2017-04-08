include ./make/arch.mk

targets := main testlockqueue testlog testtimer hashmap filemonitorio jsontest

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

filemonitorioobjs		:=
filemonitorioobjs		+= ./test/filemonitorio.o
filemonitorioobjs		+= ./src/ayla/log.o
filemonitorioobjs		+= ./src/ayla/lookup_by_name.o
filemonitorioobjs		+= ./src/ayla/timer.o
filemonitorioobjs		+= ./src/ayla/time_utils.o
filemonitorioobjs		+= ./src/ayla/assert.o
filemonitorioobjs		+= ./src/ayla/file_event.o
filemonitorioobjs		+= ./src/ayla/hashmap.o
filemonitorioobjs		+= ./src/ayla/file_io.o
filemonitorioobjs		+= ./src/ayla/filesystem_monitor.o

testjsonobjs		:=
testjsonobjs		+= ./test/jsontest.o
testjsonobjs		+= ./src/ayla/log.o
testjsonobjs		+= ./src/ayla/lookup_by_name.o
testjsonobjs		+= ./src/ayla/timer.o
testjsonobjs		+= ./src/ayla/time_utils.o
testjsonobjs		+= ./src/ayla/assert.o
testjsonobjs		+= ./src/ayla/file_event.o
testjsonobjs		+= ./src/ayla/json_parser.o

testjsonobjs		+= ./src/ayla/async.o
testjsonobjs		+= ./src/ayla/buffer.o
testjsonobjs		+= ./src/ayla/conf_io.o
testjsonobjs		+= ./src/ayla/conf_rom.o
testjsonobjs		+= ./src/ayla/crc8.o
testjsonobjs		+= ./src/ayla/crc16.o
testjsonobjs		+= ./src/ayla/crc32.o
testjsonobjs		+= ./src/ayla/crypto.o
testjsonobjs		+= ./src/ayla/hex.o
testjsonobjs		+= ./src/ayla/network_utils.o
testjsonobjs		+= ./src/ayla/parse_argv.o
testjsonobjs		+= ./src/platform/crypto.o
testjsonobjs		+= ./src/platform/ota.o
testjsonobjs		+= ./src/platform/system.o
testjsonobjs		+= ./src/platform/conf.o

testjsonobjs		+= ./src/ayla/file_io.o


midobjs	:= 
midobjs += $(objs)					$(objs:%.o=%.d) 
midobjs += $(testobjs)			$(testobjs:%.o=%.d) 
midobjs += $(testlogobjs)		$(testlogobjs:%.o=%.d)
midobjs += $(testtimerobjs) $(testtimerobjs:%.o=%.d) 
midobjs += $(hashmapobjs)		$(hashmapobjs:%.o=%.d) 
midobjs += $(filemonitorioobjs)	$(filemonitorioobjs:%.o=%.d) 
midobjs += $(testjsonobjs)	$(testjsonobjs:%.o=%.d) 

include ./make/rules.mk

$(eval $(call LinkApp, main, $(objs)))
$(eval $(call LinkApp, testlockqueue, $(testobjs)))
$(eval $(call LinkApp, testlog, $(testlogobjs)))
$(eval $(call LinkApp, testtimer, $(testtimerobjs)))
$(eval $(call LinkApp, hashmap, $(hashmapobjs)))
$(eval $(call LinkApp, filemonitorio, $(filemonitorioobjs)))
$(eval $(call LinkApp, jsontest, $(testjsonobjs)))

scp :
	scp -P 22 ./main root@192.168.10.101:/tmp

