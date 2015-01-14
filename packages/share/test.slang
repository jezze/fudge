echo < /share/icons.pcx > /system/video/vga:0/ctrl
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/vga:0/colormap
pcxdata < /share/icons.pcx > /system/video/vga:0/data
