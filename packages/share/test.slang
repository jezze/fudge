echo < /share/icons.pcx > /system/video/dev/pci/mode
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/dev/pci/colormap
pcxdata < /share/icons.pcx > /system/video/dev/pci/data
