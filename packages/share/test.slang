echo < /share/mi.pcx > /system/video/vga:0/ctrl
cconv8to6 > /system/video/vga:0/colormap | pcxcmap < /share/mi.pcx
pcxdata < /share/mi.pcx > /system/video/vga:0/data
