echo < /share/icons.pcx > /system/video/pci:0/ctrl
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/pci:0/colormap
pcxdata < /share/icons.pcx > /system/video/pci:0/data
