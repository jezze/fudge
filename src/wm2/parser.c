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

    for (; parser->expr.offset < parser->expr.count; parser->expr.offset++)
    {

        char c = parser->expr.data[parser->expr.offset];

        switch (c)
        {

        case '\0':
        case '\n':
            parser->expr.escaped = 0;
            parser->expr.line++;
            parser->expr.linebreak = 1;
            parser->expr.offset++;

            addchar(parser, result, count, i, '\0');

            return i;

        }

        if (parser->expr.escaped)
        {

            parser->expr.escaped = 0;

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
                parser->expr.escaped = 1;

                break;

            case ' ':
                if (parser->expr.inside)
                {

                    addchar(parser, result, count, i, c);

                    i++;

                }

                else
                {

                    addchar(parser, result, count, i, '\0');

                    if (i == 0)
                        continue;

                    parser->expr.offset++;

                    return i;

                }

                break;

            case '"':
                parser->expr.inside = !parser->expr.inside;

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

static void parse_attributes(struct parser *parser, struct widget *widget)
{

    while (!parser->errors && !parser->expr.linebreak)
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

static void parse_command_comment(struct parser *parser)
{

    while (!parser->errors && !parser->expr.linebreak)
        readunsafeword(parser);

}

static void parse_command_delete(struct parser *parser)
{

    fail(parser);

}

static void parse_command_insert(struct parser *parser, unsigned int source, char *in)
{

    unsigned int type = gettype(parser);
    struct widget *widget;

    if (!type)
        fail(parser);

    widget = pool_create(source, type, "", in);

    if (widget)
        parse_attributes(parser, widget);
    else
        fail(parser);

}

static void parse_command_update(struct parser *parser)
{

    fail(parser);

}

static void parse(struct parser *parser, unsigned int source, char *in)
{

    while (!parser->errors && parser->expr.offset < parser->expr.count)
    {

        parser->expr.linebreak = 0;

        switch (getcommand(parser))
        {

        case COMMAND_NONE:
            break;

        case COMMAND_COMMENT:
            parse_command_comment(parser);

            break;

        case COMMAND_DELETE:
            parse_command_delete(parser);

            break;

        case COMMAND_INSERT:
            parse_command_insert(parser, source, in);

            break;

        case COMMAND_UPDATE:
            parse_command_update(parser);

            break;

        default:
            fail(parser);

            break;

        }

    }

}

void parser_parse(struct parser *parser, unsigned int source, char *in, unsigned int count, void *data)
{

    parser->errors = 0;
    parser->expr.data = data;
    parser->expr.count = count;
    parser->expr.offset = 0;
    parser->expr.line = 0;
    parser->expr.linebreak = 0;
    parser->expr.inside = 0;
    parser->expr.escaped = 0;

    parse(parser, source, in);

}

