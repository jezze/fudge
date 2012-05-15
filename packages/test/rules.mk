packages-test: GCCFLAGS=-static -s -O2 -Wall -ffreestanding -nostdlib -Iinclude/lib -Llib/
packages-test: LIB=-lfudge

packages-test:
	@mkdir -p image/bin
	@${GCC} ${GCCFLAGS} packages/test/event1.c ${LIB} -o image/bin/event1
	@${GCC} ${GCCFLAGS} packages/test/event2.c ${LIB} -o image/bin/event2
	@${GCC} ${GCCFLAGS} packages/test/event3.c ${LIB} -o image/bin/event3

packages-test-clean:
	@rm -f image/bin/event1
	@rm -f image/bin/event2
	@rm -f image/bin/event3

