log -output-service console:0 &
shell -keyboard-service keyboard:1 &
mq -query .mounts -format "automount -service {service} -index {index} -partition {partition} -name {name}" data/config/mount.mq | sh &
audiosrv &
wm &
