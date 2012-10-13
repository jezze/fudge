#!/bin/sh

printf "bootm 0x%X\n" $(expr $(stat -c%s image/boot/uboot/u-boot.bin) + 65536)
