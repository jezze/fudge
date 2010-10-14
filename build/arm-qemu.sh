#!/bin/sh

if [ ! -f root/boot/kernel ]
then

    echo "ERROR: Kernel not found. Did you forget to compile?"
    exit

fi

qemu-system-arm -m 128 -kernel root/boot/kernel -serial stdio

