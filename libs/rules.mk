libs/%.o: CFLAGS+=-Ilibs

include libs/fudge/rules.mk
include libs/kernel/rules.mk
include libs/posix/rules.mk
