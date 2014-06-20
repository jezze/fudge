LIB_$(DIR):=$(LIBS_PATH)/cpio/libcpio.a
OBJ_$(DIR):=$(LIBS_PATH)/cpio/cpio.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
