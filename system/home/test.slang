echo < "320x200x8" > /system/video/pci:0000:0000/video/mode
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/pci:0000:0000/video/colormap
pcxdata < /share/icons.pcx > /system/video/pci:0000:0000/video/data
