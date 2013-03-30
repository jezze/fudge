include modules/arp/rules.mk
include modules/base/rules.mk
include modules/block/rules.mk
include modules/ext2/rules.mk
include modules/ipv4/rules.mk
include modules/ipv6/rules.mk
include modules/kbd/rules.mk
include modules/mouse/rules.mk
include modules/net/rules.mk
include modules/system/rules.mk
include modules/terminal/rules.mk
include modules/video/rules.mk
include modules/arch/$(ARCH)/rules.mk

$(MODULES): LDFLAGS+=-Tmodules/linker.ld -r

modules/%.o: modules/%.s
	$(AS) $(ASFLAGS) -o $@ $<

modules/%.o: modules/%.c
	$(CC) -c $(CFLAGS) -Ilibs -Imodules -o $@ $<
