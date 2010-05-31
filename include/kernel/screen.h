#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_ADDRESS 0xB8000

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

extern void screen_putc(screen_t *screen, char c);
extern void screen_puts(screen_t *screen, char *s);
extern void screen_puts_dec(screen_t *screen, uint32_t n);
extern void screen_puts_hex(screen_t *screen, uint32_t n);
extern void screen_clear(screen_t *screen);
extern void screen_cursor_move(screen_t *screen);
extern void screen_scroll(screen_t *screen);
extern void screen_init();

#endif

