echo < /share/icons.pcx > /system/video/pci:0/mode
pcxcmap < /share/icons.pcx > /system/pipe:1/0
cconv8to6 < /system/pipe:1/1 > /system/video/pci:0/colormap
pcxdata < /share/icons.pcx > /system/video/pci:0/data
