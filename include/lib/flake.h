#ifndef LIB_FLAKE_H
#define LIB_FLAKE_H

#define FLAKE_VERSION          "FLAKE1"
#define FLAKE_SIZE_MIN         0x00000080
#define FLAKE_SIZE_MAX         0x00010000

#define FLAKE_TRANSMIT_FLAG    0x4000
#define FLAKE_TRANSMIT_ERROR   0x4FFF
#define FLAKE_TRANSMIT_VERSION 0x4004
#define FLAKE_TRANSMIT_OPEN    0x4008
#define FLAKE_TRANSMIT_CLOSE   0x4009
#define FLAKE_TRANSMIT_READ    0x400A
#define FLAKE_TRANSMIT_WRITE   0x400B

#define FLAKE_RESPONSE_FLAG    0x8000
#define FLAKE_RESPONSE_ERROR   0x8FFF
#define FLAKE_RESPONSE_VERSION 0x8004
#define FLAKE_RESPONSE_OPEN    0x8008
#define FLAKE_RESPONSE_CLOSE   0x8009
#define FLAKE_RESPONSE_READ    0x800A
#define FLAKE_RESPONSE_WRITE   0x800B

struct flake_header
{

    unsigned int size;
    unsigned short type;
    unsigned short tag;

};

struct flake_transmit_error
{

    struct flake_header header;
    char *message;

};

struct flake_transmit_version
{

    struct flake_header header;
    char version[16];

};

struct flake_transmit_open
{

    struct flake_header header;
    unsigned int id;

};

struct flake_transmit_close
{

    struct flake_header header;
    unsigned int id;

};

struct flake_transmit_read
{

    struct flake_header header;
    unsigned int id;
    unsigned int offset;
    unsigned int count;

};

struct flake_transmit_write
{

    struct flake_header header;
    unsigned int id;
    unsigned int offset;
    unsigned int count;

};

struct flake_response_error
{

    struct flake_header header;
    char *message;

};

struct flake_response_version
{

    struct flake_header header;
    char version[16];

};

struct flake_response_open
{

    struct flake_header header;

};

struct flake_response_close
{

    struct flake_header header;

};

struct flake_response_read
{

    struct flake_header header;
    unsigned int count;
    void *buffer;

};

struct flake_response_write
{

    struct flake_header header;
    unsigned int count;

};

#endif

