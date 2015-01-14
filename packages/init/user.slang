/bin/motd > /system/console/vga:0/data
/bin/keymapus < /system/keyboard/ps2keyboard:0/data | /bin/shell > /system/console/vga:0/data
/bin/motd > /system/console/uart:0/data
/bin/shell < /system/console/uart:0/data > /system/console/uart:0/data
