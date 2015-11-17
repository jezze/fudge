#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

#define TOKENSKIP                       1
#define TOKENEND                        2
#define TOKENIDENT                      3
#define TOKENIN                         4
#define TOKENOUT                        5
#define TOKENPIPE                       6

static unsigned int walk_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_walk(index, CALL_PR, count - 1, buffer + 1);

    return call_walk(index, indexw, count, buffer);

}

struct token
{

    unsigned int type;
    char *str;

};

struct tokenlist
{

    unsigned int head;
    unsigned int size;
    struct token *table;

};

static void tokenlist_init(struct tokenlist *list, unsigned int size, struct token *table)
{

    list->head = 0;
    list->size = size;
    list->table = table;

}

static void tokenlist_push(struct tokenlist *list, struct token *token)
{

    list->table[list->head].type = token->type;
    list->table[list->head].str = token->str;

    if (list->head < list->size)
        list->head++;

}

static struct token *tokenlist_pop(struct tokenlist *list)
{

    if (!list->head)
        return 0;

    list->head--;

    return &list->table[list->head];

}

static void tokenlist_add(struct tokenlist *list, unsigned int type, char *str)
{

    struct token token;

    token.type = type;
    token.str = str;

    tokenlist_push(list, &token);

}

static unsigned int precedence(struct token *token)
{

    switch (token->type)
    {

    case TOKENEND:
        return 1;

    case TOKENPIPE:
        return 2;

    case TOKENIN:
    case TOKENOUT:
        return 3;

    }

    return 0;

}

static unsigned int tokenize(char c)
{

    switch (c)
    {

    case ' ':
    case '\t':
        return TOKENSKIP;

    case '<':
        return TOKENIN;

    case '>':
        return TOKENOUT;

    case '|':
        return TOKENPIPE;

    case ';':
    case '\n':
        return TOKENEND;

    }

    return TOKENIDENT;

}

static unsigned int getidentlength(unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(buffer[i]);

        if (token != TOKENIDENT)
            break;

    }

    return i;

}

static void tokenizebuffer(struct tokenlist *infix, struct buffer *stringtable, unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int token = tokenize(buffer[i]);

        if (token == TOKENSKIP)
            continue;

        if (token == TOKENIDENT)
        {

            unsigned int c;

            tokenlist_add(infix, token, (char *)stringtable->head);

            c = getidentlength(count - i, buffer + i);

            buffer_wcfifo(stringtable, c, &buffer[i]);
            buffer_wcfifo(stringtable, 1, "\0");

            i += c - 1;

        }

        else
        {

            tokenlist_add(infix, token, 0);

        }

    }

}

static void translate(struct tokenlist *postfix, struct tokenlist *infix, struct tokenlist *stack)
{

    unsigned int i;

    for (i = 0; i < infix->head; i++)
    {

        struct token *token = &infix->table[i];
        struct token *t;

        if (token->type == TOKENIDENT)
        {

            tokenlist_push(postfix, token);

            continue;

        }

        if (token->type == TOKENEND || token->type == TOKENPIPE)
        {

            while ((t = tokenlist_pop(stack)))
                tokenlist_push(postfix, t);

            tokenlist_push(postfix, token);

            continue;

        }

        while ((t = tokenlist_pop(stack)))
        {

            if (precedence(token) > precedence(t))
            {

                tokenlist_push(stack, t);

                break;

            }

            tokenlist_push(postfix, t);

        }

        tokenlist_push(stack, token);

    }

}

static void parse(struct tokenlist *postfix, struct tokenlist *stack)
{

    unsigned int i;

    for (i = 0; i < postfix->head; i++)
    {

        struct token *token = &postfix->table[i];
        struct token *t;

        switch (token->type)
        {

        case TOKENIDENT:
            tokenlist_push(stack, token);

            break;

        case TOKENIN:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_CI, CALL_PW, ascii_length(t->str), t->str))
                return;

            break;

        case TOKENOUT:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_CO, CALL_PW, ascii_length(t->str), t->str))
                return;

            break;

        case TOKENPIPE:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_CP, CALL_L0, ascii_length(t->str), t->str))
                return;

            if (!call_walk(CALL_L1, CALL_PR, 18, "system/pipe/clone/"))
                return;

            call_walk(CALL_CO, CALL_L1, 1, "0");
            call_spawn();
            call_walk(CALL_CI, CALL_L1, 1, "1");

            break;

        case TOKENEND:
            t = tokenlist_pop(stack);

            if (!t)
                return;

            if (!walk_path(CALL_CP, CALL_L0, ascii_length(t->str), t->str))
                return;

            call_walk(CALL_CO, CALL_PO, 0, 0);
            call_spawn();
            call_walk(CALL_CI, CALL_PI, 0, 0);

            break;

        }

    }

}

void main(void)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;
    char stringdata[FUDGE_BSIZE];
    struct buffer stringtable;
    struct token infixdata[1024];
    struct token postfixdata[1024];
    struct token stackdata[8];
    struct tokenlist infix;
    struct tokenlist postfix;
    struct tokenlist stack;

    buffer_init(&stringtable, FUDGE_BSIZE, stringdata);
    tokenlist_init(&infix, 1024, infixdata);
    tokenlist_init(&postfix, 1024, postfixdata);
    tokenlist_init(&stack, 8, stackdata);

    if (!call_walk(CALL_L0, CALL_PR, 4, "bin/"))
        return;

    call_open(CALL_PI);

    while ((count = file_read(CALL_PI, buffer, FUDGE_BSIZE)))
        tokenizebuffer(&infix, &stringtable, count, buffer);

    call_close(CALL_PI);

    if (stack.head)
        return;

    translate(&postfix, &infix, &stack);

    if (stack.head)
        return;

    parse(&postfix, &stack);

}

