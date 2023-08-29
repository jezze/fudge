ifeq ($(TARGET),i386-tcc)
L:=\
    $(L) \
    $(DIR_LIB)/tcc/tcc.a \

endif
$(B): $(O) $(L)
	@echo LD $@
	@$(LD) $(LD_FLAGS_KBIN) -o $@ $^

KBIN:=$(KBIN) $(B)
KMAP:=$(KMAP) $(N)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
