#define PALETTE_BUTTON                  0
#define PALETTE_BOX                     1
#define PALETTE_ITEM                    2
#define PALETTE_LINK                    3
#define PALETTE_MOUSE                   4
#define PALETTE_PANEL                   5
#define PALETTE_SELECT                  6
#define PALETTE_SWATCH                  7
#define PALETTE_TEXT                    8
#define PALETTE_WINDOW                  9
#define PALETTE_CYCLE                   10
#define PALETTE_TAB                     11
#define PALETTE_TABCHECKED              12
#define NUM_PALETTES                    13
#define SHADE_NORMAL                    0
#define SHADE_HOVER                     1
#define SHADE_FOCUS                     2
#define SHADE_DISABLED                  3
#define NUM_SHADES                      4
#define PART_SHADOW                     0
#define PART_BODY                       1
#define PART_DARK                       2
#define PART_LIGHT                      3
#define PART_TEXT                       4
#define PART_ICON                       5
#define NUM_PARTS                       6

unsigned int *cmap_get(unsigned int palette, unsigned int shade);
void cmap_set(char *name, unsigned int value);
