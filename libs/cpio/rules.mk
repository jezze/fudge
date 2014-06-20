LIB_$(DIR):=$(LIBS_PATH)/cpio/libcpio.a
OBJ_$(DIR):=$(LIBS_PATH)/cpio/cpio.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
