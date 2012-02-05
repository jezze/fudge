user: GCCFLAGS=-static -s -O2 -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Iinclude/lib -Llib/
user: LIB=-lfudge

user:
	@${GCC} ${GCCFLAGS} user/cat.c ${LIB} -o user/cat
	@${GCC} ${GCCFLAGS} user/cd.c ${LIB} -o user/cd
	@${GCC} ${GCCFLAGS} user/date.c ${LIB} -o user/date
	@${GCC} ${GCCFLAGS} user/echo.c ${LIB} -o user/echo
	@${GCC} ${GCCFLAGS} user/event1.c ${LIB} -o user/event1
	@${GCC} ${GCCFLAGS} user/event2.c ${LIB} -o user/event2
	@${GCC} ${GCCFLAGS} user/event3.c ${LIB} -o user/event3
	@${GCC} ${GCCFLAGS} user/hello.c ${LIB} -o user/hello
	@${GCC} ${GCCFLAGS} user/init.c ${LIB} -o user/init
	@${GCC} ${GCCFLAGS} user/load.c ${LIB} -o user/load
	@${GCC} ${GCCFLAGS} user/ls.c ${LIB} -o user/ls
	@${GCC} ${GCCFLAGS} user/reboot.c ${LIB} -o user/reboot
	@${GCC} ${GCCFLAGS} user/shell.c ${LIB} -o user/shell
	@${GCC} ${GCCFLAGS} user/tail.c ${LIB} -o user/tail
	@${GCC} ${GCCFLAGS} user/timer.c ${LIB} -o user/timer
	@${GCC} ${GCCFLAGS} user/unload.c ${LIB} -o user/unload
	@${GCC} user/test.c -o user/test

user-clean:
	@rm -f user/cat
	@rm -f user/cd
	@rm -f user/date
	@rm -f user/echo
	@rm -f user/event1
	@rm -f user/event2
	@rm -f user/event3
	@rm -f user/hello
	@rm -f user/init
	@rm -f user/load
	@rm -f user/ls
	@rm -f user/reboot
	@rm -f user/shell
	@rm -f user/tail
	@rm -f user/timer
	@rm -f user/unload
	@rm -f user/test

