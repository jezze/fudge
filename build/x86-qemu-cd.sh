#!/bin/sh

if [ ! -f fudge.iso ]
then
    echo "ERROR: ISO image not found. Forget to make one?"
    exit
fi

qemu -cdrom fudge.iso -cpu coreduo -serial stdio

