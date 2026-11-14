#define ENABLE_AUDIO 1
#define ENABLE_VIDEO 1

#define WRAM_SIZE                       0x2000
#define VRAM_SIZE                       0x2000
#define HRAM_SIZE                       0x0100
#define OAM_SIZE                        0x00A0

struct tm
{

    unsigned int tm_sec;
    unsigned int tm_min;
    unsigned int tm_hour;
    unsigned int tm_yday;

};

enum gb_init_error_e
{

    GB_INIT_NO_ERROR,
    GB_INIT_CARTRIDGE_UNSUPPORTED,
    GB_INIT_INVALID_CHECKSUM

};

enum gb_error_e
{

    GB_UNKNOWN_ERROR,
    GB_INVALID_OPCODE,
    GB_INVALID_READ,
    GB_INVALID_WRITE,
    GB_INVALID_MAX

};

struct cpu_registers_s
{

    union
    {

        struct
        {

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            unsigned char a;
#endif

            union
            {

                struct
                {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
                    unsigned char z : 1; /* Zero flag. */
                    unsigned char n : 1; /* Add/sub flag. */
                    unsigned char h : 1; /* Half carry flag. */
                    unsigned char c : 1; /* Carry flag. */
                    unsigned char unused : 4;
#else
                    unsigned char unused : 4;
                    unsigned char c : 1; /* Carry flag. */
                    unsigned char h : 1; /* Half carry flag. */
                    unsigned char n : 1; /* Add/sub flag. */
                    unsigned char z : 1; /* Zero flag. */
#endif
                } f_bits;

                unsigned char f;

            };

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
            unsigned char a;
#endif

        };

        unsigned short af;

    };

    union
    {

        struct
        {

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            unsigned char b;
            unsigned char c;
#else
            unsigned char c;
            unsigned char b;
#endif

        };

        unsigned short bc;

    };

    union
    {

        struct
        {

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            unsigned char d;
            unsigned char e;
#else
            unsigned char e;
            unsigned char d;
#endif

        };

        unsigned short de;

    };

    union
    {

        struct
        {

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            unsigned char h;
            unsigned char l;
#else
            unsigned char l;
            unsigned char h;
#endif

        };

        unsigned short hl;

    };

    unsigned short sp; /* Stack pointer */
    unsigned short pc; /* Program counter */

};

struct gb_registers_s
{

    unsigned char TIMA, TMA, DIV;
    union
    {

        struct
        {

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            unsigned char unused : 5;
            unsigned char tac_enable : 1; /* Timer enable */
            unsigned char tac_rate : 2; /* Input clock select */
#else
            unsigned char tac_rate : 2; /* Input clock select */
            unsigned char tac_enable : 1; /* Timer enable */
            unsigned char unused : 5;
#endif
        };

        unsigned char TAC;

    };

    unsigned char LCDC;
    unsigned char STAT;
    unsigned char SCY;
    unsigned char SCX;
    unsigned char LY;
    unsigned char LYC;
    unsigned char DMA;
    unsigned char BGP;
    unsigned char OBP0;
    unsigned char OBP1;
    unsigned char WY;
    unsigned char WX;
    unsigned char P1;
    unsigned char SB;
    unsigned char SC;
    unsigned char IF;
    unsigned char IE;

};

struct count_s
{

    unsigned int lcd_count; /* LCD Timing */
    unsigned int div_count; /* Divider Register Counter */
    unsigned int tima_count; /* Timer Counter */
    unsigned int serial_count; /* Serial Counter */

};

struct gb_s
{

    unsigned char (*gb_rom_read)(struct gb_s*, const unsigned int addr);
    unsigned char (*gb_cart_ram_read)(struct gb_s*, const unsigned int addr);
    void (*gb_cart_ram_write)(struct gb_s*, const unsigned int addr, const unsigned char val);
    void (*gb_error)(struct gb_s*, const enum gb_error_e, const unsigned short val);
    unsigned char (*gb_serial_transfer)(struct gb_s*, const unsigned char);

    struct
    {

        unsigned int gb_halt : 1;
        unsigned int gb_ime : 1;
        unsigned int gb_bios_enable : 1;
        unsigned int gb_frame : 1;

        enum
        {

            LCD_HBLANK = 0,
            LCD_VBLANK = 1,
            LCD_SEARCH_OAM = 2,
            LCD_TRANSFER = 3

        } lcd_mode : 2;

    };

    unsigned char mbc;
    unsigned char cart_ram;
    unsigned short num_rom_banks;
    unsigned char num_ram_banks;
    unsigned char selected_rom_bank;
    unsigned char cart_ram_bank;
    unsigned char enable_cart_ram;
    unsigned char cart_mode_select;

    union
    {

        struct
        {

            unsigned char sec;
            unsigned char min;
            unsigned char hour;
            unsigned char yday;
            unsigned char high;

        } rtc_bits;

        unsigned char cart_rtc[5];

    };

    struct cpu_registers_s cpu_reg;
    struct gb_registers_s gb_reg;
    struct count_s counter;
    unsigned char wram[WRAM_SIZE];
    unsigned char vram[VRAM_SIZE];
    unsigned char hram[HRAM_SIZE];
    unsigned char oam[OAM_SIZE];

    struct
    {

        unsigned char bg_palette[4];
        unsigned char sp_palette[8];
        unsigned char window_clear;
        unsigned char WY;
        unsigned int frame_skip_count : 1;
        unsigned int interlace_count : 1;

    } display;

    struct
    {

        unsigned int interlace : 1;
        unsigned int frame_skip : 1;

        union
        {

            struct
            {

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
                unsigned int down : 1;
                unsigned int up : 1;
                unsigned int left : 1;
                unsigned int right : 1;
                unsigned int start : 1;
                unsigned int select : 1;
                unsigned int b : 1;
                unsigned int a : 1;
#else
                unsigned int a : 1;
                unsigned int b : 1;
                unsigned int select : 1;
                unsigned int start : 1;
                unsigned int right : 1;
                unsigned int left : 1;
                unsigned int up : 1;
                unsigned int down : 1;
#endif

            } joypad_bits;

            unsigned char joypad;

        };

    } direct;

};

void gb_tick_rtc(struct gb_s *gb);
void gb_set_rtc(struct gb_s *gb, const struct tm * const time);
enum gb_init_error_e gb_init(struct gb_s *gb, unsigned char (*gb_rom_read)(struct gb_s*, const unsigned int), unsigned char (*gb_cart_ram_read)(struct gb_s*, const unsigned int), void (*gb_cart_ram_write)(struct gb_s*, const unsigned int, const unsigned char), void (*gb_error)(struct gb_s*, const enum gb_error_e, const unsigned short));
void gb_reset(struct gb_s *gb);
void gb_run_frame(struct gb_s *gb);
void gb_init_serial(struct gb_s *gb, unsigned char (*gb_serial_transfer)(struct gb_s*, const unsigned char));
