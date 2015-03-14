echo < /share/mi.pcx > /system/video:0/ctrl
cconv8to6 > /system/video:0/colormap | pcxcmap < /share/mi.pcx
pcxdata < /share/mi.pcx > /system/video:0/data
