#!/bin/sh
qemu -fda fudge.img -hda hda.img -cpu coreduo -serial stdio

