/bin/motd > /system/console/pci:0/data
/bin/keymapus < /system/keyboard/ps2:0/data | /bin/shell > /system/console/pci:0/data
/bin/motd > /system/console/platform:0/data
/bin/shell < /system/console/platform:0/data > /system/console/platform:0/data
