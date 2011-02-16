#!/bin/sh

if [ ! -f fudge.iso ]
then

    echo "ERROR: ISO image not found. Forget to make one?"
    exit

fi

bochs -q -f iso.bochs

