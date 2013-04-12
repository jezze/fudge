LIB:=libs/arm/libarm.a
OBJ:=libs/arm/arch.o
OBJ+=libs/arm/cpu.o
OBJ+=libs/arm/init.o
OBJ+=libs/arm/kmi.o
OBJ+=libs/arm/reg.o
OBJ+=libs/arm/uart.o

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBARM:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
