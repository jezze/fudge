#ifndef CONFIG_H
#define CONFIG_H

#define ADDRESS_KERNEL			0x100000
#define ADDRESS_VIRTUAL			0x200000
#define ADDRESS_VIDEO			0xB8000
#define ADDRESS_PAGE_DIRECTORY		0x9C000
#define ADDRESS_PAGE_TABLE		0x9D000

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;

#endif

