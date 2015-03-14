/bin/motd > /system/console:0/out
/bin/shell > /system/console:0/out | /bin/keymapus < /system/keyboard:0/data
/bin/motd > /system/console:1/out
/bin/shell < /system/console:1/in > /system/console:1/out
