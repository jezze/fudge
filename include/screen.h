#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_ADDRESS 0xB8000

extern void putc(char c);
extern void puts(char *s);
extern void puts_dec(uint32_t n);
extern void puts_hex(uint32_t n);

extern void screen_clear();
extern void screen_move_cursor();
extern void screen_scroll();
extern void screen_init();

#endif

