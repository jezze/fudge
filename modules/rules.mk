include modules/src/arch/rules.$(ARCH).mk

MODULE_EXT2=modules/src/ext2/ext2.ko
MODULE_LOG=modules/src/log/log.ko
MODULE_MBR=modules/src/mbr/mbr.ko
MODULE_NODEFS=modules/src/nodefs/nodefs.ko
MODULE_TTY=modules/src/tty/tty.ko

MODULES+=$(MODULE_EXT2) $(MODULE_LOG) $(MODULE_MBR) $(MODULE_NODEFS) $(MODULE_TTY)

OBJECTS_EXT2=modules/src/ext2/main.o modules/src/ext2/driver.o modules/src/ext2/filesystem.o
OBJECTS_LOG=modules/src/log/main.o modules/src/log/driver.o
OBJECTS_MBR=modules/src/mbr/main.o modules/src/mbr/driver.o modules/src/mbr/device.o
OBJECTS_NODEFS=modules/src/nodefs/main.o modules/src/nodefs/driver.o
OBJECTS_TTY=modules/src/tty/main.o modules/src/tty/driver.o

OBJECTS+=$(OBJECTS_EXT2) $(OBJECTS_LOG) $(OBJECTS_MBR) $(OBJECTS_NODEFS) $(OBJECTS_TTY)

CLEAN+=$(MODULES) $(OBJECTS)

modules/src/log/log.ko: lib/src/memory.o lib/src/string.o $(OBJECTS_LOG)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/ext2/ext2.ko: lib/src/memory.o lib/src/string.o $(OBJECTS_EXT2)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/mbr/mbr.ko: lib/src/memory.o lib/src/string.o $(OBJECTS_MBR)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/nodefs/nodefs.ko: lib/src/memory.o lib/src/string.o $(OBJECTS_NODEFS)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/tty/tty.ko: lib/src/memory.o lib/src/string.o $(OBJECTS_TTY)
	$(LD) $(LDFLAGS) -o $@ $^

modules/%.ko: CCFLAGS+=-Ilib/include -Ikernel/include -Imodules/include
modules/%.ko: LDFLAGS+=-Tmodules/linker.ld -r
