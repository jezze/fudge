#define KEYMOD_NONE                     0
#define KEYMOD_SHIFT                    1
#define KEYMOD_ALT                      2
#define KEYMOD_CTRL                     3
#define KEYMAP_NONE                     0
#define KEYMAP_US                       1
#define KEYMAP_SE                       2

void keymap_write(struct ring *ring, unsigned int scancode, unsigned int modifier);
