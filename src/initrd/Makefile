GCC=gcc
GCCFLAGS=-O2

all: 
	$(GCC) $(GCCFLAGS) mkinitrd.c -o mkinitrd

disk:
	./mkinitrd files/about.txt about.txt files/hello.c hello.c

clean:
	rm -f mkinitrd
	rm -f ../../root/boot/initrd

