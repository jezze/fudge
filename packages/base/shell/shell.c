#include <fudge.h>
#include <data/lifo.h>

enum token
{

    TOKEN_NULL                          = 0,
    TOKEN_NUM                           = 1,
    TOKEN_ALPHA                         = 2,
    TOKEN_ALPHANUM                      = 3,
    TOKEN_SPACE                         = 4,
    TOKEN_LT                            = 8,
    TOKEN_GT                            = 16,
    TOKEN_MINUS                         = 32,
    TOKEN_SEMICOLON                     = 64,
    TOKEN_DOT                           = 128,
    TOKEN_SLASH                         = 256,
    TOKEN_WALL                          = 512,
    TOKEN_NEWLINE                       = 1024

};

struct reader
{

    char *buffer;
    unsigned int count;
    unsigned int index;

};

static struct lifo_stack input;

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

static unsigned int open_stream(unsigned int index, unsigned int count, char *path)
{

    if (memory_match(path, "/", 1))
        return call_open(index, FUDGE_ROOT, count - 1, path + 1);

    return call_open(index, FUDGE_CWD, count, path);

}

static unsigned int open_executable(unsigned int index, unsigned int count, char *path)
{

    char temp[FUDGE_BSIZE];
    unsigned int offset = 0;

    if (memory_match(path, "/", 1))
        return call_open(index, FUDGE_ROOT, count - 1, path + 1);

    offset += memory_write(temp, FUDGE_BSIZE, "ramdisk/bin/", 12, offset);
    offset += memory_write(temp, FUDGE_BSIZE, path, count, offset);

    return call_open(index, FUDGE_ROOT, offset, temp);

}

static enum token tokenize(char c)
{

    switch (c)
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

            return TOKEN_NUM;

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
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':

            return TOKEN_ALPHA;

        case ' ':
        case '\t':

            return TOKEN_SPACE;

        case '<':

            return TOKEN_LT;

        case '>':

            return TOKEN_GT;

        case '-':

            return TOKEN_MINUS;

        case ';':

            return TOKEN_SEMICOLON;

        case '.':

            return TOKEN_DOT;

        case '/':

            return TOKEN_SLASH;

        case '|':

            return TOKEN_WALL;

        case '\n':

            return TOKEN_NEWLINE;

    }

    return TOKEN_NULL;

}

static unsigned int next(struct reader *reader)
{

    if (reader->index < reader->count)
        reader->index++;

    return reader->index;

}

static unsigned int accept(struct reader *reader, enum token token)
{

    if (tokenize(reader->buffer[reader->index - 1]) & token)
        return next(reader);

    return 0;

}

static void parse(struct reader *reader)
{

    unsigned int index;

    while (accept(reader, TOKEN_SPACE));

    index = reader->index;

    while (accept(reader, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH));

    open_executable(3, reader->index - index, reader->buffer + index - 1);

    while (!accept(reader, TOKEN_WALL | TOKEN_NEWLINE))
    {

        while (accept(reader, TOKEN_SPACE));

        if (accept(reader, TOKEN_LT))
        {

            while (accept(reader, TOKEN_SPACE));

            index = reader->index;

            while (accept(reader, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH));

            open_stream(FUDGE_IN, reader->index - index, reader->buffer + index - 1);

        }

        else if (accept(reader, TOKEN_GT))
        {

            while (accept(reader, TOKEN_SPACE));

            index = reader->index;

            while (accept(reader, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH));

            open_stream(FUDGE_OUT, reader->index - index, reader->buffer + index - 1);

        }

    }

}

static void interpret_cd(unsigned int count, char *buffer)
{

    unsigned int id;
    
    if (!count)
        return;

    if (buffer[count - 1] != '/')
        return;

    id = (buffer[0] == '/') ? call_open(3, FUDGE_ROOT, count - 1, buffer + 1) : call_open(3, FUDGE_CWD, count, buffer);

    if (!id)
        return;

    call_open(FUDGE_CWD, 3, 0, 0);

}

static void interpret(unsigned int count, char *buffer)
{

    struct reader reader;

    reader.buffer = buffer;
    reader.count = count;
    reader.index = 1;

    if (memory_match(reader.buffer, "cd ", 3))
    {

        interpret_cd(reader.count - 4, reader.buffer + 3);

        return;

    }

    while (tokenize(reader.buffer[reader.index - 1]) != TOKEN_NEWLINE)
    {

        parse(&reader);
        call_spawn(3);
        call_close(3);

    }

}

static void clear()
{

    lifo_stack_clear(&input);
    call_open(FUDGE_IN, FUDGE_ROOT, 17, "system/ps2_buffer");
    call_open(FUDGE_OUT, FUDGE_ROOT, 17, "system/vga_buffer");
    call_write(FUDGE_OUT, 0, 2, "$ ");

}

static void handle(char c)
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
    unsigned int alt = 0;
    unsigned int shift = 0;
    unsigned int ctrl = 0;

    for (;;)
    {

        count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);

        for (i = 0; i < count; i++)
        {

            unsigned int scancode = buffer[i];

            if (scancode & 0x80)
            {

                if (scancode == 0x9D)
                    ctrl = 0;

                if (scancode == 0xAA)
                    shift = 0;

                if (scancode == 0xB8)
                    alt = 0;

            }

            else
            {

                if (scancode == 0x1D)
                    ctrl = 1;

                if (scancode == 0x2A)
                    shift = 1;

                if (scancode == 0x38)
                    alt = 1;

                if (ctrl)
                    scancode = 0;

                if (alt)
                    scancode = 0;

                if (shift)
                    scancode += 128;

                handle(mapUS[scancode]);

            }

        }

    }

}

void main()
{

    clear();
    poll();

}

