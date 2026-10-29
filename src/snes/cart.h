
#ifndef CART_H
#define CART_H

#include "port.h"

typedef struct Cart Cart;

#include "snes.h"

struct Cart {
  Snes* snes;
  uint8_t type;
  bool hasBattery;

  uint8_t* rom;
  uint32_t romSize;
  uint8_t* ram;
  uint32_t ramSize;
};

// TODO: how to handle reset & load?

Cart* cart_init(Snes* snes);
void cart_reset(Cart* cart); // will reset special chips etc, general reading is set up in load
void cart_load(Cart* cart, int type, uint8_t* rom, int romSize, int ramSize, bool hasBattery); // takes rom, sets up ram buffer
uint8_t cart_read(Cart* cart, uint8_t bank, uint16_t adr);
void cart_write(Cart* cart, uint8_t bank, uint16_t adr, uint8_t val);

#endif
