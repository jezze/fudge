#!/bin/sh

dd if=/dev/zero of=fudge.img bs=512 count=2880
dd if=root/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
dd if=root/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
dd if=menu.lst conv=notrunc of=fudge.img bs=512 seek=200
dd if=root/boot/kernel conv=notrunc of=fudge.img bs=512 seek=300
dd if=root/boot/initrd conv=notrunc of=fudge.img bs=512 seek=400

grub <<EOF
device (fd0) fudge.img
install (fd0)0+1 (fd0) (fd0)1+199 (fd0)200+1
EOF

