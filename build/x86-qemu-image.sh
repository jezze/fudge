#!/bin/sh

if [ ! -f fudge.img ]
then

    echo "ERROR: Floppy image does not exist. Forget to make one?"
    exit

fi

if [ ! -f hda.img ]
then

    echo "ERROR: Hard drive image does not exist. Forget to make one?"
    exit

fi

qemu -fda fudge.img -hda hda.img -cpu coreduo -serial stdio

