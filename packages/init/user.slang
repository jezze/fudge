/bin/motd > /system/console/vga:0/out
/bin/shell > /system/console/vga:0/out | /bin/keymapus < /system/keyboard/ps2keyboard:0/data
/bin/motd > /system/console/uart:0/out
/bin/shell < /system/console/uart:0/in > /system/console/uart:0/out
