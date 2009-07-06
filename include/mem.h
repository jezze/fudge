#ifndef MEM_H
#define MEM_H

extern void *memcpy(void *, const void *, int32_t);
extern void *memset(void *, int8_t, int32_t);
extern uint16_t *memsetw(uint16_t *, uint16_t, int32_t);
extern uint8_t inb(uint16_t);
extern uint16_t inw(uint16_t);
extern void outb(uint16_t, uint8_t);

#endif

