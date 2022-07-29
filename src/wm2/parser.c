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

static void fail(struct parser *parser)
{

    parser->errors++;

    parser->fail();

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

    char command[32];
    unsigned int count = readword(parser, command, 32);

    return (count) ? getkey(commands, 5, command) : 0;

}

static void parseskip(struct parser *parser)
{

    char word[4096];

    readword(parser, word, 4096);

}

static void parse_attributes(struct parser *parser, struct widget *widget)
{

    while (!parser->errors && !parser->expr.linebreak)
    {

    }

}

static void parse_command_comment(struct parser *parser)
{

    while (!parser->errors && !parser->expr.linebreak)
        parseskip(parser);

}

static void parse_command_delete(struct parser *parser)
{

    fail(parser);

}

static void parse_command_insert(struct parser *parser, char *in)
{

    struct widget *widget = pool_create(0, 0, "", in);

    if (widget)
        parse_attributes(parser, widget);
    else
        fail(parser);

}

static void parse_command_update(struct parser *parser)
{

    fail(parser);

}

static void parse(struct parser *parser, char *in)
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
            parse_command_insert(parser, in);

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

void parser_parse(struct parser *parser, char *in, unsigned int count, void *data)
{

    parser->errors = 0;
    parser->expr.data = data;
    parser->expr.count = count;
    parser->expr.offset = 0;
    parser->expr.line = 0;
    parser->expr.linebreak = 0;
    parser->expr.inside = 0;
    parser->expr.escaped = 0;

    parse(parser, in);

}

void parser_init(struct parser *parser, void (*fail)(void), struct widget *(*find)(char *name), struct widget *(*create)(unsigned int type, char *id, char *in), struct widget *(*destroy)(struct widget *widget), void (*clear)(struct widget *widget))
{

    parser->fail = fail;
    parser->find = find;
    parser->create = create;
    parser->destroy = destroy;
    parser->clear = clear;

}

