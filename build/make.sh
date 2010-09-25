#!/bin/sh

case $1
in
    arm) sh arm-make.sh;;
    x86) sh x86-make.sh;;
    *) echo "Usage: make.sh [arm|x86]";;

esac

