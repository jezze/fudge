echo < /share/giant.pcx > /system/video:0/ctrl
cconv8to6 > /system/video:0/colormap | pcxcmap < /share/giant.pcx
pcxdata < /share/giant.pcx > /system/video:0/data
