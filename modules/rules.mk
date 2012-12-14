include modules/arch/$(ARCH)/rules.mk

MODULES_ARP=modules/arp/arp.ko
MODULES_BASE=modules/base/base.ko
MODULES_BLOCK=modules/block/block.ko
MODULES_SYSTEM=modules/system/system.ko
MODULES_EXT2=modules/ext2/ext2.ko
MODULES_NET=modules/net/net.ko
MODULES_NODEFS=modules/nodefs/nodefs.ko
MODULES_IPV4=modules/ipv4/ipv4.ko
MODULES_VIDEO=modules/video/video.ko

MODULES_OBJECTS_ARP=modules/arp/main.o modules/arp/protocol.o
MODULES_OBJECTS_BASE=modules/base/main.o
MODULES_OBJECTS_BLOCK=modules/block/main.o modules/block/interface.o
MODULES_OBJECTS_SYSTEM=modules/system/main.o modules/system/filesystem.o
MODULES_OBJECTS_EXT2=modules/ext2/main.o modules/ext2/protocol.o modules/ext2/filesystem.o
MODULES_OBJECTS_NET=modules/net/main.o modules/net/interface.o
MODULES_OBJECTS_NODEFS=modules/nodefs/main.o modules/nodefs/filesystem.o
MODULES_OBJECTS_IPV4=modules/ipv4/main.o modules/ipv4/protocol.o
MODULES_OBJECTS_VIDEO=modules/video/main.o modules/video/filesystem.o modules/video/interface.o

MODULES+=$(MODULES_ARP) $(MODULES_BASE) $(MODULES_BLOCK) $(MODULES_SYSTEM) $(MODULES_EXT2) $(MODULES_NET) $(MODULES_NODEFS) $(MODULES_IPV4) $(MODULES_VIDEO)
MODULES_OBJECTS+=$(MODULES_OBJECTS_ARP) $(MODULES_OBJECTS_BASE) $(MODULES_OBJECTS_BLOCK) $(MODULES_OBJECTS_SYSTEM) $(MODULES_OBJECTS_EXT2) $(MODULES_OBJECTS_NET) $(MODULES_OBJECTS_NODEFS) $(MODULES_OBJECTS_IPV4) $(MODULES_OBJECTS_VIDEO)

$(MODULES): LDFLAGS+=-Tmodules/linker.ld -r

$(MODULES_ARP): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_ARP)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BASE): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_BASE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BLOCK): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_BLOCK)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_SYSTEM): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_SYSTEM)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_EXT2): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_EXT2)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NET): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_NET)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NODEFS): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_NODEFS)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_IPV4): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_IPV4)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_VIDEO): libs/fudge/memory.o libs/fudge/string.o $(MODULES_OBJECTS_VIDEO)
	$(LD) $(LDFLAGS) -o $@ $^

modules/%.o: modules/%.c
	$(CC) -c $(CFLAGS) -Ilibs -Ikernel -Imodules -o $@ $<
