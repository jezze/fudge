MODULES_ARP=modules/src/arp/arp.ko
MODULES_BASE=modules/src/base/base.ko
MODULES_BLOCK=modules/src/block/block.ko
MODULES_SYSTEM=modules/src/system/system.ko
MODULES_EXT2=modules/src/ext2/ext2.ko
MODULES_NET=modules/src/net/net.ko
MODULES_NODEFS=modules/src/nodefs/nodefs.ko
MODULES_IPV4=modules/src/ipv4/ipv4.ko
MODULES_VIDEO=modules/src/video/video.ko

MODULES_OBJECTS_ARP=modules/src/arp/main.o modules/src/arp/protocol.o
MODULES_OBJECTS_BASE=modules/src/base/main.o
MODULES_OBJECTS_BLOCK=modules/src/block/main.o modules/src/block/interface.o
MODULES_OBJECTS_SYSTEM=modules/src/system/main.o modules/src/system/filesystem.o
MODULES_OBJECTS_EXT2=modules/src/ext2/main.o modules/src/ext2/protocol.o modules/src/ext2/filesystem.o
MODULES_OBJECTS_NET=modules/src/net/main.o modules/src/net/interface.o
MODULES_OBJECTS_NODEFS=modules/src/nodefs/main.o modules/src/nodefs/filesystem.o
MODULES_OBJECTS_IPV4=modules/src/ipv4/main.o modules/src/ipv4/protocol.o
MODULES_OBJECTS_VIDEO=modules/src/video/main.o modules/src/video/filesystem.o modules/src/video/interface.o

MODULES+=$(MODULES_ARP) $(MODULES_BASE) $(MODULES_BLOCK) $(MODULES_SYSTEM) $(MODULES_EXT2) $(MODULES_NET) $(MODULES_NODEFS) $(MODULES_IPV4) $(MODULES_VIDEO)
MODULES_OBJECTS+=$(MODULES_OBJECTS_ARP) $(MODULES_OBJECTS_BASE) $(MODULES_OBJECTS_BLOCK) $(MODULES_OBJECTS_SYSTEM) $(MODULES_OBJECTS_EXT2) $(MODULES_OBJECTS_NET) $(MODULES_OBJECTS_NODEFS) $(MODULES_OBJECTS_IPV4) $(MODULES_OBJECTS_VIDEO)

$(MODULES): LDFLAGS+=-Tmodules/linker.ld -r

$(MODULES_ARP): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_ARP)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BASE): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_BASE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BLOCK): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_BLOCK)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_SYSTEM): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_SYSTEM)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_EXT2): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_EXT2)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NET): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_NET)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NODEFS): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_NODEFS)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_IPV4): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_IPV4)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_VIDEO): libs/fudge/src/memory.o libs/fudge/src/string.o $(MODULES_OBJECTS_VIDEO)
	$(LD) $(LDFLAGS) -o $@ $^

modules/%.o: modules/%.c
	$(CC) -c $(CFLAGS) -Ilibs/fudge/include -Ikernel/include -Imodules/include -o $@ $<
