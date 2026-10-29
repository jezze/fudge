// cx4 simulator for LakeSnes, (c) 2023 dink
// License: MIT

// Big Thanks:
//   Ikari, Nocash, Overload, Segher for all the CX4 research and documentation!
//   Rupert Carmichael for help with degree interpolation
//   Atan for pie recipe

// Notes:
//1: Since neither X2 or X3 relies on: this core doesn't emulate
//     2 of the cache peculiarities documented by ikari: Roll over to
//     second prg-cache on end of first & stop at end of second cache
//2: ~224-cycle/frame deficit
//     Where does it come from?  Possible cpu startup/stop latency?
//     Related to cache population (7f48)?  Somewhere else?
//3: Ugly sound on the X2 titlescreen is caused by the "DMA frame boundary"
//     issue, it can be solved (with duct-tape) by swapping the vblank order
//     in snes.cpp: snes_runFrame()

#include "port.h"
#include "snes.h"
#include "cx4.h"

#define DEBUG_STARTSTOP 0
#define DEBUG_CACHE     0
#define DEBUG_DMA		0

// CPU flags
enum {
	CC_Z = 1 << 0, // Zero
	CC_C = 1 << 1, // Carry
	CC_N = 1 << 2, // Negative
	CC_V = 1 << 3, // oVerflow
	CC_I = 1 << 4  // Interrupt
};

// BUS state
enum {
	B_IDLE  = 0,
	B_READ  = 1,
	B_WRITE = 2
};

// IRQ config
enum {
	IRQ_ACKNOWLEDGE = 1 << 0
};

#define set_flg(flg, f) do { cx4.cc = (cx4.cc & ~flg) | ((f) ? flg : 0); } while (0)
#define get_flg(flg) (!!(cx4.cc & flg))

#define set_Z(f) set_flg(CC_Z, f)
#define get_Z()  get_flg(CC_Z)
#define set_C(f) set_flg(CC_C, f)
#define get_C()  get_flg(CC_C)
#define set_N(f) set_flg(CC_N, f)
#define get_N()  get_flg(CC_N)
#define set_V(f) set_flg(CC_V, f)
#define get_V()  get_flg(CC_V)
#define set_I(f) set_flg(CC_I, f)
#define get_I()  get_flg(CC_I)
#define set_NZ(f) do { set_N(f & 0x800000);	set_Z(!f); } while (0)

#define set_A(f) do { cx4.A = (f) & 0xffffff; } while (0)
#define get_A() (cx4.A << ((0x10080100 >> (8 * sub_op)) & 0xff))

#define set_byte(var, data, offset) (var = (var & (~(0xff << (((offset) & 3) * 8)))) | (data << (((offset) & 3) * 8)))
#define get_byte(var, offset) (var >> ((offset) & 3) * 8)

#define signextend(f, frombits) ((int32_t)((f) << (32 - (frombits))) >> (32 - (frombits)))

#define struct_sizeto(type, member) offsetof(type, member) + sizeof(((type*)0)->member)

typedef struct Stack {
	uint32_t PC;
	uint32_t PB;
} Stack;

typedef struct CX4 {
	uint64_t cycles;
	uint64_t cycles_start;
	SnesSync sync;
	uint64_t suspend_timer;
	uint32_t running;

	uint32_t prg_base_address;
	uint16_t prg_startup_bank;
	uint8_t prg_startup_pc;
	uint8_t prg_cache_page;
	uint8_t prg_cache_lock;
	uint32_t prg_cache[2];
	uint16_t prg[2][0x100];
	int32_t prg_cache_timer;

	uint8_t PC;
	uint16_t PB;
	uint16_t PB_latch;
	uint8_t cc;
	uint32_t A;
	uint32_t SP;
	Stack stack[0x08];
	uint32_t reg[0x10];
	uint8_t vectors[0x20];
	uint8_t ram[0x400 * 3];

	uint64_t multiplier;

	// bus
	uint32_t bus_address;
	uint32_t bus_mode;
	uint32_t bus_data;
	int32_t bus_timer;

	// cpu registers (bus)
	uint32_t bus_address_pointer;
	uint32_t ram_address_pointer;
	uint32_t rom_data;
	uint32_t ram_data;

	uint8_t irqcfg;
	uint8_t unkcfg;
	uint8_t waitstate;

	uint32_t dma_source;
	uint32_t dma_dest;
	uint16_t dma_length;
	int32_t dma_timer;

	// - calculated @ init -
	int32_t struct_data_length;
	uint64_t sync_to;
	Snes *snes;
} CX4;

static CX4 cx4;

// the data rom, as upstream generated it: 0x800000 / i, 0x100000 * sqrt(i), then 0x1000000 * sin, 0x800000 / (pi / 2) * asin(i / 128.0),
// 0x10000 * tan and 0x1000000 * cos (0xffffff for i 0) in 90 / 128 degree steps
static const uint32_t cx4Rom[0x400] = {
	0xffffff, 0x800000, 0x400000, 0x2aaaaa, 0x200000, 0x199999, 0x155555, 0x124924,
	0x100000, 0x0e38e3, 0x0ccccc, 0x0ba2e8, 0x0aaaaa, 0x09d89d, 0x092492, 0x088888,
	0x080000, 0x078787, 0x071c71, 0x06bca1, 0x066666, 0x061861, 0x05d174, 0x0590b2,
	0x055555, 0x051eb8, 0x04ec4e, 0x04bda1, 0x049249, 0x0469ee, 0x044444, 0x042108,
	0x040000, 0x03e0f8, 0x03c3c3, 0x03a83a, 0x038e38, 0x03759f, 0x035e50, 0x034834,
	0x033333, 0x031f38, 0x030c30, 0x02fa0b, 0x02e8ba, 0x02d82d, 0x02c859, 0x02b931,
	0x02aaaa, 0x029cbc, 0x028f5c, 0x028282, 0x027627, 0x026a43, 0x025ed0, 0x0253c8,
	0x024924, 0x023ee0, 0x0234f7, 0x022b63, 0x022222, 0x02192e, 0x021084, 0x020820,
	0x020000, 0x01f81f, 0x01f07c, 0x01e913, 0x01e1e1, 0x01dae6, 0x01d41d, 0x01cd85,
	0x01c71c, 0x01c0e0, 0x01bacf, 0x01b4e8, 0x01af28, 0x01a98e, 0x01a41a, 0x019ec8,
	0x019999, 0x01948b, 0x018f9c, 0x018acb, 0x018618, 0x018181, 0x017d05, 0x0178a4,
	0x01745d, 0x01702e, 0x016c16, 0x016816, 0x01642c, 0x016058, 0x015c98, 0x0158ed,
	0x015555, 0x0151d0, 0x014e5e, 0x014afd, 0x0147ae, 0x01446f, 0x014141, 0x013e22,
	0x013b13, 0x013813, 0x013521, 0x01323e, 0x012f68, 0x012c9f, 0x0129e4, 0x012735,
	0x012492, 0x0121fb, 0x011f70, 0x011cf0, 0x011a7b, 0x011811, 0x0115b1, 0x01135c,
	0x011111, 0x010ecf, 0x010c97, 0x010a68, 0x010842, 0x010624, 0x010410, 0x010204,
	0x010000, 0x00fe03, 0x00fc0f, 0x00fa23, 0x00f83e, 0x00f660, 0x00f489, 0x00f2b9,
	0x00f0f0, 0x00ef2e, 0x00ed73, 0x00ebbd, 0x00ea0e, 0x00e865, 0x00e6c2, 0x00e525,
	0x00e38e, 0x00e1fc, 0x00e070, 0x00dee9, 0x00dd67, 0x00dbeb, 0x00da74, 0x00d901,
	0x00d794, 0x00d62b, 0x00d4c7, 0x00d368, 0x00d20d, 0x00d0b6, 0x00cf64, 0x00ce16,
	0x00cccc, 0x00cb87, 0x00ca45, 0x00c907, 0x00c7ce, 0x00c698, 0x00c565, 0x00c437,
	0x00c30c, 0x00c1e4, 0x00c0c0, 0x00bfa0, 0x00be82, 0x00bd69, 0x00bc52, 0x00bb3e,
	0x00ba2e, 0x00b921, 0x00b817, 0x00b70f, 0x00b60b, 0x00b509, 0x00b40b, 0x00b30f,
	0x00b216, 0x00b11f, 0x00b02c, 0x00af3a, 0x00ae4c, 0x00ad60, 0x00ac76, 0x00ab8f,
	0x00aaaa, 0x00a9c8, 0x00a8e8, 0x00a80a, 0x00a72f, 0x00a655, 0x00a57e, 0x00a4a9,
	0x00a3d7, 0x00a306, 0x00a237, 0x00a16b, 0x00a0a0, 0x009fd8, 0x009f11, 0x009e4c,
	0x009d89, 0x009cc8, 0x009c09, 0x009b4c, 0x009a90, 0x0099d7, 0x00991f, 0x009868,
	0x0097b4, 0x009701, 0x00964f, 0x0095a0, 0x0094f2, 0x009445, 0x00939a, 0x0092f1,
	0x009249, 0x0091a2, 0x0090fd, 0x00905a, 0x008fb8, 0x008f17, 0x008e78, 0x008dda,
	0x008d3d, 0x008ca2, 0x008c08, 0x008b70, 0x008ad8, 0x008a42, 0x0089ae, 0x00891a,
	0x008888, 0x0087f7, 0x008767, 0x0086d9, 0x00864b, 0x0085bf, 0x008534, 0x0084a9,
	0x008421, 0x008399, 0x008312, 0x00828c, 0x008208, 0x008184, 0x008102, 0x008080,
	0x000000, 0x100000, 0x16a09e, 0x1bb67a, 0x200000, 0x23c6ef, 0x27311c, 0x2a54ff,
	0x2d413c, 0x300000, 0x3298b0, 0x3510e5, 0x376cf5, 0x39b056, 0x3bddd4, 0x3df7bd,
	0x400000, 0x41f83d, 0x43e1db, 0x45be0c, 0x478dde, 0x49523a, 0x4b0bf1, 0x4cbbb9,
	0x4e6238, 0x500000, 0x519595, 0x532370, 0x54a9fe, 0x5629a2, 0x57a2b7, 0x591590,
	0x5a8279, 0x5be9ba, 0x5d4b94, 0x5ea843, 0x600000, 0x6152fe, 0x62a170, 0x63eb83,
	0x653160, 0x667332, 0x67b11d, 0x68eb44, 0x6a21ca, 0x6b54cd, 0x6c846c, 0x6db0c2,
	0x6ed9eb, 0x700000, 0x712318, 0x72434a, 0x7360ad, 0x747b54, 0x759354, 0x76a8bf,
	0x77bba8, 0x78cc1f, 0x79da34, 0x7ae5f9, 0x7bef7a, 0x7cf6c8, 0x7dfbef, 0x7efefd,
	0x800000, 0x80ff01, 0x81fc0f, 0x82f734, 0x83f07b, 0x84e7ee, 0x85dd98, 0x86d182,
	0x87c3b6, 0x88b43d, 0x89a31f, 0x8a9066, 0x8b7c19, 0x8c6641, 0x8d4ee4, 0x8e360b,
	0x8f1bbc, 0x900000, 0x90e2db, 0x91c456, 0x92a475, 0x938341, 0x9460bd, 0x953cf1,
	0x9617e2, 0x96f196, 0x97ca11, 0x98a159, 0x997773, 0x9a4c64, 0x9b2031, 0x9bf2de,
	0x9cc470, 0x9d94eb, 0x9e6454, 0x9f32af, 0xa00000, 0xa0cc4a, 0xa19792, 0xa261dc,
	0xa32b2a, 0xa3f382, 0xa4bae6, 0xa5815a, 0xa646e1, 0xa70b7e, 0xa7cf35, 0xa89209,
	0xa953fd, 0xaa1513, 0xaad550, 0xab94b4, 0xac5345, 0xad1103, 0xadcdf2, 0xae8a15,
	0xaf456e, 0xb00000, 0xb0b9cc, 0xb172d6, 0xb22b20, 0xb2e2ac, 0xb3997c, 0xb44f93,
	0xb504f3, 0xb5b99d, 0xb66d95, 0xb720dc, 0xb7d375, 0xb88560, 0xb936a0, 0xb9e738,
	0xba9728, 0xbb4673, 0xbbf51a, 0xbca320, 0xbd5086, 0xbdfd4e, 0xbea979, 0xbf5509,
	0xc00000, 0xc0aa5f, 0xc15428, 0xc1fd5c, 0xc2a5fd, 0xc34e0d, 0xc3f58c, 0xc49c7d,
	0xc542e1, 0xc5e8b8, 0xc68e05, 0xc732c9, 0xc7d706, 0xc87abb, 0xc91deb, 0xc9c098,
	0xca62c1, 0xcb0469, 0xcba591, 0xcc463a, 0xcce664, 0xcd8612, 0xce2544, 0xcec3fc,
	0xcf623a, 0xd00000, 0xd09d4e, 0xd13a26, 0xd1d689, 0xd27277, 0xd30df3, 0xd3a8fc,
	0xd44394, 0xd4ddbc, 0xd57774, 0xd610be, 0xd6a99b, 0xd7420b, 0xd7da0f, 0xd871a9,
	0xd908d8, 0xd99f9f, 0xda35fe, 0xdacbf5, 0xdb6185, 0xdbf6b0, 0xdc8b76, 0xdd1fd8,
	0xddb3d7, 0xde4773, 0xdedaad, 0xdf6d86, 0xe00000, 0xe09219, 0xe123d4, 0xe1b530,
	0xe24630, 0xe2d6d2, 0xe36719, 0xe3f704, 0xe48694, 0xe515cb, 0xe5a4a8, 0xe6332d,
	0xe6c15a, 0xe74f2f, 0xe7dcad, 0xe869d6, 0xe8f6a9, 0xe98326, 0xea0f50, 0xea9b26,
	0xeb26a8, 0xebb1d9, 0xec3cb7, 0xecc743, 0xed517f, 0xeddb6a, 0xee6506, 0xeeee52,
	0xef7750, 0xf00000, 0xf08861, 0xf11076, 0xf1983e, 0xf21fba, 0xf2a6ea, 0xf32dcf,
	0xf3b469, 0xf43ab9, 0xf4c0c0, 0xf5467d, 0xf5cbf2, 0xf6511e, 0xf6d602, 0xf75a9f,
	0xf7def5, 0xf86305, 0xf8e6ce, 0xf96a52, 0xf9ed90, 0xfa708a, 0xfaf33f, 0xfb75b1,
	0xfbf7df, 0xfc79ca, 0xfcfb72, 0xfd7cd8, 0xfdfdfb, 0xfe7ede, 0xfeff7f, 0xff7fdf,
	0x000000, 0x03243a, 0x064855, 0x096c32, 0x0c8fb2, 0x0fb2b7, 0x12d520, 0x15f6d0,
	0x1917a6, 0x1c3785, 0x1f564e, 0x2273e1, 0x259020, 0x28aaed, 0x2bc428, 0x2edbb3,
	0x31f170, 0x350540, 0x381704, 0x3b269f, 0x3e33f2, 0x413ee0, 0x444749, 0x474d10,
	0x4a5018, 0x4d5043, 0x504d72, 0x534789, 0x563e69, 0x5931f7, 0x5c2214, 0x5f0ea4,
	0x61f78a, 0x64dca9, 0x67bde5, 0x6a9b20, 0x6d7440, 0x704927, 0x7319ba, 0x75e5dd,
	0x78ad74, 0x7b7065, 0x7e2e93, 0x80e7e4, 0x839c3c, 0x864b82, 0x88f59a, 0x8b9a6b,
	0x8e39d9, 0x90d3cc, 0x93682a, 0x95f6d9, 0x987fbf, 0x9b02c5, 0x9d7fd1, 0x9ff6ca,
	0xa26799, 0xa4d224, 0xa73655, 0xa99414, 0xabeb49, 0xae3bdd, 0xb085ba, 0xb2c8c9,
	0xb504f3, 0xb73a22, 0xb96841, 0xbb8f3a, 0xbdaef9, 0xbfc767, 0xc1d870, 0xc3e200,
	0xc5e403, 0xc7de65, 0xc9d112, 0xcbbbf7, 0xcd9f02, 0xcf7a1f, 0xd14d3d, 0xd31848,
	0xd4db31, 0xd695e4, 0xd84852, 0xd9f269, 0xdb941a, 0xdd2d53, 0xdebe05, 0xe04621,
	0xe1c597, 0xe33c59, 0xe4aa59, 0xe60f87, 0xe76bd7, 0xe8bf3b, 0xea09a6, 0xeb4b0b,
	0xec835e, 0xedb293, 0xeed89d, 0xeff573, 0xf10908, 0xf21352, 0xf31447, 0xf40bdd,
	0xf4fa0a, 0xf5dec6, 0xf6ba07, 0xf78bc5, 0xf853f7, 0xf91297, 0xf9c79d, 0xfa7301,
	0xfb14be, 0xfbaccd, 0xfc3b27, 0xfcbfc9, 0xfd3aab, 0xfdabcb, 0xfe1323, 0xfe70af,
	0xfec46d, 0xff0e57, 0xff4e6d, 0xff84ab, 0xffb10f, 0xffd397, 0xffec43, 0xfffb10,
	0x000000, 0x00a2f9, 0x0145f6, 0x01e8f8, 0x028c01, 0x032f14, 0x03d234, 0x047564,
	0x0518a5, 0x05bbfb, 0x065f68, 0x0702ef, 0x07a692, 0x084a54, 0x08ee38, 0x099240,
	0x0a366e, 0x0adac7, 0x0b7f4c, 0x0c2401, 0x0cc8e7, 0x0d6e02, 0x0e1355, 0x0eb8e3,
	0x0f5eae, 0x1004b9, 0x10ab08, 0x11519e, 0x11f87d, 0x129fa9, 0x134725, 0x13eef4,
	0x149719, 0x153f99, 0x15e875, 0x1691b2, 0x173b53, 0x17e55c, 0x188fd1, 0x193ab4,
	0x19e60a, 0x1a91d8, 0x1b3e20, 0x1beae7, 0x1c9831, 0x1d4602, 0x1df45f, 0x1ea34c,
	0x1f52ce, 0x2002ea, 0x20b3a3, 0x216500, 0x221705, 0x22c9b8, 0x237d1e, 0x24313c,
	0x24e618, 0x259bb9, 0x265224, 0x27095f, 0x27c171, 0x287a61, 0x293436, 0x29eef6,
	0x2aaaaa, 0x2b6759, 0x2c250a, 0x2ce3c7, 0x2da398, 0x2e6485, 0x2f2699, 0x2fe9dc,
	0x30ae59, 0x31741b, 0x323b2c, 0x330398, 0x33cd6b, 0x3498b1, 0x356578, 0x3633ce,
	0x3703c1, 0x37d560, 0x38a8bb, 0x397de4, 0x3a54ec, 0x3b2de6, 0x3c08e6, 0x3ce601,
	0x3dc54d, 0x3ea6e3, 0x3f8adc, 0x407152, 0x415a62, 0x42462c, 0x4334d0, 0x442671,
	0x451b37, 0x46134a, 0x470ed6, 0x480e0c, 0x491120, 0x4a184c, 0x4b23cd, 0x4c33ea,
	0x4d48ec, 0x4e6327, 0x4f82f9, 0x50a8c9, 0x51d50a, 0x53083f, 0x5442fc, 0x5585ea,
	0x56d1cc, 0x582782, 0x598815, 0x5af4bc, 0x5c6eed, 0x5df86c, 0x5f9369, 0x6142a3,
	0x6309a5, 0x64ed1e, 0x66f381, 0x692617, 0x6b9322, 0x6e52a5, 0x71937c, 0x75ceb4,
	0x000000, 0x000324, 0x000648, 0x00096d, 0x000c93, 0x000fba, 0x0012e2, 0x00160b,
	0x001936, 0x001c63, 0x001f93, 0x0022c4, 0x0025f9, 0x002930, 0x002c6b, 0x002fa9,
	0x0032eb, 0x003632, 0x00397c, 0x003ccb, 0x00401f, 0x004379, 0x0046d8, 0x004a3d,
	0x004da8, 0x005119, 0x005492, 0x005811, 0x005b99, 0x005f28, 0x0062c0, 0x006660,
	0x006a09, 0x006dbc, 0x00717a, 0x007541, 0x007914, 0x007cf2, 0x0080dc, 0x0084d2,
	0x0088d5, 0x008ce6, 0x009105, 0x009533, 0x009970, 0x009dbe, 0x00a21c, 0x00a68b,
	0x00ab0d, 0x00afa2, 0x00b44b, 0x00b909, 0x00bddc, 0x00c2c6, 0x00c7c8, 0x00cce3,
	0x00d218, 0x00d767, 0x00dcd3, 0x00e25d, 0x00e806, 0x00edcf, 0x00f3bb, 0x00f9ca,
	0x010000, 0x01065c, 0x010ce2, 0x011394, 0x011a73, 0x012183, 0x0128c6, 0x01303e,
	0x0137ef, 0x013fdc, 0x014808, 0x015077, 0x01592d, 0x01622d, 0x016b7d, 0x017522,
	0x017f21, 0x018980, 0x019444, 0x019f76, 0x01ab1c, 0x01b73e, 0x01c3e7, 0x01d11f,
	0x01def1, 0x01ed69, 0x01fc95, 0x020c83, 0x021d44, 0x022ee9, 0x024186, 0x025533,
	0x026a09, 0x028025, 0x0297a7, 0x02b0b5, 0x02cb78, 0x02e823, 0x0306ec, 0x032815,
	0x034beb, 0x0372c6, 0x039d10, 0x03cb47, 0x03fe02, 0x0435f7, 0x047405, 0x04b93f,
	0x0506ff, 0x055ef9, 0x05c35d, 0x063709, 0x06bdcf, 0x075ce6, 0x081b97, 0x09046d,
	0x0a2736, 0x0b9cc6, 0x0d8e81, 0x1046e9, 0x145aff, 0x1b2671, 0x28bc48, 0x517bb5,
	0xffffff, 0xfffb10, 0xffec43, 0xffd397, 0xffb10f, 0xff84ab, 0xff4e6d, 0xff0e57,
	0xfec46d, 0xfe70af, 0xfe1323, 0xfdabcb, 0xfd3aab, 0xfcbfc9, 0xfc3b27, 0xfbaccd,
	0xfb14be, 0xfa7301, 0xf9c79d, 0xf91297, 0xf853f7, 0xf78bc5, 0xf6ba07, 0xf5dec6,
	0xf4fa0a, 0xf40bdd, 0xf31447, 0xf21352, 0xf10908, 0xeff573, 0xeed89d, 0xedb293,
	0xec835e, 0xeb4b0b, 0xea09a6, 0xe8bf3b, 0xe76bd7, 0xe60f87, 0xe4aa59, 0xe33c59,
	0xe1c597, 0xe04621, 0xdebe05, 0xdd2d53, 0xdb941a, 0xd9f269, 0xd84852, 0xd695e4,
	0xd4db31, 0xd31848, 0xd14d3d, 0xcf7a1f, 0xcd9f02, 0xcbbbf7, 0xc9d112, 0xc7de65,
	0xc5e403, 0xc3e200, 0xc1d870, 0xbfc767, 0xbdaef9, 0xbb8f3a, 0xb96841, 0xb73a22,
	0xb504f3, 0xb2c8c9, 0xb085ba, 0xae3bdd, 0xabeb49, 0xa99414, 0xa73655, 0xa4d224,
	0xa26799, 0x9ff6ca, 0x9d7fd1, 0x9b02c5, 0x987fbf, 0x95f6d9, 0x93682a, 0x90d3cc,
	0x8e39d9, 0x8b9a6b, 0x88f59a, 0x864b82, 0x839c3c, 0x80e7e4, 0x7e2e93, 0x7b7065,
	0x78ad74, 0x75e5dd, 0x7319ba, 0x704927, 0x6d7440, 0x6a9b20, 0x67bde5, 0x64dca9,
	0x61f78a, 0x5f0ea4, 0x5c2214, 0x5931f7, 0x563e69, 0x534789, 0x504d72, 0x4d5043,
	0x4a5018, 0x474d10, 0x444749, 0x413ee0, 0x3e33f2, 0x3b269f, 0x381704, 0x350540,
	0x31f170, 0x2edbb3, 0x2bc428, 0x28aaed, 0x259020, 0x2273e1, 0x1f564e, 0x1c3785,
	0x1917a6, 0x15f6d0, 0x12d520, 0x0fb2b7, 0x0c8fb2, 0x096c32, 0x064855, 0x03243a,
};

void cx4_init(void *mem)
{
	cx4.snes = (Snes *)mem;

	cx4.struct_data_length = struct_sizeto(CX4, dma_timer);
}

void cx4_reset()
{
	memset(&cx4, 0, cx4.struct_data_length);
	cx4.A = 0xffffff;
	cx4.cc = 0x00;
	cx4.running = 0;
	cx4.unkcfg = 1;
	cx4.waitstate = 0x33;
	cx4.bus_mode = B_IDLE;
}

#define CACHE_PAGE 0x100

static uint32_t resolve_cache_address()
{
	return cx4.prg_base_address + cx4.PB * (CACHE_PAGE << 1);
}

static int find_cache(uint32_t address)
{
	for (int i = 0; i < 2; i++) {
		if (cx4.prg_cache[i] == address) {
			return i;
		}
	}
	return -1;
}

static void populate_cache(uint32_t address)
{
	cx4.prg_cache_timer = 224; // what is the source of this?  (re: note at top of file)

	if (cx4.prg_cache[cx4.prg_cache_page] == address) return;
#if 0
	int temp = -1;
	if ((temp = find_cache(address)) != -1) {
		//bprintf(0, _T("populate cache is already cached!  %x\n"), address);
		cx4.prg_cache_page = temp;
		return;
	}
#endif

#if DEBUG_CACHE
	//bprintf(0, _T("caching bank  %x  @  cache pg.  %x  (prev: %x)\n"), cx4.PB, cx4.prg_cache_page, cx4.prg_cache[cx4.prg_cache_page]);
#endif

	cx4.prg_cache[cx4.prg_cache_page] = address;

	for (int i = 0; i < CACHE_PAGE; i++) {
		uint16_t lo = snes_read(cx4.snes, address++);
		cx4.prg[cx4.prg_cache_page][i] = lo | (snes_read(cx4.snes, address++) << 8);
	}

	cx4.prg_cache_timer += ((cx4.waitstate & 0x07) * CACHE_PAGE) * 2;
#if DEBUG_CACHE
	//bprintf(0, _T("cache loaded, cycles %d\n"), cx4.prg_cache_timer);
#endif
}

static void do_cache()
{
	int new_page;

#if DEBUG_CACHE
	//bprintf(0, _T("cache list: %x  %x\n"), cx4.prg_cache[0], cx4.prg_cache[1]);
#endif

	// is our page cached?
	if ((new_page = find_cache(resolve_cache_address())) != -1) {
		//bprintf(0, _T("our page is already cached, yay.\n"));
		cx4.prg_cache_page = new_page;
		return;
	} else {
		// not cached, go to next slot
		cx4.prg_cache_page = (cx4.prg_cache_page + 1) & 1;
#if 0
		// Locked page issue:
		// (X2) after boss battle, on the "You got ..." screen: the blue raster box
		// flickers in for a frame or 2 before it fades in, because:
		// Cache pg 0 is locked with bank 0, pg1 is unlocked
		// pg 1 has to swap between pb 2 and 9 about 100 times per frame, and
		// this eats a lot of cycles!

		// can we use this slot?
		if (cx4.prg_cache_lock & (1 << cx4.prg_cache_page)) {
			//bprintf(0, _T("-> page %x is locked (with %x) ...\n"), cx4.prg_cache_page, cx4.prg_cache[cx4.prg_cache_page]);
			cx4.prg_cache_page = (cx4.prg_cache_page + 1) & 1;
			// how about the other one?
			if (cx4.prg_cache_lock & (1 << cx4.prg_cache_page)) {
				//bprintf(0, _T("CX4: we can't cache, operations terminated.\n"));
				cx4.running = 0;
				return; // not cached, can't cache. uhoh!
			}
		}
#endif
	}

	populate_cache(resolve_cache_address());
}

static void cycle_advance(int32_t cyc)
{
	if (cx4.bus_timer) {
		cx4.bus_timer -= cyc;

		if (cx4.bus_timer < 1) {
			switch (cx4.bus_mode) {
				case B_READ: cx4.bus_data = snes_read(cx4.snes, cx4.bus_address); break;
				case B_WRITE: snes_write(cx4.snes, cx4.bus_address, cx4.bus_data); break;
			}
			cx4.bus_mode = B_IDLE;
			cx4.bus_timer = 0;
		}
	}

	cx4.cycles += cyc;
}

static uint16_t fetch()
{
#if 0
	// debug: bypass cache
	uint16_t opcode = 0;
	uint32_t address = (cx4.prg_base_address + (cx4.PB * (CACHE_PAGE << 1)) + (cx4.PC << 1)) & 0xffffff;
	opcode  = snes_read(cx4.snes, address++);
	opcode |= snes_read(cx4.snes, address++) << 8;
#else
	const uint16_t opcode = cx4.prg[cx4.prg_cache_page][cx4.PC];
#endif
	cx4.PC++;
	if (cx4.PC == 0) {
		//bprintf(0, _T("PC == 0!  PB / Next:  %x  %x\n"), cx4.PB, cx4.PB_latch);
		cx4.PB = cx4.PB_latch;

		do_cache();
	}

	cycle_advance(1);

	return opcode;
}

#define is_internal_ram(a) ((a & 0x40e000) == 0x6000)

static uint32_t get_waitstate(uint32_t address)
{
	// assumptions: waitstate is always the same for cart ROM and RAM
	// .waitstate 0x33 (boot) 0x44 (set by X2/X3)
	return (is_internal_ram(address)) ? 0 : (cx4.waitstate & 0x07);
}

static void do_dma()
{
	uint32_t dest = cx4.dma_dest;
	uint32_t source = cx4.dma_source;

	uint32_t dest_cyc = get_waitstate(dest);
	uint32_t source_cyc = get_waitstate(source);
#if DEBUG_DMA
	//bprintf(0, _T("dma\tsrc/dest/len:  %x  %x  %x\n"), source, dest, cx4.dma_length);
#endif
	for (int i = 0; i < cx4.dma_length; i++) {
		snes_write(cx4.snes, dest++, snes_read(cx4.snes, source++));
	}

	cx4.dma_timer = cx4.dma_length * (1 + dest_cyc + source_cyc);
#if DEBUG_DMA
	//bprintf(0, _T("dma end, cycles %d\n"), cx4.dma_timer);
#endif
}

uint8_t cx4_read(uint32_t address)
{
	cx4_run(); // get up-to-date

	if ((address & 0xfff) < 0xc00) {
		return cx4.ram[address & 0xfff];
	}

	if (address >= 0x7f80 && (address & 0x3f) <= 0x2f) {
		address &= 0x3f;
		return get_byte(cx4.reg[address / 3], address % 3);
	}

	switch (address) {
		case 0x7f40: return (cx4.dma_source >> 0) & 0xff;
		case 0x7f41: return (cx4.dma_source >> 8) & 0xff;
		case 0x7f42: return (cx4.dma_source >> 16) & 0xff;
		case 0x7f43: return (cx4.dma_length >> 0) & 0xff;
		case 0x7f44: return (cx4.dma_length >> 8) & 0xff;
		case 0x7f45: return (cx4.dma_dest >> 0) & 0xff;
		case 0x7f46: return (cx4.dma_dest >> 8) & 0xff;
		case 0x7f47: return (cx4.dma_dest >> 16) & 0xff;
		case 0x7f48: return cx4.prg_cache_page;
		case 0x7f49: return (cx4.prg_base_address >> 0) & 0xff;
		case 0x7f4a: return (cx4.prg_base_address >> 8) & 0xff;
		case 0x7f4b: return (cx4.prg_base_address >> 16) & 0xff;
		case 0x7f4c: return cx4.prg_cache_lock;
		case 0x7f4d: return (cx4.prg_startup_bank >> 0) & 0xff;
		case 0x7f4e: return (cx4.prg_startup_bank >> 8) & 0xff;
		case 0x7f4f: return cx4.prg_startup_pc;
		case 0x7f50: return cx4.waitstate;
		case 0x7f51: return cx4.irqcfg;
		case 0x7f52: return cx4.unkcfg;
		case 0x7f53: case 0x7f54: case 0x7f55: case 0x7f56:
		case 0x7f57: case 0x7f59: case 0x7f5b: case 0x7f5c:
		case 0x7f5d: case 0x7f5e: case 0x7f5f: {
			// cx4 status (address 58 & 5a not mapped, returns 0)
			// tr.. ..is
			//           t.transfer in-progress (bus, cache, dma)
			//           r.running or transfer in-progress
			//           i.irq flag
			//           s.processor suspended
			const int transfer = (cx4.prg_cache_timer > 0) || (cx4.bus_timer > 0) || (cx4.dma_timer > 0);
			const int running = transfer || cx4.running;
			const uint8_t res = (transfer << 7) | (running << 6) | (get_I() << 1) | (cx4.suspend_timer != 0);
			return res;
		}
		case 0x7f60: case 0x7f61: case 0x7f62: case 0x7f63:
		case 0x7f64: case 0x7f65: case 0x7f66: case 0x7f67:
		case 0x7f68: case 0x7f69: case 0x7f6a: case 0x7f6b:
		case 0x7f6c: case 0x7f6d: case 0x7f6e: case 0x7f6f:
		case 0x7f70: case 0x7f71: case 0x7f72: case 0x7f73:
		case 0x7f74: case 0x7f75: case 0x7f76: case 0x7f77:
		case 0x7f78: case 0x7f79: case 0x7f7a: case 0x7f7b:
		case 0x7f7c: case 0x7f7d: case 0x7f7e: case 0x7f7f:
			// this provides the vector table for when the cx4 chip disconnects
			// the rom(s) from the bus during cpu/transfer operations
			return cx4.vectors[address & 0x1f];
	}

	return 0;
}

void cx4_write(uint32_t address, uint8_t data)
{
	cx4_run();

	if ((address & 0xfff) < 0xc00) {
		cx4.ram[address & 0xfff] = data;
		return;
	}

	if (address >= 0x7f80 && (address & 0x3f) <= 0x2f) {
		address &= 0x3f;
		set_byte(cx4.reg[address / 3], data, address % 3);
		return;
	}

	switch (address) {
		case 0x7f40: cx4.dma_source = (cx4.dma_source & 0xffff00) | (data << 0); break;
		case 0x7f41: cx4.dma_source = (cx4.dma_source & 0xff00ff) | (data << 8); break;
		case 0x7f42: cx4.dma_source = (cx4.dma_source & 0x00ffff) | (data << 16); break;
		case 0x7f43: cx4.dma_length = (cx4.dma_length & 0xff00) | (data << 0); break;
		case 0x7f44: cx4.dma_length = (cx4.dma_length & 0x00ff) | (data << 8); break;
		case 0x7f45: cx4.dma_dest = (cx4.dma_dest & 0xffff00) | (data << 0); break;
		case 0x7f46: cx4.dma_dest = (cx4.dma_dest & 0xff00ff) | (data << 8); break;
		case 0x7f47: cx4.dma_dest = (cx4.dma_dest & 0x00ffff) | (data << 16); do_dma(); break;
		case 0x7f48: cx4.prg_cache_page = data & 0x01; populate_cache(resolve_cache_address()); break;
		case 0x7f49: cx4.prg_base_address = (cx4.prg_base_address & 0xffff00) | (data << 0); break;
		case 0x7f4a: cx4.prg_base_address = (cx4.prg_base_address & 0xff00ff) | (data << 8); break;
		case 0x7f4b: cx4.prg_base_address = (cx4.prg_base_address & 0x00ffff) | (data << 16); break;
		case 0x7f4c: cx4.prg_cache_lock = data & 0x03; break;
		case 0x7f4d: cx4.prg_startup_bank = (cx4.prg_startup_bank & 0xff00) | data; break;
		case 0x7f4e: cx4.prg_startup_bank = (cx4.prg_startup_bank & 0x00ff) | ((data & 0x7f) << 8); break;
		case 0x7f4f:
			cx4.prg_startup_pc = data;
			if (cx4.running == 0) {
				cx4.PB = cx4.prg_startup_bank;
				cx4.PC = cx4.prg_startup_pc;
				cx4.running = 1;
				cx4.cycles_start = cx4.cycles;
#if DEBUG_STARTSTOP
				//bprintf(0, _T("cx4 start @ %I64u  -  "), cx4.cycles);
				//bprintf(0, _T("cache PB: %x\tPC: %x\tcache: %x\n"), cx4.PB, cx4.PC, resolve_cache_address());
#endif
				do_cache();
			}
			break;
		case 0x7f50: cx4.waitstate = data & 0x77; break; // oooo aaaa  o.rom, a.ram
		case 0x7f51:
			cx4.irqcfg = data & 0x01;
			if (cx4.irqcfg & IRQ_ACKNOWLEDGE) {
				cpu_setIrq(cx4.snes->cpu, false);
				set_I(0);
			}
			break;
		case 0x7f52: cx4.unkcfg = data & 0x01; break; // this is up for debate, previously thought to en/disable 2nd rom chip on certain carts
		case 0x7f53: cx4.running = 0; break;
		case 0x7f55: case 0x7f56: case 0x7f57: case 0x7f58:
		case 0x7f59: case 0x7f5a: case 0x7f5b: case 0x7f5c: {
			const int32_t offset = (address - 0x7f55);
			cx4.suspend_timer = (offset == 0) ? -1 : (offset << 5);
			break;
		}
		case 0x7f5d: cx4.suspend_timer = 0; break;
		case 0x7f5e: set_I(0); break;
		case 0x7f60: case 0x7f61: case 0x7f62: case 0x7f63:
		case 0x7f64: case 0x7f65: case 0x7f66: case 0x7f67:
		case 0x7f68: case 0x7f69: case 0x7f6a: case 0x7f6b:
		case 0x7f6c: case 0x7f6d: case 0x7f6e: case 0x7f6f:
		case 0x7f70: case 0x7f71: case 0x7f72: case 0x7f73:
		case 0x7f74: case 0x7f75: case 0x7f76: case 0x7f77:
		case 0x7f78: case 0x7f79: case 0x7f7a: case 0x7f7b:
		case 0x7f7c: case 0x7f7d: case 0x7f7e: case 0x7f7f:
			cx4.vectors[address & 0x1f] = data; break;
	}
}

// special function (purpose?) registers

static uint32_t get_sfr(uint8_t address)
{
	switch (address & 0x7f) {
		case 0x01: return (cx4.multiplier >> 24) & 0xffffff;
		case 0x02: return (cx4.multiplier >>  0) & 0xffffff;
		case 0x03: return cx4.bus_data;
		case 0x08: return cx4.rom_data;
		case 0x0c: return cx4.ram_data;
		case 0x13: return cx4.bus_address_pointer;
		case 0x1c: return cx4.ram_address_pointer;
		case 0x20: return cx4.PC;
		case 0x28: return cx4.PB_latch;
		case 0x2e: // rom
		case 0x2f: // ram
			cx4.bus_timer = ((cx4.waitstate >> ((~address & 1) << 2)) & 0x07) + 1;
			cx4.bus_address = cx4.bus_address_pointer;
			cx4.bus_mode = B_READ;
			return 0;
		case 0x50: return 0x000000;
		case 0x51: return 0xffffff;
		case 0x52: return 0x00ff00;
		case 0x53: return 0xff0000;
		case 0x54: return 0x00ffff;
		case 0x55: return 0xffff00;
		case 0x56: return 0x800000;
		case 0x57: return 0x7fffff;
		case 0x58: return 0x008000;
		case 0x59: return 0x007fff;
		case 0x5a: return 0xff7fff;
		case 0x5b: return 0xffff7f;
		case 0x5c: return 0x010000;
		case 0x5d: return 0xfeffff;
		case 0x5e: return 0x000100;
		case 0x5f: return 0x00feff;
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			return cx4.reg[address & 0x0f];
	}

	return 0;
}

static void set_sfr(uint8_t address, uint32_t data)
{
	switch (address & 0x7f) {
		case 0x01: cx4.multiplier = (cx4.multiplier & 0x000000ffffff) | ((uint64_t)data << 24); break;
		case 0x02: cx4.multiplier = (cx4.multiplier & 0xffffff000000) | ((uint64_t)data <<  0); break;
		case 0x03: cx4.bus_data = data; break;
		case 0x08: cx4.rom_data = data; break;
		case 0x0c: cx4.ram_data = data; break;
		case 0x13: cx4.bus_address_pointer = data; break;
		case 0x1c: cx4.ram_address_pointer = data; break;
		case 0x20: cx4.PC = data; break;
		case 0x28: cx4.PB_latch = (data & 0x7fff); break;
		case 0x2e: // rom
		case 0x2f: // ram
			cx4.bus_timer = ((cx4.waitstate >> ((~address & 1) << 2)) & 0x07) + 1;
			cx4.bus_address = cx4.bus_address_pointer;
			cx4.bus_mode = B_WRITE;
			break;
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			cx4.reg[address & 0x0f] = data; break;
	}
}

static void jmpjsr(bool is_jsr, bool take, uint8_t page, uint8_t address) {
	if (take) {
		if (is_jsr) {
			cx4.stack[cx4.SP].PC = cx4.PC;
			cx4.stack[cx4.SP].PB = cx4.PB;
			cx4.SP = (cx4.SP + 1) & 0x07;
		}
		if (page) {
			cx4.PB = cx4.PB_latch;
			do_cache();
		}
		cx4.PC = address;
		cycle_advance(2);
	}
}

static uint32_t add(uint32_t a1, uint32_t a2)
{
	const uint32_t sum = a1 + a2;

	set_C(sum & 0xff000000);
	set_V(((a1 ^ sum) & (a2 ^ sum)) & 0x800000);
	set_N(sum & 0x800000);
	set_Z(!(sum & 0xffffff));

	return sum & 0xffffff;
}

static uint32_t sub(uint32_t m, uint32_t s)
{
	const int32_t diff = m - s;

	set_C(!(diff < 0));
	set_V(((m ^ diff) & (s ^ diff)) & 0x800000);
	set_NZ(diff);

	return diff & 0xffffff;
}

#define DIRECT_IMM 0x0400
#define get_immed() ((opcode & DIRECT_IMM) ? immed : get_sfr(immed))

static void run_insn()
{
	const uint16_t opcode = fetch();
	const uint8_t sub_op = (opcode & 0x0300) >> 8;
	const uint8_t immed  = (opcode & 0x00ff) >> 0;
	uint32_t temp = 0;

	switch (opcode & 0xfc00) {
		case 0x0000: // nop
			break;

		case 0x0800: // jmp page,pc
			jmpjsr(false, true, sub_op, immed); break;
		case 0x0c00: // jmp if flag,page,pc
			jmpjsr(false, get_Z(), sub_op, immed); break;
		case 0x1000:
			jmpjsr(false, get_C(), sub_op, immed); break;
		case 0x1400:
			jmpjsr(false, get_N(), sub_op, immed); break;
		case 0x1800:
			jmpjsr(false, get_V(), sub_op, immed); break;
		case 0x2800: // jsr page,pc
			jmpjsr(true, true, sub_op, immed); break;
		case 0x2c00: // jsr if flag,page,pc
			jmpjsr(true, get_Z(), sub_op, immed); break;
		case 0x3000:
			jmpjsr(true, get_C(), sub_op, immed); break;
		case 0x3400:
			jmpjsr(true, get_N(), sub_op, immed); break;
		case 0x3800:
			jmpjsr(true, get_V(), sub_op, immed); break;

		case 0x3c00: // return
			cx4.SP = (cx4.SP - 1) & 0x07;
			cx4.PC = cx4.stack[cx4.SP].PC;
			cx4.PB = cx4.stack[cx4.SP].PB;
			do_cache();
			cycle_advance(2);
			break;

		case 0x1c00: // finish/execute bus transfer
			cycle_advance(cx4.bus_timer);
			break;

		case 0x2400: // skip cc,imm
			if (!!(cx4.cc & (1 << ((0x13 >> sub_op) & 3))) == immed) { // note: re-indexes processor flags to match order of sub_op [O,C,Z,N]
				fetch();
			}
			break;

		case 0x4000: // inc bus address
			cx4.bus_address_pointer = (cx4.bus_address_pointer + 1) & 0xffffff;
			break;

		case 0x4800: // cmp immed,A
		case 0x4c00:
			sub(get_immed(), get_A());
			break;

		case 0x5000: // cmp A,immed
		case 0x5400:
			sub(get_A(), get_immed());
			break;

		case 0x5800: // sign_extend A[?,8,16,? bit]
			cx4.A = signextend(cx4.A, sub_op << 3) & 0xffffff;
			set_NZ(cx4.A);
			break;

		case 0x6000: // mov x,immed
		case 0x6400:
			switch (sub_op) {
				case 0: cx4.A = get_immed(); break;
				case 1: cx4.bus_data = get_immed(); break;
				case 2: cx4.bus_address_pointer = get_immed(); break;
				case 3: cx4.PB_latch = get_immed() & 0x7fff; break;
			}
			break;

		case 0xe000: // mov sfr[imm],x
			switch (sub_op) {
				case 0: set_sfr(immed, cx4.A); break;
				case 1: set_sfr(immed, cx4.bus_data); break;
				case 2: set_sfr(immed, cx4.bus_address_pointer); break;
				case 3: set_sfr(immed, cx4.PB_latch); break;
			}
			break;

		case 0x6800: // RDRAM subop,A
			temp = cx4.A & 0xfff;
			if (temp < 0xc00) {
				set_byte(cx4.ram_data, cx4.ram[temp], sub_op);
			}
			break;
		case 0x6c00: // RDRAM immed,A
			temp = (cx4.ram_address_pointer + immed) & 0xfff;
			if (temp < 0xc00) {
				set_byte(cx4.ram_data, cx4.ram[temp], sub_op);
			}
			break;

		case 0xe800: // WRRAM subop,A
			temp = cx4.A & 0xfff;
			if (temp < 0xc00) {
				cx4.ram[temp] = get_byte(cx4.ram_data, sub_op);
			}
			break;
		case 0xec00: // WRRAM immed,A
			temp = (cx4.ram_address_pointer + immed) & 0xfff;
			if (temp < 0xc00) {
				cx4.ram[temp] = get_byte(cx4.ram_data, sub_op);
			}
			break;

		case 0x7000: // RDROM
			cx4.rom_data = cx4Rom[cx4.A & 0x3ff];
			break;
		case 0x7400:
			cx4.rom_data = cx4Rom[((sub_op << 8) | immed) & 0x3ff];
			break;

		case 0x7c00: // mov PB_latch[l/h],imm
			set_byte(cx4.PB_latch, immed, sub_op);
			cx4.PB_latch &= 0x7fff;
			break;

		case 0x8000: // ADD A,imm
		case 0x8400:
			cx4.A = add(get_A(), get_immed());
			break;

		case 0x8800: // SUB imm,A
		case 0x8c00:
			cx4.A = sub(get_immed(), get_A());
			break;

		case 0x9000: // SUB A,imm
		case 0x9400:
			cx4.A = sub(get_A(), get_immed());
			break;

		case 0x9800: // MUL imm,A
		case 0x9c00:
			cx4.multiplier = ((int64_t)signextend(get_immed(), 24) * signextend(cx4.A, 24)) & 0xffffffffffff;
			break;

		case 0xa000: // XNOR A,imm
		case 0xa400:
			set_A(~(get_A()) ^ get_immed());
			set_NZ(cx4.A);
			break;

		case 0xa800: // XOR A,imm
		case 0xac00:
			set_A((get_A()) ^ get_immed());
			set_NZ(cx4.A);
			break;

		case 0xb000: // AND A,imm
		case 0xb400:
			set_A((get_A()) & get_immed());
			set_NZ(cx4.A);
			break;

		case 0xb800: // OR A,imm
		case 0xbc00:
			set_A((get_A()) | get_immed());
			set_NZ(cx4.A);
			break;

		case 0xc000: // SHR A,imm
		case 0xc400:
			set_A(cx4.A >> (get_immed() & 0x1f));
			set_NZ(cx4.A);
			break;

		case 0xc800: // ASR A,imm
		case 0xcc00:
			set_A(signextend(cx4.A, 24) >> (get_immed() & 0x1f));
			set_NZ(cx4.A);
			break;

		case 0xd000: // ROR A,imm
		case 0xd400:
			temp = get_immed() & 0x1f;
			set_A((cx4.A >> temp) | (cx4.A << (24 - temp)));
			set_NZ(cx4.A);
			break;

		case 0xd800: // SHL A,imm
		case 0xdc00:
			set_A(cx4.A << (get_immed() & 0x1f));
			set_NZ(cx4.A);
			break;

		case 0xf000: // XCHG A,regs
			temp = cx4.A;
			cx4.A = cx4.reg[immed & 0xf];
			cx4.reg[immed & 0xf] = temp;
			break;

		case 0xf800: // clear
			cx4.A = cx4.ram_address_pointer = cx4.ram_data = cx4.PB_latch = 0x00;
			break;

		case 0xfc00: // stop
#if DEBUG_STARTSTOP
			//bprintf(0, _T("cx4 OP-stop, cycles ran %d\n"), (int)((int64_t)cx4.cycles - cx4.cycles_start));
#endif
			cx4.running = 0;
			if (~cx4.irqcfg & IRQ_ACKNOWLEDGE) {
				set_I(1);
				cpu_setIrq(cx4.snes->cpu, true);
			}
			break;
	}
}

static void tally_cycles()
{
	// 20000000 / (1364 * 312 * 50) or 20000000 / (1364 * 262 * 60) cx4 cycles per master cycle
	cx4.sync_to = (cx4.snes->palTiming) ? snes_sync(cx4.snes, &cx4.sync, 12500, 13299) : snes_sync(cx4.snes, &cx4.sync, 125000, 134013);
}

static inline uint64_t cycles_left()
{
	return cx4.sync_to - cx4.cycles;
}

void cx4_run()
{
	int tcyc = 0;
	tally_cycles();

	while (cx4.cycles < cx4.sync_to) {
		if (cx4.prg_cache_timer) {
			tcyc = (cycles_left() > cx4.prg_cache_timer) ? cx4.prg_cache_timer : 1;
			cycle_advance(tcyc);
			cx4.prg_cache_timer -= tcyc;
		} else if (cx4.dma_timer) {
			tcyc = (cycles_left() > cx4.dma_timer) ? cx4.dma_timer : 1;
			cycle_advance(tcyc);
			cx4.dma_timer -= tcyc;
		} else if (cx4.suspend_timer) {
			tcyc = (cycles_left() > cx4.suspend_timer) ? cx4.suspend_timer : 1;
			cycle_advance(tcyc);
			cx4.suspend_timer -= tcyc;
		} else if (!cx4.running) {
			cycle_advance(cycles_left());
		} else {
			run_insn();
		}
	}
}
