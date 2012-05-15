packages-base: GCCFLAGS=-static -s -O2 -Wall -ffreestanding -nostdlib -Iinclude/lib -Llib/
packages-base: LIB=-lfudge

packages-base:
	@mkdir -p image/bin
	@${GCC} ${GCCFLAGS} packages/base/cd.c ${LIB} -o image/bin/cd
	@${GCC} ${GCCFLAGS} packages/base/date.c ${LIB} -o image/bin/date
	@${GCC} ${GCCFLAGS} packages/base/echo.c ${LIB} -o image/bin/echo
	@${GCC} ${GCCFLAGS} packages/base/hello.c ${LIB} -o image/bin/hello
	@${GCC} ${GCCFLAGS} packages/base/init.c ${LIB} -o image/bin/init
	@${GCC} ${GCCFLAGS} packages/base/load.c ${LIB} -o image/bin/load
	@${GCC} ${GCCFLAGS} packages/base/ls.c ${LIB} -o image/bin/ls
	@${GCC} ${GCCFLAGS} packages/base/ncat.c ${LIB} -o image/bin/ncat
	@${GCC} ${GCCFLAGS} packages/base/reboot.c ${LIB} -o image/bin/reboot
	@${GCC} ${GCCFLAGS} packages/base/shell.c ${LIB} -o image/bin/shell
	@${GCC} ${GCCFLAGS} packages/base/tail.c ${LIB} -o image/bin/tail
	@${GCC} ${GCCFLAGS} packages/base/timer.c ${LIB} -o image/bin/timer
	@${GCC} ${GCCFLAGS} packages/base/unload.c ${LIB} -o image/bin/unload

packages-base-clean:
	@rm -f image/bin/cd
	@rm -f image/bin/date
	@rm -f image/bin/echo
	@rm -f image/bin/hello
	@rm -f image/bin/init
	@rm -f image/bin/load
	@rm -f image/bin/ls
	@rm -f image/bin/ncat
	@rm -f image/bin/reboot
	@rm -f image/bin/shell
	@rm -f image/bin/tail
	@rm -f image/bin/timer
	@rm -f image/bin/unload

