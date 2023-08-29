ifeq ($(TARGET),i386-tcc)
O:=\
    $(O) \
    $(DIR_LIB)/abi/$(ARCH)/crt0.o \

L:=\
    $(L) \
    $(DIR_LIB)/tcc/tcc.a \

endif
$(B): $(O) $(L)
	@echo LD $@
	@$(LD) $(LD_FLAGS_BIN) -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
