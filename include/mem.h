#ifndef MEM_H
#define MEM_H

extern void *memcpy(void *dest, const void *src, int32_t count);
extern void *memset(void *dest, int8_t val, int32_t count);
extern uint16_t *memsetw(uint16_t *dest, uint16_t val, int32_t count);
extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern void outb(uint16_t port, uint8_t data);

#endif

