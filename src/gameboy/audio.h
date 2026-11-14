#define AUDIO_RATE                      48000
/* a frame runs on every timer tick (60 Hz), so each frame makes one tick of sound; at the real 59.7 Hz the device's buffers would slowly fill and the sound fall behind */
#define AUDIO_TICKRATE                  60
#define AUDIO_SAMPLES                   (AUDIO_RATE / AUDIO_TICKRATE)

unsigned char audio_read(const unsigned short addr);
void audio_write(const unsigned short addr, const unsigned char val);
void audio_init(void);
void audio_render(short *out);
