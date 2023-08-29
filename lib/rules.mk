include $(DIR_LIB)/abi/rules.mk
include $(DIR_LIB)/fudge/rules.mk
include $(DIR_LIB)/image/rules.mk
include $(DIR_LIB)/hash/rules.mk
ifeq ($(ARCH),x86)
include $(DIR_LIB)/math/rules.mk
endif
include $(DIR_LIB)/net/rules.mk
include $(DIR_LIB)/socket/rules.mk
ifeq ($(TARGET),i386-tcc)
include $(DIR_LIB)/tcc/rules.mk
endif
