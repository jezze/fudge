log -output-service console:0 &
mq -query ".networks[type=dynamic]" -format "dhcpsrv -ethernet-service {device} &" data/config/network.mq | sh &
respawn -command "shell -keyboard-service keyboard:1" &
mq -query .mounts -format "automount -service {service} -index {index} -partition {partition} -name {name}" data/config/mount.mq | sh &
audiosrv &
wm &
