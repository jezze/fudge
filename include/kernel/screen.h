#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_ADDRESS 0xB8000
#define SCREEN_CHARACTER_WIDTH 80
#define SCREEN_CHARACTER_HEIGHT 25

typedef struct screen_context
{

    uint16_t attribute;

} screen_context_t;

typedef struct screen
{

    uint16_t *address;
    uint32_t cursorX;
    uint32_t cursorY;
    screen_context_t context;

} screen_t;

extern screen_t screen;

extern void screen_putc(char c);
extern void screen_puts(char *s);
extern void screen_puts_dec(uint32_t n);
extern void screen_puts_hex(uint32_t n);
extern void screen_clear();
extern void screen_cursor_move();
extern void screen_scroll();
extern void screen_init();

#endif

