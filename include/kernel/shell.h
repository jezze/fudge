#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

#define SHELL_BUFFER_SIZE 256
#define SHELL_CHARACTER_WIDTH  80
#define SHELL_CHARACTER_HEIGHT 25
#define SHELL_COLOR_BLACK      0x00
#define SHELL_COLOR_BLUE       0x01
#define SHELL_COLOR_BLUE2      0x09
#define SHELL_COLOR_BROWN      0x06
#define SHELL_COLOR_BROWN2     0x0e
#define SHELL_COLOR_CYAN       0x03
#define SHELL_COLOR_CYAN2      0x0b
#define SHELL_COLOR_GRAY       0x08
#define SHELL_COLOR_GRAY2      0x07
#define SHELL_COLOR_GREEN      0x02
#define SHELL_COLOR_GREEN2     0x0a
#define SHELL_COLOR_MAGENTA    0x05
#define SHELL_COLOR_MAGENTA2   0x0d
#define SHELL_COLOR_RED        0x04
#define SHELL_COLOR_RED2       0x0c
#define SHELL_COLOR_WHITE      0x0f

extern void shell_init();

#endif
