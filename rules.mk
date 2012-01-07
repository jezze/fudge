ARCH=x86
TARGET=i586-pc-fudge
ASM=${TARGET}-as
ASMFLAGS=-c
GCC=${TARGET}-gcc
GCCFLAGS=-c -O2 -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=${TARGET}-ld
AR=ar
ARFLAGS=rvs

