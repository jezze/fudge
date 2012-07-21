include modules/src/arch/rules.$(ARCH).mk

MODULES+=modules/src/ext2/ext2.ko
MODULES+=modules/src/log/log.ko
MODULES+=modules/src/mbr/mbr.ko
MODULES+=modules/src/nodefs/nodefs.ko
MODULES+=modules/src/tty/tty.ko

MODULESOBJ+=modules/src/ext2/main.o modules/src/ext2/driver.o modules/src/ext2/filesystem.o
MODULESOBJ+=modules/src/log/main.o modules/src/log/driver.o
MODULESOBJ+=modules/src/mbr/main.o modules/src/mbr/driver.o modules/src/mbr/device.o
MODULESOBJ+=modules/src/nodefs/main.o modules/src/nodefs/driver.o
MODULESOBJ+=modules/src/tty/main.o modules/src/tty/driver.o

CLEAN+=$(MODULESOBJ) $(MODULES)

modules/src/log/log.ko: lib/src/memory.o lib/src/string.o modules/src/log/main.o modules/src/log/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/ext2/ext2.ko: lib/src/memory.o lib/src/string.o modules/src/ext2/main.o modules/src/ext2/driver.o modules/src/ext2/filesystem.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/mbr/mbr.ko: lib/src/memory.o lib/src/string.o modules/src/mbr/main.o modules/src/mbr/driver.o modules/src/mbr/device.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/nodefs/nodefs.ko: lib/src/memory.o lib/src/string.o modules/src/nodefs/main.o modules/src/nodefs/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/tty/tty.ko: lib/src/memory.o lib/src/string.o modules/src/tty/main.o modules/src/tty/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/%.ko: CCFLAGS+=-Ilib/include -Ikernel/include -Imodules/include
modules/%.ko: LDFLAGS+=-Tmodules/linker.ld -r
