#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"
#include "parser.h"

#define COMMAND_NONE                    1
#define COMMAND_COMMENT                 2
#define COMMAND_DELETE                  3
#define COMMAND_INSERT                  4
#define COMMAND_UPDATE                  5

struct parser
{

    char *data;
    unsigned int count;
    unsigned int offset;
    unsigned int line;
    unsigned int linebreak;
    unsigned int quoted;
    unsigned int escaped;
    unsigned int errors;

};

struct token
{

    unsigned int key;
    char *value;

};

static struct token commands[] = {
    {COMMAND_NONE, ""},
    {COMMAND_COMMENT, "#"},
    {COMMAND_DELETE, "-"},
    {COMMAND_INSERT, "+"},
    {COMMAND_UPDATE, "="}
};

char strdata[0x4000];
unsigned int strdataoffset;

static void fail(struct parser *parser)
{

    parser->errors++;

}

static void addchar(struct parser *parser, char *result, unsigned int count, unsigned int i, char c)
{

    if (i < count)
        result[i] = c;
    else
        fail(parser);

}

static unsigned int readword(struct parser *parser, char *result, unsigned int count)
{

    unsigned int i = 0;

    for (; parser->offset < parser->count; parser->offset++)
    {

        char c = parser->data[parser->offset];

        switch (c)
        {

        case '\0':
        case '\n':
            parser->escaped = 0;
            parser->line++;
            parser->linebreak = 1;
            parser->offset++;

            addchar(parser, result, count, i, '\0');

            return i;

        }

        if (parser->escaped)
        {

            parser->escaped = 0;

            switch (c)
            {

            case 'n':
                addchar(parser, result, count, i, '\n');

                i++;

                break;

            default:
                addchar(parser, result, count, i, c);

                i++;

                break;

            }

        }

        else
        {

            switch (c)
            {

            case '\\':
                parser->escaped = 1;

                break;

            case ' ':
                if (parser->quoted)
                {

                    addchar(parser, result, count, i, c);

                    i++;

                }

                else
                {

                    addchar(parser, result, count, i, '\0');

                    if (i == 0)
                        continue;

                    parser->offset++;

                    return i;

                }

                break;

            case '"':
                parser->quoted = !parser->quoted;

                break;

            default:
                addchar(parser, result, count, i, c);

                i++;

                break;

            }

        }

    }

    return 0;

}

static char *readunsafeword(struct parser *parser)
{

    char *word = strdata + strdataoffset;
    unsigned int count = readword(parser, word, 0x4000 - strdataoffset);

    return (count) ? word : 0;

}

static char *readsafeword(struct parser *parser)
{

    char *word = strdata + strdataoffset;
    unsigned int count = readword(parser, word, 0x4000 - strdataoffset);

    if (count)
    {

        strdataoffset += count + 1;

        return word;

    }

    return 0;

}

static unsigned int getkey(struct token *tokens, unsigned int n, char *value)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        if (cstring_match(tokens[i].value, value))
            return tokens[i].key;

    }

    return 0;

}

static unsigned int getcommand(struct parser *parser)
{

    char *word = readunsafeword(parser);

    return (word) ? getkey(commands, 5, word) : 0;

}

static unsigned int gettype(struct parser *parser)
{

    char *word = readunsafeword(parser);

    return (word) ? widget_gettype(word) : 0;

}

static void parseattributes(struct parser *parser, struct widget *widget)
{

    while (!parser->errors && !parser->linebreak)
    {

        char *word = readunsafeword(parser);
        unsigned int attribute;

        if (!word)
            fail(parser);

        attribute = widget_getattribute(word);
        word = readsafeword(parser);

        if (!word)
            fail(parser);

        widget_setattribute(widget, attribute, word);

    }

}

static void parsecomment(struct parser *parser)
{

    while (!parser->errors && !parser->linebreak)
        readunsafeword(parser);

}

static void parsedelete(struct parser *parser)
{

    fail(parser);

}

static void parseinsert(struct parser *parser, unsigned int source, char *in)
{

    unsigned int type = gettype(parser);

    if (type)
    {

        struct widget *widget = pool_create(source, type, "", in);

        if (widget)
            parseattributes(parser, widget);

    }

    else
    {

        fail(parser);
        
    }

}

static void parseupdate(struct parser *parser, unsigned int source)
{

    fail(parser);

}

static void parse(struct parser *parser, unsigned int source, char *in)
{

    while (!parser->errors && parser->offset < parser->count)
    {

        parser->linebreak = 0;

        switch (getcommand(parser))
        {

        case COMMAND_NONE:
            break;

        case COMMAND_COMMENT:
            parsecomment(parser);

            break;

        case COMMAND_DELETE:
            parsedelete(parser);

            break;

        case COMMAND_INSERT:
            parseinsert(parser, source, in);

            break;

        case COMMAND_UPDATE:
            parseupdate(parser, source);

            break;

        default:
            fail(parser);

            break;

        }

    }

}

void parser_parse(unsigned int source, char *in, unsigned int count, void *data)
{

    struct parser parser;

    parser.errors = 0;
    parser.data = data;
    parser.count = count;
    parser.offset = 0;
    parser.line = 0;
    parser.linebreak = 0;
    parser.quoted = 0;
    parser.escaped = 0;

    parse(&parser, source, in);

}

