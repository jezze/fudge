#define PALETTE_BUTTON                  0
#define PALETTE_CHECKBOX                1
#define PALETTE_BOX                     2
#define PALETTE_ITEM                    3
#define PALETTE_LINK                    4
#define PALETTE_MOUSE                   5
#define PALETTE_PANEL                   6
#define PALETTE_SELECT                  7
#define PALETTE_SWATCH                  8
#define PALETTE_TEXT                    9
#define PALETTE_WINDOW                  10
#define NUM_PALETTES                    11
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
