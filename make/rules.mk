subdirs		+=
components	+= 
cpdirs		+=
targets		+= 
midobjs		+=

all	: $(subdirs) $(components) $(cpdirs) $(targets) 

%$(OEXT): %$(CPPEXT)
	$(CPP) $(COMPILE) $< $(CPPFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" $(OUT) $@

%$(OEXT): %$(CEXT)
	$(CC) $(COMPILE) $< $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" $(OUT) $@

%$(SABEXT): %$(SAXEXT)
	$(SEDONACC) $< $(SEDONAOUT) .

%$(SCODEEXT): %$(XMLEXT)
	$(SEDONACC) $< $(SEDONAOUT) .
	
$(subdirs):
	make -C $@

$(components):
	$(SEDONACC) ./$@/kit.xml $(SEDONAOUT) .
	$(SEDONACC) ./$@/kit.xml

$(cpdirs):
	$(CP) $(BASE_DIR)/scode/src/$@ $(SEDONA_HOME)/src/my-$@ $(CPFLAGS)

clean:
	$(foreach dir,$(subdirs),make -C $(dir) clean;)
	$(foreach dir,$(cpdirs),$(RM) $(RMFLAGS) $(SEDONA_HOME)/src/my-$(dir);)
	$(RM) $(RMFLAGS) $(targets)
	$(RM) $(RMFLAGS) $(midobjs)
	
define LinkApp
$1: $2
	$(LD) $2 $(LDFLAGS) $(OUT) $1
	#$(STRIP) $1
endef

define Stage
$1:
	$(SEDONACC) $2 $(SEDONAOUT) $1
endef


