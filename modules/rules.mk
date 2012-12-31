MODULES_ARP=modules/arp/arp.ko
MODULES_BASE=modules/base/base.ko
MODULES_BLOCK=modules/block/block.ko
MODULES_EXT2=modules/ext2/ext2.ko
MODULES_IPV4=modules/ipv4/ipv4.ko
MODULES_KBD=modules/kbd/kbd.ko
MODULES_NET=modules/net/net.ko
MODULES_NODEFS=modules/nodefs/nodefs.ko
MODULES_SYSTEM=modules/system/system.ko
MODULES_VIDEO=modules/video/video.ko

MODULES_OBJECTS_ARP=modules/arp/main.o modules/arp/protocol.o
MODULES_OBJECTS_BASE=modules/base/main.o
MODULES_OBJECTS_BLOCK=modules/block/main.o modules/block/interface.o modules/block/protocol.o
MODULES_OBJECTS_EXT2=modules/ext2/main.o modules/ext2/protocol.o modules/ext2/filesystem.o
MODULES_OBJECTS_IPV4=modules/ipv4/main.o modules/ipv4/protocol.o
MODULES_OBJECTS_KBD=modules/kbd/main.o modules/kbd/interface.o modules/kbd/protocol.o
MODULES_OBJECTS_NET=modules/net/main.o modules/net/interface.o modules/net/protocol.o
MODULES_OBJECTS_NODEFS=modules/nodefs/main.o modules/nodefs/filesystem.o
MODULES_OBJECTS_SYSTEM=modules/system/main.o modules/system/filesystem.o
MODULES_OBJECTS_VIDEO=modules/video/main.o modules/video/interface.o modules/video/protocol.o

MODULES+=$(MODULES_ARP)
MODULES+=$(MODULES_BASE)
MODULES+=$(MODULES_BLOCK)
MODULES+=$(MODULES_EXT2)
MODULES+=$(MODULES_IPV4)
MODULES+=$(MODULES_KBD)
MODULES+=$(MODULES_NODEFS)
MODULES+=$(MODULES_SYSTEM)
MODULES+=$(MODULES_VIDEO)

MODULES_OBJECTS+=$(MODULES_OBJECTS_ARP)
MODULES_OBJECTS+=$(MODULES_OBJECTS_BASE)
MODULES_OBJECTS+=$(MODULES_OBJECTS_BLOCK)
MODULES_OBJECTS+=$(MODULES_OBJECTS_EXT2)
MODULES_OBJECTS+=$(MODULES_OBJECTS_IPV4)
MODULES_OBJECTS+=$(MODULES_OBJECTS_KBD)
MODULES_OBJECTS+=$(MODULES_OBJECTS_NET)
MODULES_OBJECTS+=$(MODULES_OBJECTS_NODEFS)
MODULES_OBJECTS+=$(MODULES_OBJECTS_SYSTEM)
MODULES_OBJECTS+=$(MODULES_OBJECTS_VIDEO)

include modules/arch/$(ARCH)/rules.mk

$(MODULES): LDFLAGS+=-Tmodules/linker.ld -r

$(MODULES_ARP): $(MODULES_OBJECTS_ARP) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BASE): $(MODULES_OBJECTS_BASE) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BLOCK): $(MODULES_OBJECTS_BLOCK) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_EXT2): $(MODULES_OBJECTS_EXT2) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_IPV4): $(MODULES_OBJECTS_IPV4) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_KBD): $(MODULES_OBJECTS_KBD) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NET): $(MODULES_OBJECTS_NET) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_NODEFS): $(MODULES_OBJECTS_NODEFS) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_SYSTEM): $(MODULES_OBJECTS_SYSTEM) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_VIDEO): $(MODULES_OBJECTS_VIDEO) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

modules/%.o: modules/%.c
	$(CC) -c $(CFLAGS) -Ilibs -Ikernel -Imodules -o $@ $<
