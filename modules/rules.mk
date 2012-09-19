include modules/src/arch/rules.$(ARCH).mk

MODULES_BASE=modules/src/base/base.ko
MODULES_BLOCK=modules/src/block/block.ko
MODULES_EXT2=modules/src/ext2/ext2.ko
MODULES_NET=modules/src/net/net.ko
MODULES_NODEFS=modules/src/nodefs/nodefs.ko
MODULES_IPV4=modules/src/ipv4/ipv4.ko
MODULES_TTY=modules/src/tty/tty.ko
MODULES_VIDEO=modules/src/video/video.ko

MODULES_OBJECTS_BASE=modules/src/base/main.o
MODULES_OBJECTS_BLOCK=modules/src/block/main.o modules/src/block/filesystem.o
MODULES_OBJECTS_EXT2=modules/src/ext2/main.o modules/src/ext2/protocol.o modules/src/ext2/filesystem.o
MODULES_OBJECTS_NET=modules/src/net/main.o modules/src/net/filesystem.o
MODULES_OBJECTS_NODEFS=modules/src/nodefs/main.o modules/src/nodefs/filesystem.o
MODULES_OBJECTS_IPV4=modules/src/ipv4/main.o modules/src/ipv4/protocol.o
MODULES_OBJECTS_TTY=modules/src/tty/main.o modules/src/tty/driver.o modules/src/tty/filesystem.o
MODULES_OBJECTS_VIDEO=modules/src/video/main.o modules/src/video/filesystem.o

MODULES+=$(MODULES_BASE) $(MODULES_BLOCK) $(MODULES_EXT2) $(MODULES_NET) $(MODULES_NODEFS) $(MODULES_IPV4) $(MODULES_TTY) $(MODULES_VIDEO)
MODULES_OBJECTS+=$(MODULES_OBJECTS_BASE) $(MODULES_OBJECTS_BLOCK) $(MODULES_OBJECTS_EXT2) $(MODULES_OBJECTS_NET) $(MODULES_OBJECTS_NODEFS) $(MODULES_OBJECTS_IPV4) $(MODULES_OBJECTS_TTY) $(MODULES_OBJECTS_VIDEO)

modules/%.ko: CCFLAGS+=-Ilib/include -Ikernel/include -Imodules/include
modules/%.ko: LDFLAGS+=-Tmodules/linker.ld -r

$(MODULES_BASE): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_BASE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BLOCK): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_BLOCK)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_EXT2): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_EXT2)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NET): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_NET)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NODEFS): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_NODEFS)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_IPV4): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_IPV4)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_TTY): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_TTY)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_VIDEO): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_VIDEO)
	$(LD) $(LDFLAGS) -o $@ $^
