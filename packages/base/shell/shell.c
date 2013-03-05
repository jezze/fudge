#include <fudge.h>
#include <data/lifo.h>

enum symbol
{

    SYM_NULL        = 0,
    SYM_NUM         = 1,
    SYM_ALPHA       = 2,
    SYM_ALPHANUM    = 3,
    SYM_SPACE       = 4,
    SYM_LT          = 8,
    SYM_GT          = 16,
    SYM_MINUS       = 32,
    SYM_SEMICOLON   = 64,
    SYM_DOT         = 128,
    SYM_SLASH       = 256,
    SYM_WALL        = 512,
    SYM_NEWLINE     = 1024

};

static struct lifo_stack input;
static char *data;
static enum symbol sym;
static unsigned int pos;
static unsigned int len;

static char mapUS[256] =
{
       0,   27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '+', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  '`',    0, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,   27,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '=', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  '~',    0,  '|',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  '<',  '>',  '?',    0,    0,    0,  ' ',    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0
};

static void next()
{

    if (pos >= len)
    {

        sym = SYM_NEWLINE;

        return;

    }

    switch (data[pos])
    {

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

            sym = SYM_NUM;

            break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':

            sym = SYM_ALPHA;

            break;

        case ' ':
        case '\t':

            sym = SYM_SPACE;

            break;

        case '<':

            sym = SYM_LT;

            break;

        case '>':

            sym = SYM_GT;

            break;

        case '-':

            sym = SYM_MINUS;

            break;

        case ';':

            sym = SYM_SEMICOLON;

            break;

        case '.':

            sym = SYM_DOT;

            break;

        case '/':

            sym = SYM_SLASH;

            break;

        case '|':

            sym = SYM_WALL;

            break;

        case '\n':

            sym = SYM_NEWLINE;

            break;

        default:

            sym = SYM_NULL;

            break;

    }

    pos++;

}

static int accept(enum symbol s)
{

    if (sym & s)
    {

        next();

        return 1;

    }

    return 0;

}

static unsigned int setup_executable(unsigned int length, char *path)
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset = 0;

    if (memory_match(path, "/", 1))
        return call_open(3, FUDGE_ROOT, length - 1, path + 1);

    offset += memory_write(buffer, FUDGE_BSIZE, "ramdisk/bin/", 12, offset);
    offset += memory_write(buffer, FUDGE_BSIZE, path, length, offset);

    return call_open(3, FUDGE_ROOT, offset, buffer);

}

static unsigned int setup_stream(unsigned int index, unsigned int length, char *path)
{

    if (memory_match(path, "/", 1))
        return call_open(index, FUDGE_ROOT, length - 1, path + 1);

    return call_open(index, FUDGE_CWD, length, path);

}

static void clear()
{

    lifo_stack_clear(&input);
    setup_stream(FUDGE_IN, 18, "/system/ps2_buffer");
    setup_stream(FUDGE_OUT, 18, "/system/vga_buffer");
    call_write(FUDGE_OUT, 0, 2, "$ ");

}

static void changedir(unsigned int length, char *command)
{

    unsigned int id;
    
    if (!length)
        return;

    if (command[length - 1] != '/')
        return;

    id = (command[0] == '/') ? call_open(3, FUDGE_ROOT, length - 1, command + 1) : call_open(3, FUDGE_CWD, length, command);

    if (!id)
        return;

    call_open(FUDGE_CWD, 3, 0, 0);

}

static void parse()
{

    unsigned int pstart;

    while (accept(SYM_SPACE));

    pstart = pos;

    while (accept(SYM_ALPHANUM | SYM_DOT | SYM_SLASH));

    setup_executable(pos - pstart, data + pstart - 1);

    while (!accept(SYM_WALL | SYM_NEWLINE))
    {

        while (accept(SYM_SPACE));

        if (accept(SYM_LT))
        {

            while (accept(SYM_SPACE));

            pstart = pos;

            while (accept(SYM_ALPHANUM | SYM_DOT | SYM_SLASH));

            setup_stream(FUDGE_IN, pos - pstart, data + pstart - 1);

        }

        else if (accept(SYM_GT))
        {

            while (accept(SYM_SPACE));

            pstart = pos;

            while (accept(SYM_ALPHANUM | SYM_DOT | SYM_SLASH));

            setup_stream(FUDGE_OUT, pos - pstart, data + pstart - 1);

        }

    }

}

static void interpret(unsigned int length, char *command)
{

    if (memory_match(command, "cd ", 3))
    {

        changedir(length - 4, command + 3);

        return;

    }

    data = command;
    pos = 0;
    len = length;

    next();

    while (sym != SYM_NEWLINE)
    {

        parse();
        call_spawn(3);
        call_close(3);

    }

}

static void handle_input(char c)
{

    switch (c)
    {

        case '\0':
        case '\t':

            break;

        case '\b':

            if (!lifo_stack_pop(&input))
                break;

            call_write(FUDGE_OUT, 0, 3, "\b \b");

            break;

        case '\r':
        case '\n':

            lifo_stack_push(&input, c);
            call_write(FUDGE_OUT, 0, 1, &c);

            if (!lifo_stack_isempty(&input))
                interpret(input.head, input.buffer);

            clear();

            break;

        default:

            lifo_stack_push(&input, c);
            call_write(FUDGE_OUT, 0, 1, &c);

            break;

    }

}

static void poll()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int i;
    unsigned int toggleAlt = 0;
    unsigned int toggleShift = 0;
    unsigned int toggleCtrl = 0;

    for (;;)
    {

        count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);

        for (i = 0; i < count; i++)
        {

            unsigned int scancode = buffer[i];

            if (scancode == 0x38)
                toggleAlt = 1;

            if (scancode == 0xB8)
                toggleAlt = 0;

            if (scancode == 0x1D)
                toggleCtrl = 1;

            if (scancode == 0x9D)
                toggleCtrl = 0;

            if (scancode == 0x2A)
                toggleShift = 1;

            if (scancode == 0xAA)
                toggleShift = 0;

            if (scancode & 0x80)
            {

            }

            else
            {

                if (toggleCtrl)
                    scancode = 0;

                if (toggleAlt)
                    scancode = 0;

                if (toggleShift)
                    scancode += 128;

                handle_input(mapUS[scancode]);

            }

        }

    }

}

void main()
{

    clear();
    poll();

}

