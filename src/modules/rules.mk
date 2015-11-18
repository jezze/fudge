MODULES:=\
    $(SRC_PATH)/modules/arp/arp.ko \
    $(SRC_PATH)/modules/audio/audio.ko \
    $(SRC_PATH)/modules/base/base.ko \
    $(SRC_PATH)/modules/block/block.ko \
    $(SRC_PATH)/modules/clock/clock.ko \
    $(SRC_PATH)/modules/console/console.ko \
    $(SRC_PATH)/modules/ethernet/ethernet.ko \
    $(SRC_PATH)/modules/event/event.ko \
    $(SRC_PATH)/modules/ipv4/ipv4.ko \
    $(SRC_PATH)/modules/ipv6/ipv6.ko \
    $(SRC_PATH)/modules/keyboard/keyboard.ko \
    $(SRC_PATH)/modules/log/log.ko \
    $(SRC_PATH)/modules/mouse/mouse.ko \
    $(SRC_PATH)/modules/mtwist/mtwist.ko \
    $(SRC_PATH)/modules/null/null.ko.0 \
    $(SRC_PATH)/modules/pipe/pipe.ko \
    $(SRC_PATH)/modules/pipe/pipe.ko.0 \
    $(SRC_PATH)/modules/pipe/pipe.ko.1 \
    $(SRC_PATH)/modules/pipe/pipe.ko.2 \
    $(SRC_PATH)/modules/pipe/pipe.ko.3 \
    $(SRC_PATH)/modules/pipe/pipe.ko.4 \
    $(SRC_PATH)/modules/pipe/pipe.ko.5 \
    $(SRC_PATH)/modules/pipe/pipe.ko.6 \
    $(SRC_PATH)/modules/pipe/pipe.ko.7 \
    $(SRC_PATH)/modules/system/system.ko \
    $(SRC_PATH)/modules/timer/timer.ko \
    $(SRC_PATH)/modules/udp/udp.ko \
    $(SRC_PATH)/modules/video/video.ko \

OBJ_MODULES:=\
    $(SRC_PATH)/modules/arp/main.o \
    $(SRC_PATH)/modules/audio/main.o \
    $(SRC_PATH)/modules/base/main.o \
    $(SRC_PATH)/modules/block/main.o \
    $(SRC_PATH)/modules/clock/main.o \
    $(SRC_PATH)/modules/console/main.o \
    $(SRC_PATH)/modules/ethernet/main.o \
    $(SRC_PATH)/modules/event/main.o \
    $(SRC_PATH)/modules/ipv4/main.o \
    $(SRC_PATH)/modules/ipv6/main.o \
    $(SRC_PATH)/modules/keyboard/main.o \
    $(SRC_PATH)/modules/log/main.o \
    $(SRC_PATH)/modules/mouse/main.o \
    $(SRC_PATH)/modules/mtwist/main.o \
    $(SRC_PATH)/modules/null/main.o \
    $(SRC_PATH)/modules/pipe/main.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/pipe/node.o \
    $(SRC_PATH)/modules/system/main.o $(SRC_PATH)/modules/system/backend.o $(SRC_PATH)/modules/system/protocol.o \
    $(SRC_PATH)/modules/timer/main.o \
    $(SRC_PATH)/modules/udp/main.o \
    $(SRC_PATH)/modules/video/main.o \

$(SRC_PATH)/modules/arp/arp.ko: $(SRC_PATH)/modules/arp/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/audio/audio.ko: $(SRC_PATH)/modules/audio/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/base/base.ko: $(SRC_PATH)/modules/base/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/block/block.ko: $(SRC_PATH)/modules/block/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/clock/clock.ko: $(SRC_PATH)/modules/clock/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/console/console.ko: $(SRC_PATH)/modules/console/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/ethernet/ethernet.ko: $(SRC_PATH)/modules/ethernet/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/event/event.ko: $(SRC_PATH)/modules/event/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/ipv4/ipv4.ko: $(SRC_PATH)/modules/ipv4/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/ipv6/ipv6.ko: $(SRC_PATH)/modules/ipv6/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/keyboard/keyboard.ko: $(SRC_PATH)/modules/keyboard/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/log/log.ko: $(SRC_PATH)/modules/log/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/mouse/mouse.ko: $(SRC_PATH)/modules/mouse/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/mtwist/mtwist.ko: $(SRC_PATH)/modules/mtwist/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/null/null.ko.0: $(SRC_PATH)/modules/null/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko: $(SRC_PATH)/modules/pipe/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.0: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.1: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.2: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.3: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.4: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.5: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.6: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/pipe/pipe.ko.7: $(SRC_PATH)/modules/pipe/node.o $(MOD_DEPS)
$(SRC_PATH)/modules/system/system.ko: $(SRC_PATH)/modules/system/main.o $(SRC_PATH)/modules/system/backend.o $(SRC_PATH)/modules/system/protocol.o $(MOD_DEPS)
$(SRC_PATH)/modules/timer/timer.ko: $(SRC_PATH)/modules/timer/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/udp/udp.ko: $(SRC_PATH)/modules/udp/main.o $(MOD_DEPS)
$(SRC_PATH)/modules/video/video.ko: $(SRC_PATH)/modules/video/main.o $(MOD_DEPS)

include $(SRC_PATH)/modules/arch/$(ARCH)/rules.mk

CLEAN:=$(CLEAN) $(MODULES) $(OBJ_MODULES)

$(SRC_PATH)/modules/%.ko: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.0: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.0:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.1: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.1:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.2: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.2:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.3: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.3:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.4: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.4:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.5: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.5:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.6: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.6:
	$(LD) -o $@ $(LDFLAGS) $^

$(SRC_PATH)/modules/%.ko.7: LDFLAGS+=-T$(SRC_PATH)/modules/linker.ld -r
$(SRC_PATH)/modules/%.ko.7:
	$(LD) -o $@ $(LDFLAGS) $^

