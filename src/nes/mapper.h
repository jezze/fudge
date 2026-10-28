#pragma once

#include "port.h"

#define INES_HEADER_SIZE 16

typedef enum TVSystem {
    NTSC = 0,
    DUAL,
    PAL,
    DENDY,
} TVSystem;

typedef enum {
    NO_MIRRORING,
    VERTICAL,
    HORIZONTAL,
    ONE_SCREEN,
    ONE_SCREEN_LOWER,
    ONE_SCREEN_UPPER,
    FOUR_SCREEN,
} Mirroring;

typedef enum MapperFormat {
    ARCHAIC_INES,
    INES,
    NES2,
} MapperFormat;

struct Emulator;

typedef struct ROMData {
    char *rom_name;
    uint8_t *rom;
    uint32_t rom_size;
} ROMData;

typedef struct Mapper {
    uint8_t *CHR_ROM;
    uint8_t *PRG_ROM;
    uint8_t *PRG_RAM;
    uint8_t *PRG_ptrs[8];
    uint8_t *CHR_ptrs[8];
    uint8_t PRG_regs[8];
    uint8_t CHR_regs[8];
    uint16_t PRG_banks;
    uint16_t CHR_banks;
    size_t CHR_RAM_size;
    uint8_t RAM_banks;
    size_t RAM_size;
    Mirroring mirroring;
    TVSystem type;
    MapperFormat format;
    uint16_t name_table_map[4];
    uint32_t clamp;
    uint32_t PRG_RAM_clamp;
    uint16_t mapper_num;
    uint8_t submapper;
    void (*set_bus)(struct Mapper *, uint16_t);
    uint8_t (*read_ROM)(struct Mapper *, uint16_t);
    void (*write_ROM)(struct Mapper *, uint16_t, uint8_t);
    uint8_t (*read_PRG)(struct Mapper *, uint16_t);
    void (*write_PRG)(struct Mapper *, uint16_t, uint8_t);
    uint8_t (*read_CHR)(struct Mapper *, uint16_t);
    void (*write_CHR)(struct Mapper *, uint16_t, uint8_t);
    void (*reset)(struct Mapper *);

    // mapper extension structs would be attached here
    // memory should be allocated dynamically and should
    // not be freed since this is done by the generic mapper functions
    void *extension;
    struct Emulator *emulator;
} Mapper;

int load_data(ROMData *data, Mapper *mapper);
void set_mirroring(Mapper *mapper, Mirroring mirroring);

// mapper specifics

int load_UXROM(Mapper *mapper);
int load_MMC1(Mapper *mapper);
int load_CNROM(Mapper *mapper);
int load_GNROM(Mapper *mapper);
int load_AOROM(Mapper *mapper);
int load_MMC3(Mapper *mapper);
int load_colordreams(Mapper *mapper);
int load_colordreams46(Mapper *mapper);
int load_VRC1(Mapper *mapper);
int load_UN1ROM(Mapper *mapper);
int load_mapper180(Mapper *mapper);
int load_mapper185(Mapper *mapper);
int load_CPROM(Mapper *mapper);
