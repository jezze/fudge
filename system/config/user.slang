/bin/echo < /config/initmod.conf | /bin/dep | /bin/initmod
/bin/keymapus > /system/keyboard/dev/ps2/keymap
/bin/motd > /system/terminal/dev/pci/data
/bin/shell < /system/keyboard/dev/ps2/data > /system/terminal/dev/pci/data
