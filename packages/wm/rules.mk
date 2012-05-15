packages-wm: GCCFLAGS=-static -s -O2 -Wall -ffreestanding -nostdlib -Iinclude/lib -Llib/
packages-wm: LIB=-lfudge

packages-wm:
	@mkdir -p image/bin
	@${GCC} ${GCCFLAGS} packages/wm/wm.c ${LIB} -o image/bin/wm

packages-wm-clean:
	@rm -f image/bin/wm

