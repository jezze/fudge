echo < "320x200x8" > /system/video/dev/pci:0000:0000/mode
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/dev/pci:0000:0000/colormap
pcxdata < /share/icons.pcx > /system/video/dev/pci:0000:0000/data
