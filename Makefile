ROOTDIR=$(shell pwd)
WORKDIR=$(ROOTDIR)/build

#targets	 += zwdevd
#targets := testlockqueue 
#targets := testlog 
#targets := testtimer
#targets := hashmap
#targets := filemonitorio
#targets := jsontest
#targets := testserial
#targets := frame
#targets := testsession
#targets := statemachine
targets	:= testclasscmd

.PHONY: targets

all : $(targets)


srcs							:= $(ROOTDIR)/main.c
srcs							+= $(ROOTDIR)/src/ayla/log.c
srcs							+= $(ROOTDIR)/src/ayla/lookup_by_name.c
srcs							+= $(ROOTDIR)/product/zwave/src/serial.c
srcs							+= $(ROOTDIR)/product/zwave/src/transport.c
srcs							+= $(ROOTDIR)/product/zwave/src/frame.c
srcs							+= $(ROOTDIR)/product/zwave/src/session.c
srcs							+= $(ROOTDIR)/product/zwave/src/api.c
srcs							+= $(ROOTDIR)/product/zwave/src/statemachine.c
srcs							+= $(ROOTDIR)/src/ayla/timer.c
srcs							+= $(ROOTDIR)/src/ayla/time_utils.c
srcs							+= $(ROOTDIR)/src/ayla/assert.c
srcs							+= $(ROOTDIR)/src/ayla/file_event.c
srcs							+= $(ROOTDIR)/src/lockqueue.c
srcs							+= $(ROOTDIR)/src/mutex.c
srcs							+= $(ROOTDIR)/src/cond.c
srcs							+= $(ROOTDIR)/src/list.c

testsrcs					:= $(ROOTDIR)/test/test.c
testsrcs					+= $(ROOTDIR)/src/list.c
testsrcs					+= $(ROOTDIR)/src/mutex.c
testsrcs					+= $(ROOTDIR)/src/cond.c
testsrcs					+= $(ROOTDIR)/src/lockqueue.c

testlogsrcs				:= $(ROOTDIR)/test/logtest.c
testlogsrcs				+= $(ROOTDIR)/src/ayla/log.c
testlogsrcs				+= $(ROOTDIR)/src/ayla/lookup_by_name.c

testtimersrcs			:=
testtimersrcs			+= $(ROOTDIR)/test/timertest.c
testtimersrcs			+= $(ROOTDIR)/src/ayla/log.c
testtimersrcs			+= $(ROOTDIR)/src/ayla/lookup_by_name.c
testtimersrcs			+= $(ROOTDIR)/src/ayla/timer.c
testtimersrcs			+= $(ROOTDIR)/src/ayla/time_utils.c
testtimersrcs			+= $(ROOTDIR)/src/ayla/assert.c
testtimersrcs			+= $(ROOTDIR)/src/ayla/file_event.c

hashmapsrcs				:= 
hashmapsrcs				+= $(ROOTDIR)/test/hashmaptest.c
hashmapsrcs				+= $(ROOTDIR)/src/ayla/log.c
hashmapsrcs				+= $(ROOTDIR)/src/ayla/lookup_by_name.c
hashmapsrcs				+= $(ROOTDIR)/src/ayla/timer.c
hashmapsrcs				+= $(ROOTDIR)/src/ayla/time_utils.c
hashmapsrcs				+= $(ROOTDIR)/src/ayla/assert.c
hashmapsrcs				+= $(ROOTDIR)/src/ayla/file_event.c
hashmapsrcs				+= $(ROOTDIR)/src/ayla/hashmap.c

filemonitoriosrcs		:=
filemonitoriosrcs		+= $(ROOTDIR)/test/filemonitorio.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/log.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/lookup_by_name.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/timer.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/time_utils.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/assert.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/file_event.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/hashmap.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/file_io.c
filemonitoriosrcs		+= $(ROOTDIR)/src/ayla/filesystem_monitor.c

testjsonsrcs		:=
testjsonsrcs		+= $(ROOTDIR)/test/jsontest.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/log.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/lookup_by_name.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/timer.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/time_utils.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/assert.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/file_event.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/json_parser.c

testjsonsrcs		+= $(ROOTDIR)/src/ayla/async.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/buffer.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/conf_io.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/conf_rom.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/crc8.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/crc16.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/crc32.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/crypto.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/hex.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/network_utils.c
testjsonsrcs		+= $(ROOTDIR)/src/ayla/parse_argv.c
testjsonsrcs		+= $(ROOTDIR)/src/platform/crypto.c
testjsonsrcs		+= $(ROOTDIR)/src/platform/ota.c
testjsonsrcs		+= $(ROOTDIR)/src/platform/system.c
testjsonsrcs		+= $(ROOTDIR)/src/platform/conf.c

testjsonsrcs		+= $(ROOTDIR)/src/ayla/file_io.c

testserialsrcs	:= $(ROOTDIR)/test/testserial.c
testserialsrcs	+= $(ROOTDIR)/src/ayla/log.c
testserialsrcs	+= $(ROOTDIR)/src/ayla/lookup_by_name.c
testserialsrcs	+= $(ROOTDIR)/product/zwave/src/serial.c
testserialsrcs	+= $(ROOTDIR)/product/zwave/src/transport.c

framesrcs							:= $(ROOTDIR)/test/testframe.c
framesrcs							+= $(ROOTDIR)/src/ayla/log.c
framesrcs							+= $(ROOTDIR)/src/ayla/lookup_by_name.c
framesrcs							+= $(ROOTDIR)/product/zwave/src/serial.c
framesrcs							+= $(ROOTDIR)/product/zwave/src/transport.c
framesrcs							+= $(ROOTDIR)/product/zwave/src/frame.c
framesrcs							+= $(ROOTDIR)/src/ayla/timer.c
framesrcs							+= $(ROOTDIR)/src/ayla/time_utils.c
framesrcs							+= $(ROOTDIR)/src/ayla/assert.c
framesrcs							+= $(ROOTDIR)/src/ayla/file_event.c


testsessionsrcs							:= $(ROOTDIR)/test/testsession.c
testsessionsrcs							+= $(ROOTDIR)/src/ayla/log.c
testsessionsrcs							+= $(ROOTDIR)/src/ayla/lookup_by_name.c
testsessionsrcs							+= $(ROOTDIR)/product/zwave/src/serial.c
testsessionsrcs							+= $(ROOTDIR)/product/zwave/src/transport.c
testsessionsrcs							+= $(ROOTDIR)/product/zwave/src/frame.c
testsessionsrcs							+= $(ROOTDIR)/product/zwave/src/session.c
testsessionsrcs							+= $(ROOTDIR)/src/ayla/timer.c
testsessionsrcs							+= $(ROOTDIR)/src/ayla/time_utils.c
testsessionsrcs							+= $(ROOTDIR)/src/ayla/assert.c
testsessionsrcs							+= $(ROOTDIR)/src/ayla/file_event.c
testsessionsrcs							+= $(ROOTDIR)/src/lockqueue.c
testsessionsrcs							+= $(ROOTDIR)/src/mutex.c
testsessionsrcs							+= $(ROOTDIR)/src/cond.c
testsessionsrcs							+= $(ROOTDIR)/src/list.c

statemachinesrcs							:= $(ROOTDIR)/test/statemachine.c
statemachinesrcs							+= $(ROOTDIR)/src/ayla/log.c
statemachinesrcs							+= $(ROOTDIR)/src/ayla/lookup_by_name.c
statemachinesrcs							+= $(ROOTDIR)/product/zwave/src/statemachine.c
statemachinesrcs							+= $(ROOTDIR)/src/ayla/timer.c
statemachinesrcs							+= $(ROOTDIR)/src/ayla/time_utils.c
statemachinesrcs							+= $(ROOTDIR)/src/ayla/assert.c
statemachinesrcs							+= $(ROOTDIR)/src/ayla/file_event.c
statemachinesrcs							+= $(ROOTDIR)/src/lockqueue.c
statemachinesrcs							+= $(ROOTDIR)/src/mutex.c
statemachinesrcs							+= $(ROOTDIR)/src/cond.c
statemachinesrcs							+= $(ROOTDIR)/src/list.c


testclasscmdsrcs							:= $(ROOTDIR)/test/testclasscmd.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/log.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/lookup_by_name.c
testclasscmdsrcs							+= $(ROOTDIR)/product/zwave/src/classcmd.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/timer.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/time_utils.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/assert.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/file_event.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/hashmap.c
testclasscmdsrcs							+= $(ROOTDIR)/src/ayla/file_io.c
testclasscmdsrcs							+= $(ROOTDIR)/src/lockqueue.c
testclasscmdsrcs							+= $(ROOTDIR)/src/mutex.c
testclasscmdsrcs							+= $(ROOTDIR)/src/cond.c
testclasscmdsrcs							+= $(ROOTDIR)/src/list.c




objs = $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(srcs)))
testserialobjs = $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(testserialsrcs)))
statemachineobjs = $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(statemachinesrcs)))
testclasscmdobjs = $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(testclasscmdsrcs)))

-include $(ROOTDIR)/make/arch.mk
-include $(ROOTDIR)/make/rules.mk

$(eval $(call LinkApp,zwdevd,$(objs)))
$(eval $(call LinkApp,testlockqueue,$(testobjs)))
$(eval $(call LinkApp,testlog,$(testlogobjs)))
$(eval $(call LinkApp,testtimer,$(testtimerobjs)))
$(eval $(call LinkApp,hashmap,$(hashmapobjs)))
$(eval $(call LinkApp,filemonitorio,$(filemonitorioobjs)))
$(eval $(call LinkApp,jsontest,$(testjsonobjs)))
$(eval $(call LinkApp,testserial,$(testserialobjs)))
$(eval $(call LinkApp,frame,$(frameobjs)))
$(eval $(call LinkApp,testsession,$(testsessionobjs)))
$(eval $(call LinkApp,statemachine,$(statemachineobjs)))
$(eval $(call LinkApp,testclasscmd,$(testclasscmdobjs)))


scp :
	scp -P 22 $(ROOTDIR)/main root@192.168.10.101:/tmp

startvc :
	(cd $(ROOTDIR)/utils/VirtualCom;\
		$(ROOTDIR)/StartVirtualComServer.sh start; \
		cd -; \
	)
stopvc :
	(cd $(ROOTDIR)/utils/VirtualCom;\
		$(ROOTDIR)/StartVirtualComServer.sh stop; \
		cd -;\
	)

