MODULES += \
    src/modules/arp/arp.ko \
    src/modules/audio/audio.ko \
    src/modules/base/base.ko \
    src/modules/block/block.ko \
    src/modules/clock/clock.ko \
    src/modules/console/console.ko \
    src/modules/ethernet/ethernet.ko \
    src/modules/event/event.ko \
    src/modules/ipv4/ipv4.ko \
    src/modules/ipv6/ipv6.ko \
    src/modules/keyboard/keyboard.ko \
    src/modules/log/log.ko \
    src/modules/mouse/mouse.ko \
    src/modules/mtwist/mtwist.ko \
    src/modules/null/null.ko.0 \
    src/modules/pipe/pipe.ko \
    src/modules/pipe/pipe.ko.0 \
    src/modules/pipe/pipe.ko.1 \
    src/modules/pipe/pipe.ko.2 \
    src/modules/pipe/pipe.ko.3 \
    src/modules/pipe/pipe.ko.4 \
    src/modules/pipe/pipe.ko.5 \
    src/modules/pipe/pipe.ko.6 \
    src/modules/pipe/pipe.ko.7 \
    src/modules/system/system.ko \
    src/modules/timer/timer.ko \
    src/modules/udp/udp.ko \
    src/modules/video/video.ko \
    src/modules/window/window.ko.0 \

CLEAN += \
    src/modules/arp/arp.ko src/modules/arp/main.o \
    src/modules/audio/audio.ko src/modules/audio/main.o \
    src/modules/base/base.ko src/modules/base/main.o \
    src/modules/block/block.ko src/modules/block/main.o \
    src/modules/clock/clock.ko src/modules/clock/main.o \
    src/modules/console/console.ko src/modules/console/main.o \
    src/modules/ethernet/ethernet.ko src/modules/ethernet/main.o \
    src/modules/event/event.ko src/modules/event/main.o \
    src/modules/ipv4/ipv4.ko src/modules/ipv4/main.o \
    src/modules/ipv6/ipv6.ko src/modules/ipv6/main.o \
    src/modules/keyboard/keyboard.ko src/modules/keyboard/main.o \
    src/modules/log/log.ko src/modules/log/main.o \
    src/modules/mouse/mouse.ko src/modules/mouse/main.o \
    src/modules/mtwist/mtwist.ko src/modules/mtwist/main.o \
    src/modules/null/null.ko.0 src/modules/null/main.o \
    src/modules/pipe/pipe.ko src/modules/pipe/main.o \
    src/modules/pipe/pipe.ko.0 src/modules/pipe/node.o \
    src/modules/pipe/pipe.ko.1 src/modules/pipe/node.o \
    src/modules/pipe/pipe.ko.2 src/modules/pipe/node.o \
    src/modules/pipe/pipe.ko.3 src/modules/pipe/node.o \
    src/modules/pipe/pipe.ko.4 src/modules/pipe/node.o \
    src/modules/pipe/pipe.ko.5 src/modules/pipe/node.o \
    src/modules/pipe/pipe.ko.6 src/modules/pipe/node.o \
    src/modules/pipe/pipe.ko.7 src/modules/pipe/node.o \
    src/modules/system/system.ko src/modules/system/main.o src/modules/system/backend.o src/modules/system/protocol.o \
    src/modules/timer/timer.ko src/modules/timer/main.o \
    src/modules/udp/udp.ko src/modules/udp/main.o \
    src/modules/video/video.ko src/modules/video/main.o \
    src/modules/window/window.ko.0 src/modules/window/main.o \

src/modules/arp/arp.ko: src/modules/arp/main.o
src/modules/audio/audio.ko: src/modules/audio/main.o
src/modules/base/base.ko: src/modules/base/main.o
src/modules/block/block.ko: src/modules/block/main.o
src/modules/clock/clock.ko: src/modules/clock/main.o
src/modules/console/console.ko: src/modules/console/main.o
src/modules/ethernet/ethernet.ko: src/modules/ethernet/main.o
src/modules/event/event.ko: src/modules/event/main.o
src/modules/ipv4/ipv4.ko: src/modules/ipv4/main.o
src/modules/ipv6/ipv6.ko: src/modules/ipv6/main.o
src/modules/keyboard/keyboard.ko: src/modules/keyboard/main.o
src/modules/log/log.ko: src/modules/log/main.o
src/modules/mouse/mouse.ko: src/modules/mouse/main.o
src/modules/mtwist/mtwist.ko: src/modules/mtwist/main.o
src/modules/null/null.ko.0: src/modules/null/main.o
src/modules/pipe/pipe.ko: src/modules/pipe/main.o
src/modules/pipe/pipe.ko.0: src/modules/pipe/node.o
src/modules/pipe/pipe.ko.1: src/modules/pipe/node.o
src/modules/pipe/pipe.ko.2: src/modules/pipe/node.o
src/modules/pipe/pipe.ko.3: src/modules/pipe/node.o
src/modules/pipe/pipe.ko.4: src/modules/pipe/node.o
src/modules/pipe/pipe.ko.5: src/modules/pipe/node.o
src/modules/pipe/pipe.ko.6: src/modules/pipe/node.o
src/modules/pipe/pipe.ko.7: src/modules/pipe/node.o
src/modules/system/system.ko: src/modules/system/main.o src/modules/system/backend.o src/modules/system/protocol.o
src/modules/timer/timer.ko: src/modules/timer/main.o
src/modules/udp/udp.ko: src/modules/udp/main.o
src/modules/video/video.ko: src/modules/video/main.o
src/modules/window/window.ko.0: src/modules/window/main.o

include src/modules/arch/$(ARCH)/rules.mk

src/modules/%.ko: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.0: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.0:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.1: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.1:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.2: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.2:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.3: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.3:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.4: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.4:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.5: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.5:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.6: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.6:
	$(LD) -o $@ $(LDFLAGS) $^

src/modules/%.ko.7: LDFLAGS+=-Tsrc/modules/linker.ld -r
src/modules/%.ko.7:
	$(LD) -o $@ $(LDFLAGS) $^

