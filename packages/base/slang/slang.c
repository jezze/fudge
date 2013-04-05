#include <fudge.h>

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
    TOKEN_QUOTE                         = 1024,
    TOKEN_NEWLINE                       = 2048

};

struct lexer
{

    char *buffer;
    unsigned int count;
    unsigned int next;

};

struct directory
{

    char *path;
    unsigned int count;

};

static struct directory binaries[1] =
{

    {"bin/", 4}

};

static unsigned int open(unsigned int index, unsigned int count, char *path, unsigned int cdir, struct directory *dirs)
{

    unsigned int i;

    if (memory_match(path, "/", 1))
        return call_open(index, FUDGE_ROOT, count - 1, path + 1);

    for (i = 0; i < cdir; i++)
    {

        char temp[FUDGE_BSIZE];
        unsigned int offset = 0;

        offset += memory_write(temp, FUDGE_BSIZE, dirs[i].path, dirs[i].count, offset);
        offset += memory_write(temp, FUDGE_BSIZE, path, count, offset);

        if (call_open(index, FUDGE_ROOT, offset, temp))
            return index;

    }

    return call_open(index, FUDGE_CWD, count, path);

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

        case '"':

            return TOKEN_QUOTE;

        case '\n':

            return TOKEN_NEWLINE;

    }

    return TOKEN_NULL;

}

static enum token current(struct lexer *lexer)
{

    return tokenize(lexer->buffer[lexer->next - 1]);

}

static unsigned int next(struct lexer *lexer)
{

    if (lexer->next < lexer->count)
        lexer->next++;

    return lexer->next;

}

static unsigned int accept(struct lexer *lexer, enum token token)
{

    if (current(lexer) & token)
        return next(lexer);

    return 0;

}

static unsigned int command;
static unsigned int in;
static unsigned int out;

static unsigned int parse_command(struct lexer *lexer)
{

    unsigned int index = lexer->next;

    if (!accept(lexer, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH))
        return 0;

    while (accept(lexer, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH));

    command = open(3, lexer->next - index, lexer->buffer + index - 1, 1, binaries);

    return 1;

}

static unsigned int parse_in(struct lexer *lexer)
{

    unsigned int index;

    if (!accept(lexer, TOKEN_LT))
        return 0;

    while (accept(lexer, TOKEN_SPACE));

    index = lexer->next;

    if (!accept(lexer, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH))
        return 0;

    while (accept(lexer, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH));

    in = open(FUDGE_IN, lexer->next - index, lexer->buffer + index - 1, 0, 0);

    return 1;

}

static unsigned int parse_out(struct lexer *lexer)
{

    unsigned int index;

    if (!accept(lexer, TOKEN_GT))
        return 0;

    while (accept(lexer, TOKEN_SPACE));

    index = lexer->next;

    if (!accept(lexer, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH))
        return 0;

    while (accept(lexer, TOKEN_ALPHANUM | TOKEN_DOT | TOKEN_SLASH));

    out = open(FUDGE_OUT, lexer->next - index, lexer->buffer + index - 1, 0, 0);

    return 1;

}

static unsigned int parse_data(struct lexer *lexer)
{

    unsigned int index;

    if (!accept(lexer, TOKEN_QUOTE))
        return 0;

    index = lexer->next;

    while (accept(lexer, ~TOKEN_QUOTE));

    if (!accept(lexer, TOKEN_QUOTE))
        return 0;

    call_write(FUDGE_IN, 0, lexer->next - index - 1, lexer->buffer + index - 1);

    return 1;

}

static unsigned int parse_expression(struct lexer *lexer)
{

    char path[64];
    unsigned int loop = 0;

    call_open(4, FUDGE_IN, 0, 0);
    call_open(5, FUDGE_OUT, 0, 0);

    do
    {

        if (loop)
        {

            memory_copy(path, "temp/", 5);
            string_write_num(path + 5, 32, loop, 10);

            call_open(FUDGE_OUT, FUDGE_ROOT, string_length(path), path);
            call_spawn(3);
            call_open(FUDGE_IN, FUDGE_OUT, 0, 0);
            call_open(FUDGE_OUT, 5, 0, 0);

        }

        command = 0;
        in = 0;
        out = 0;

        while (accept(lexer, TOKEN_SPACE));

        if (!parse_command(lexer))
            return 0;

        while (accept(lexer, TOKEN_SPACE));

        if (parse_in(lexer))
        {

            while (accept(lexer, TOKEN_SPACE));

            parse_out(lexer);

        }

        else if (parse_out(lexer))
        {

            while (accept(lexer, TOKEN_SPACE));

            parse_in(lexer);

        }

        while (accept(lexer, TOKEN_SPACE));

        parse_data(lexer);

        while (accept(lexer, TOKEN_SPACE));

        loop++;

    } while (accept(lexer, TOKEN_WALL));

    call_spawn(3);

    return 1;

}

static unsigned int parse(struct lexer *lexer)
{

    do
    {

        while (accept(lexer, TOKEN_SPACE));

        if (!parse_expression(lexer))
            return 0;

        while (accept(lexer, TOKEN_SPACE));

    } while (accept(lexer, TOKEN_SEMICOLON));

    while (accept(lexer, TOKEN_SPACE));

    if (!accept(lexer, TOKEN_NEWLINE))
        return 0;

    return 1;

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct lexer lexer;

    lexer.buffer = buffer;
    lexer.count = call_read(FUDGE_IN, 0, FUDGE_BSIZE, buffer);
    lexer.next = 1;

    parse(&lexer);

}

