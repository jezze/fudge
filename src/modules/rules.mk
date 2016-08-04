MOD:=\
    $(DIR_SRC)/modules/arp/arp.ko \
    $(DIR_SRC)/modules/audio/audio.ko \
    $(DIR_SRC)/modules/base/base.ko \
    $(DIR_SRC)/modules/block/block.ko \
    $(DIR_SRC)/modules/clock/clock.ko \
    $(DIR_SRC)/modules/con/con.ko \
    $(DIR_SRC)/modules/con/con.ko.0 \
    $(DIR_SRC)/modules/con/con.ko.1 \
    $(DIR_SRC)/modules/con/con.ko.2 \
    $(DIR_SRC)/modules/con/con.ko.3 \
    $(DIR_SRC)/modules/con/con.ko.4 \
    $(DIR_SRC)/modules/con/con.ko.5 \
    $(DIR_SRC)/modules/con/con.ko.6 \
    $(DIR_SRC)/modules/con/con.ko.7 \
    $(DIR_SRC)/modules/console/console.ko \
    $(DIR_SRC)/modules/ethernet/ethernet.ko \
    $(DIR_SRC)/modules/event/event.ko \
    $(DIR_SRC)/modules/icmp/icmp.ko \
    $(DIR_SRC)/modules/ipv4/ipv4.ko \
    $(DIR_SRC)/modules/ipv6/ipv6.ko \
    $(DIR_SRC)/modules/keyboard/keyboard.ko \
    $(DIR_SRC)/modules/log/log.ko \
    $(DIR_SRC)/modules/mouse/mouse.ko \
    $(DIR_SRC)/modules/mtwist/mtwist.ko \
    $(DIR_SRC)/modules/null/null.ko.0 \
    $(DIR_SRC)/modules/pipe/pipe.ko \
    $(DIR_SRC)/modules/pipe/pipe.ko.0 \
    $(DIR_SRC)/modules/pipe/pipe.ko.1 \
    $(DIR_SRC)/modules/pipe/pipe.ko.2 \
    $(DIR_SRC)/modules/pipe/pipe.ko.3 \
    $(DIR_SRC)/modules/pipe/pipe.ko.4 \
    $(DIR_SRC)/modules/pipe/pipe.ko.5 \
    $(DIR_SRC)/modules/pipe/pipe.ko.6 \
    $(DIR_SRC)/modules/pipe/pipe.ko.7 \
    $(DIR_SRC)/modules/system/system.ko \
    $(DIR_SRC)/modules/timer/timer.ko \
    $(DIR_SRC)/modules/udp/udp.ko \
    $(DIR_SRC)/modules/video/video.ko \

OBJ_MOD:=\
    $(DIR_SRC)/modules/arp/main.o \
    $(DIR_SRC)/modules/audio/main.o \
    $(DIR_SRC)/modules/base/main.o \
    $(DIR_SRC)/modules/block/main.o \
    $(DIR_SRC)/modules/clock/main.o \
    $(DIR_SRC)/modules/con/main.o \
    $(DIR_SRC)/modules/con/node.o \
    $(DIR_SRC)/modules/console/main.o \
    $(DIR_SRC)/modules/ethernet/main.o \
    $(DIR_SRC)/modules/event/main.o \
    $(DIR_SRC)/modules/icmp/main.o \
    $(DIR_SRC)/modules/ipv4/main.o \
    $(DIR_SRC)/modules/ipv6/main.o \
    $(DIR_SRC)/modules/keyboard/main.o \
    $(DIR_SRC)/modules/log/main.o \
    $(DIR_SRC)/modules/mouse/main.o \
    $(DIR_SRC)/modules/mtwist/main.o \
    $(DIR_SRC)/modules/null/main.o \
    $(DIR_SRC)/modules/pipe/main.o \
    $(DIR_SRC)/modules/pipe/node.o \
    $(DIR_SRC)/modules/system/main.o $(DIR_SRC)/modules/system/backend.o $(DIR_SRC)/modules/system/protocol.o \
    $(DIR_SRC)/modules/timer/main.o \
    $(DIR_SRC)/modules/udp/main.o \
    $(DIR_SRC)/modules/video/main.o \

LDLIBS_MOD:=\
    $(DIR_SRC)/fudge/fudge.a \

$(DIR_SRC)/modules/arp/arp.ko: $(DIR_SRC)/modules/arp/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/audio/audio.ko: $(DIR_SRC)/modules/audio/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/base/base.ko: $(DIR_SRC)/modules/base/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/block/block.ko: $(DIR_SRC)/modules/block/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/clock/clock.ko: $(DIR_SRC)/modules/clock/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko: $(DIR_SRC)/modules/con/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.0: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.1: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.2: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.3: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.4: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.5: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.6: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/con/con.ko.7: $(DIR_SRC)/modules/con/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/console/console.ko: $(DIR_SRC)/modules/console/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/ethernet/ethernet.ko: $(DIR_SRC)/modules/ethernet/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/event/event.ko: $(DIR_SRC)/modules/event/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/icmp/icmp.ko: $(DIR_SRC)/modules/icmp/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/ipv4/ipv4.ko: $(DIR_SRC)/modules/ipv4/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/ipv6/ipv6.ko: $(DIR_SRC)/modules/ipv6/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/keyboard/keyboard.ko: $(DIR_SRC)/modules/keyboard/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/log/log.ko: $(DIR_SRC)/modules/log/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/mouse/mouse.ko: $(DIR_SRC)/modules/mouse/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/mtwist/mtwist.ko: $(DIR_SRC)/modules/mtwist/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/null/null.ko.0: $(DIR_SRC)/modules/null/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko: $(DIR_SRC)/modules/pipe/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.0: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.1: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.2: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.3: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.4: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.5: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.6: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/pipe/pipe.ko.7: $(DIR_SRC)/modules/pipe/node.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/system/system.ko: $(DIR_SRC)/modules/system/main.o $(DIR_SRC)/modules/system/backend.o $(DIR_SRC)/modules/system/protocol.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/timer/timer.ko: $(DIR_SRC)/modules/timer/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/udp/udp.ko: $(DIR_SRC)/modules/udp/main.o $(LDLIBS_MOD)
$(DIR_SRC)/modules/video/video.ko: $(DIR_SRC)/modules/video/main.o $(LDLIBS_MOD)

include $(DIR_SRC)/modules/arch/$(ARCH)/rules.mk

OBJ:=$(OBJ) $(OBJ_MOD)

$(DIR_SRC)/modules/%.ko: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.0: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.0:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.1: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.1:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.2: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.2:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.3: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.3:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.4: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.4:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.5: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.5:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.6: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.6:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DIR_SRC)/modules/%.ko.7: LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(DIR_SRC)/modules/%.ko.7:
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

