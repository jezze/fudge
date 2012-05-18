packages-net: GCCFLAGS=-static -s -O2 -Wall -ffreestanding -nostdlib -Iinclude/lib -Llib/
packages-net: LIB=-lfudge

packages-net:
	@mkdir -p image/bin
	@${GCC} ${GCCFLAGS} packages/net/ncat.c ${LIB} -o image/bin/ncat

packages-net-clean:
	@rm -f image/bin/ncat

