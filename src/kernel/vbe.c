#include <lib/types.h>
#include <kernel/screen.h>
#include <kernel/vbe.h>

vbe_controller_t controller;

void vbe_set_controller(uint32_t num)
{

    screen_puts_dec(&screen, num);

}

void vbe_init()
{

    uint32_t s = sum(5, 2);
    screen_puts_dec(&screen, s);

}
