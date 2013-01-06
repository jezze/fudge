MODULES+=modules/video/video.ko
MODULES_OBJECTS+=modules/video/main.o

modules/video/video.ko: modules/video/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
