include modules/src/arch/rules.$(ARCH).mk

MODULES_EXT2=modules/src/ext2/ext2.ko
MODULES_LOG=modules/src/log/log.ko
MODULES_MBR=modules/src/mbr/mbr.ko
MODULES_NODEFS=modules/src/nodefs/nodefs.ko
MODULES_TTY=modules/src/tty/tty.ko

MODULES+=$(MODULES_EXT2) $(MODULES_LOG) $(MODULES_MBR) $(MODULES_NODEFS) $(MODULES_TTY)

MODULES_OBJECTS_EXT2=modules/src/ext2/main.o modules/src/ext2/driver.o modules/src/ext2/filesystem.o
MODULES_OBJECTS_LOG=modules/src/log/main.o modules/src/log/driver.o
MODULES_OBJECTS_MBR=modules/src/mbr/main.o modules/src/mbr/driver.o modules/src/mbr/device.o
MODULES_OBJECTS_NODEFS=modules/src/nodefs/main.o modules/src/nodefs/driver.o
MODULES_OBJECTS_TTY=modules/src/tty/main.o modules/src/tty/driver.o

MODULES_OBJECTS+=$(MODULES_OBJECTS_EXT2) $(MODULES_OBJECTS_LOG) $(MODULES_OBJECTS_MBR) $(MODULES_OBJECTS_NODEFS) $(MODULES_OBJECTS_TTY)

modules/src/log/log.ko: lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_LOG)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/ext2/ext2.ko: lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_EXT2)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/mbr/mbr.ko: lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_MBR)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/nodefs/nodefs.ko: lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_NODEFS)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/tty/tty.ko: lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_TTY)
	$(LD) $(LDFLAGS) -o $@ $^

modules/%.ko: CCFLAGS+=-Ilib/include -Ikernel/include -Imodules/include
modules/%.ko: LDFLAGS+=-Tmodules/linker.ld -r
