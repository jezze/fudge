#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_ADDRESS 0xB8000

extern void putc(char);
extern void puts(char *);
extern void puts_dec(uint32_t);
extern void puts_hex(uint32_t);

extern void screen_clear();
extern void screen_move_cursor();
extern void screen_scroll();
extern void screen_init();

#endif

