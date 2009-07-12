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

extern void putc(char c);
extern void puts(char *s);
extern void puts_dec(uint32_t n);
extern void puts_hex(uint32_t n);

extern void screen_clear();
extern void screen_move_cursor();
extern void screen_scroll();
extern void screen_init();

#endif

