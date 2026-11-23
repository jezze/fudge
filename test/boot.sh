log -output-service console:0 &
audiosrv &
automount -service block -index 0 -partition 0 -name tfat &
automount -service block -index 0 -partition 1 -name text2 &
initrd:test/runner
