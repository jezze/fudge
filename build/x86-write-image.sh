#!/bin/sh

if [ ! -f fudge.img ]
then

    echo "ERROR: Floppy image does not exist. Forget to make one?"
    exit

fi

grub <<EOF
device (fd0) fudge.img
install (fd0)0+1 (fd0) (fd0)1+199 (fd0)200+1
EOF

