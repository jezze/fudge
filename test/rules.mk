B:=\
    $(DIR_TEST)/abort \

O:=\
    $(DIR_TEST)/abort.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/flood \

O:=\
    $(DIR_TEST)/flood.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/init \

O:=\
    $(DIR_TEST)/init.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/peer \

O:=\
    $(DIR_TEST)/peer.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/pingpong \

O:=\
    $(DIR_TEST)/pingpong.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/runner \

O:=\
    $(DIR_TEST)/runner.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/sender \

O:=\
    $(DIR_TEST)/sender.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/spawn \

O:=\
    $(DIR_TEST)/spawn.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/stale \

O:=\
    $(DIR_TEST)/stale.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/window0 \

O:=\
    $(DIR_TEST)/window0.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

B:=\
    $(DIR_TEST)/window1 \

O:=\
    $(DIR_TEST)/window1.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

include $(DIR_MK)/tbin.mk

$(DIR_BUILDTESTINITRD): $(DIR_BUILDINITRD) $(TESTBIN) $(DIR_TEST)/boot.sh
	@echo BUILD TESTINITRD $@
	@rm -rf $@
	@cp -r $(DIR_BUILDINITRD) $@
	@mkdir -p $@/test
	@cp $(filter-out $(DIR_TEST)/init,$(TESTBIN)) $(DIR_TEST)/boot.sh $@/test
	@cp $(DIR_TEST)/init $@/bin/init

$(TESTRAMDISK): $(DIR_BUILDTESTINITRD)
	@echo RAMDISK $@
	@find $^ -depth | cpio -o -H newc > $@

$(DIR_BUILDTESTISO): $(KERNEL) $(TESTRAMDISK)
	@echo ISO $@
	@rm -rf $@
	@mkdir -p $@/bin
	@mkdir -p $@/boot/grub
	@cp $(KERNEL) $@
	@cp $(TESTRAMDISK) $@/$(RAMDISK)
	@cp $(DIR_TEST)/init $@/bin
	@echo "set timeout=0" > $@/boot/grub/grub.cfg
	@sed 's|(hd0,3)/bin/|/bin/|' grub/$(LOADER).cfg >> $@/boot/grub/grub.cfg

$(TESTISO): $(DIR_BUILDTESTISO)
	@echo ISO $@
	@grub-mkrescue -o $@ $^ 2> /dev/null
