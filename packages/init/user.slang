/bin/dep < /config/initmod.conf | /bin/initmod
/bin/keymapus > /system/keyboard/dev/ps2/keymap
/bin/motd > /system/terminal/dev/pci/data
/bin/shell < /system/keyboard/dev/ps2/data > /system/terminal/dev/pci/data
/bin/motd > /system/terminal/dev/uart0/data
/bin/shell < /system/terminal/dev/uart0/data > /system/terminal/dev/uart0/data
