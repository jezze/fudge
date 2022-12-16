#define AUDIO_SAMPLE_RATE               48000.0
#define DMG_CLOCK_FREQ                  4194304.0
#define SCREEN_REFRESH_CYCLES           70224.0
#define VERTICAL_SYNC                   (DMG_CLOCK_FREQ/SCREEN_REFRESH_CYCLES)
#define AUDIO_SAMPLES                   ((unsigned)(AUDIO_SAMPLE_RATE / VERTICAL_SYNC))

void audio_callback(void *ptr, unsigned char *data, int len);
unsigned char audio_read(const unsigned short addr);
void audio_write(const unsigned short addr, const unsigned char val);
void audio_init(void);
