#include <fudge.h>
#include "html.h"
#include "js.h"

#define ARGUMENTS                       8
#define STEPLIMIT                       2000000
#define DEPTHLIMIT                      24
#define NUMBERSIZE                      16
#define PUNCTUATORS                     46
#define BINARIES                        21
#define NODE_NUMBER                     1
#define NODE_STRING                     2
#define NODE_NAME                       3
#define NODE_ARRAY                      4
#define NODE_OBJECT                     5
#define NODE_PROPERTY                   6
#define NODE_FUNCTION                   7
#define NODE_MEMBER                     8
#define NODE_INDEX                      9
#define NODE_CALL                       10
#define NODE_ASSIGN                     11
#define NODE_BINARY                     12
#define NODE_LOGICAL                    13
#define NODE_UNARY                      14
#define NODE_UPDATE                     15
#define NODE_CONDITIONAL                16
#define NODE_VAR                        20
#define NODE_DECLARATOR                 21
#define NODE_EXPRESSION                 22
#define NODE_RETURN                     23
#define NODE_IF                         24
#define NODE_BLOCK                      25
#define NODE_WHILE                      26
#define NODE_DO                         27
#define NODE_FOR                        28
#define NODE_TRY                        29
#define NODE_THROW                      30
#define NODE_DECLARATION                31
#define NODE_BREAK                      32
#define NODE_CONTINUE                   33
#define NODE_EMPTY                      34
#define NODE_FOREACH                    35
#define NODE_SWITCH                     36
#define NODE_CASE                       37
#define LOOP_OF                         1
#define LOOP_IN                         2
#define PATTERN_ARRAY                   1
#define PATTERN_OBJECT                  2
#define OP_ADD                          1
#define OP_SUBTRACT                     2
#define OP_MULTIPLY                     3
#define OP_DIVIDE                       4
#define OP_MODULO                       5
#define OP_EQUAL                        6
#define OP_NOTEQUAL                     7
#define OP_STRICTEQUAL                  8
#define OP_STRICTNOTEQUAL               9
#define OP_LESS                         10
#define OP_GREATER                      11
#define OP_LESSEQUAL                    12
#define OP_GREATEREQUAL                 13
#define OP_AND                          14
#define OP_OR                           15
#define OP_NULLISH                      16
#define OP_BITAND                       17
#define OP_BITOR                        18
#define OP_BITXOR                       19
#define OP_SHIFTLEFT                    20
#define OP_SHIFTRIGHT                   21
#define OP_NOT                          22
#define OP_NEGATE                       23
#define OP_PLUS                         24
#define OP_TYPEOF                       25
#define OP_VOID                         26
#define OP_BITNOT                       27
#define OP_INCREMENT                    28
#define OP_DECREMENT                    29
#define OP_ASSIGN                       30
#define TOKEN_END                       0
#define TOKEN_NUMBER                    1
#define TOKEN_STRING                    2
#define TOKEN_NAME                      3
#define TOKEN_PUNCTUATOR                4
#define COMPLETION_NORMAL               0
#define COMPLETION_RETURN               1
#define COMPLETION_BREAK                2
#define COMPLETION_CONTINUE             3
#define KIND_PLAIN                      1
#define KIND_ARRAY                      2
#define KIND_FUNCTION                   3
#define KIND_NATIVE                     4
#define KIND_SCOPE                      5
#define KIND_WINDOW                     6
#define KIND_DOCUMENT                   7
#define KIND_ELEMENT                    8
#define KIND_STORAGE                    9
#define KIND_MEDIA                      10
#define KIND_CONSOLE                    11
#define NATIVE_NOOP                     0
#define NATIVE_GETELEMENTBYID           1
#define NATIVE_ADDEVENTLISTENER         2
#define NATIVE_SETATTRIBUTE             3
#define NATIVE_GETATTRIBUTE             4
#define NATIVE_REMOVEATTRIBUTE          5
#define NATIVE_HASATTRIBUTE             6
#define NATIVE_MATCHMEDIA               7
#define NATIVE_GETITEM                  8
#define NATIVE_SETITEM                  9
#define NATIVE_REMOVEITEM               10
#define NATIVE_INDEXOF                  11
#define NATIVE_PUSH                     12
#define NATIVE_JOIN                     13
#define NATIVE_TOUPPERCASE              14
#define NATIVE_TOLOWERCASE              15
#define NATIVE_QUERYSELECTOR            16

struct token
{

    unsigned int type;
    unsigned int start;
    unsigned int length;
    int value;

};

struct parser
{

    struct js_engine *js;
    char *source;
    unsigned int count;
    unsigned int offset;
    struct token token;
    unsigned int failed;

};

struct completion
{

    unsigned int type;
    struct js_value value;

};

struct binary
{

    char *text;
    unsigned int op;
    unsigned int precedence;
    unsigned int logical;

};

struct call
{

    struct js_value function;
    struct js_value self;
    struct js_value arguments[ARGUMENTS];
    unsigned int count;

};

static char *punctuators[PUNCTUATORS] = {
    ">>>=", "===", "!==", "**=", "...", "<<=", ">>=", ">>>", "=>", "==", "!=", "<=", ">=", "&&", "||", "??",
    "++", "--", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<", ">>", "**", "?.", "{", "}",
    "(", ")", "[", "]", ";", ",", "<", ">", "+", "-", "*", "/", "%", "&"
};
static char *singles = "|^!~?:=.";
static struct binary binaries[BINARIES] = {
    {"??", OP_NULLISH, 1, 1}, {"||", OP_OR, 2, 1}, {"&&", OP_AND, 3, 1}, {"|", OP_BITOR, 4, 0}, {"^", OP_BITXOR, 5, 0}, {"&", OP_BITAND, 6, 0},
    {"===", OP_STRICTEQUAL, 7, 0}, {"!==", OP_STRICTNOTEQUAL, 7, 0}, {"==", OP_EQUAL, 7, 0}, {"!=", OP_NOTEQUAL, 7, 0},
    {"<=", OP_LESSEQUAL, 8, 0}, {">=", OP_GREATEREQUAL, 8, 0}, {"<", OP_LESS, 8, 0}, {">", OP_GREATER, 8, 0},
    {"<<", OP_SHIFTLEFT, 9, 0}, {">>", OP_SHIFTRIGHT, 9, 0}, {"+", OP_ADD, 10, 0}, {"-", OP_SUBTRACT, 10, 0},
    {"*", OP_MULTIPLY, 11, 0}, {"/", OP_DIVIDE, 11, 0}, {"%", OP_MODULO, 11, 0}
};

static struct js_value execlist(struct js_engine *js, unsigned int first, unsigned int scope, struct completion *completion);
static void execute(struct js_engine *js, unsigned int index, unsigned int scope, struct completion *completion);
static struct js_value evaluate(struct js_engine *js, unsigned int node, unsigned int scope);
static unsigned int parseexpression(struct parser *parser);
static unsigned int parseassignment(struct parser *parser);
static unsigned int parsestatement(struct parser *parser);
static unsigned int parseblock(struct parser *parser);

static unsigned int isspace(char c)
{

    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v';

}

static unsigned int isnamestart(char c)
{

    return cstring_isalpha(c) || c == '_' || c == '$' || (unsigned char)c >= 0x80;

}

static unsigned int isnamechar(char c)
{

    return isnamestart(c) || cstring_isdigit(c);

}

static char *getstring(struct js_engine *js, unsigned int offset)
{

    return js->strings + offset;

}

static unsigned int addstring(struct js_engine *js, char *text, unsigned int length)
{

    unsigned int offset = js->stringcount;

    if (offset + length + 1 > JS_STRINGSIZE)
        return 0;

    buffer_copy(js->strings + offset, text, length);

    js->strings[offset + length] = '\0';
    js->stringcount += length + 1;

    return offset;

}

static unsigned int intern(struct js_engine *js, char *text, unsigned int length)
{

    unsigned int i;

    for (i = 0; i < js->interncount; i++)
    {

        char *string = getstring(js, js->interns[i]);

        if (cstring_length(string) == length && buffer_match(string, text, length))
            return js->interns[i];

    }

    if (js->interncount >= JS_INTERNS)
        return addstring(js, text, length);

    js->interns[js->interncount] = addstring(js, text, length);

    return js->interns[js->interncount++];

}

static unsigned int internstring(struct js_engine *js, char *text)
{

    return intern(js, text, cstring_length(text));

}

static struct js_value value(unsigned int type, int number)
{

    struct js_value result;

    result.type = type;
    result.number = number;

    return result;

}

static struct js_value undefined(void)
{

    return value(JS_UNDEFINED, 0);

}

static struct js_value stringvalue(struct js_engine *js, char *text)
{

    return value(JS_STRING, addstring(js, text, cstring_length(text)));

}

static void copyvalue(struct js_value *to, struct js_value *from)
{

    to->type = from->type;
    to->number = from->number;

}

static unsigned int newobject(struct js_engine *js, unsigned int kind, unsigned int parent, unsigned int data)
{

    struct js_object *object;

    if (js->objectcount >= JS_OBJECTS)
        return 0;

    object = &js->objects[js->objectcount];
    object->kind = kind;
    object->property = 0;
    object->parent = parent;
    object->data = data;

    return js->objectcount++;

}

static struct js_property *findproperty(struct js_engine *js, unsigned int object, unsigned int name)
{

    unsigned int property;

    for (property = js->objects[object].property; property; property = js->properties[property].next)
    {

        if (js->properties[property].name == name)
            return &js->properties[property];

    }

    return 0;

}

static void putproperty(struct js_engine *js, unsigned int object, unsigned int name, struct js_value *value)
{

    struct js_property *property = findproperty(js, object, name);

    if (!property)
    {

        if (js->propertycount >= JS_PROPERTIES || !object)
            return;

        property = &js->properties[js->propertycount];
        property->name = name;
        property->next = js->objects[object].property;
        js->objects[object].property = js->propertycount++;

    }

    copyvalue(&property->value, value);

}

static void throwerror(struct js_engine *js, char *message)
{

    if (!js->throwing)
    {

        js->throwing = 1;
        js->thrown = stringvalue(js, message);

    }

}

static unsigned int isfunction(struct js_engine *js, struct js_value *value)
{

    return value->type == JS_OBJECT && (js->objects[value->number].kind == KIND_FUNCTION || js->objects[value->number].kind == KIND_NATIVE);

}

static unsigned int truthy(struct js_engine *js, struct js_value *value)
{

    switch (value->type)
    {

    case JS_BOOLEAN:
    case JS_NUMBER:
        return value->number != 0;

    case JS_STRING:
        return getstring(js, value->number)[0] != '\0';

    case JS_OBJECT:
        return 1;

    }

    return 0;

}

static int parseinteger(char *text)
{

    int sign = 1;
    int result = 0;

    while (isspace(text[0]))
        text++;

    if (text[0] == '-' || text[0] == '+')
    {

        sign = (text[0] == '-') ? -1 : 1;
        text++;

    }

    while (cstring_isdigit(text[0]))
    {

        result = result * 10 + (text[0] - '0');
        text++;

    }

    return sign * result;

}

static int tonumber(struct js_engine *js, struct js_value *value)
{

    if (value->type == JS_STRING)
        return parseinteger(getstring(js, value->number));

    if (value->type == JS_NUMBER || value->type == JS_BOOLEAN)
        return value->number;

    return 0;

}

static unsigned int numberstring(struct js_engine *js, int number)
{

    char text[NUMBERSIZE];
    unsigned int magnitude = (number < 0) ? (unsigned int)(-number) : (unsigned int)number;
    unsigned int length = cstring_write_fmt(text, NUMBERSIZE, 0, (number < 0) ? "-%u" : "%u", &magnitude);

    return intern(js, text, length);

}

static unsigned int tostring(struct js_engine *js, struct js_value *value);

static unsigned int concat(struct js_engine *js, unsigned int a, unsigned int b);

static unsigned int joinarray(struct js_engine *js, unsigned int array, char *separator)
{

    unsigned int glue = internstring(js, separator);
    unsigned int result = internstring(js, "");
    unsigned int length = js->objects[array].data;
    unsigned int i;

    for (i = 0; i < length && !js->throwing; i++)
    {

        struct js_property *property = findproperty(js, array, numberstring(js, i));

        if (i)
            result = concat(js, result, glue);

        if (property && property->value.type > JS_NULL)
            result = concat(js, result, tostring(js, &property->value));

    }

    return result;

}

static unsigned int tostring(struct js_engine *js, struct js_value *value)
{

    switch (value->type)
    {

    case JS_STRING:
        return value->number;

    case JS_NUMBER:
        return numberstring(js, value->number);

    case JS_BOOLEAN:
        return internstring(js, (value->number) ? "true" : "false");

    case JS_NULL:
        return internstring(js, "null");

    case JS_OBJECT:
        if (js->objects[value->number].kind == KIND_ARRAY)
            return joinarray(js, value->number, ",");

        return internstring(js, (isfunction(js, value)) ? "function" : "[object Object]");

    }

    return internstring(js, "undefined");

}

static unsigned int concat(struct js_engine *js, unsigned int a, unsigned int b)
{

    unsigned int la = cstring_length(getstring(js, a));
    unsigned int lb = cstring_length(getstring(js, b));
    unsigned int offset = js->stringcount;

    if (offset + la + lb + 1 > JS_STRINGSIZE)
    {

        throwerror(js, "Out of string memory");

        return internstring(js, "");

    }

    buffer_copy(js->strings + offset, getstring(js, a), la);
    buffer_copy(js->strings + offset + la, getstring(js, b), lb);

    js->strings[offset + la + lb] = '\0';
    js->stringcount += la + lb + 1;

    return offset;

}

static unsigned int strictequal(struct js_engine *js, struct js_value *a, struct js_value *b)
{

    if (a->type != b->type)
        return 0;

    if (a->type == JS_STRING)
        return cstring_match(getstring(js, a->number), getstring(js, b->number));

    if (a->type == JS_UNDEFINED || a->type == JS_NULL)
        return 1;

    return a->number == b->number;

}

static unsigned int looseequal(struct js_engine *js, struct js_value *a, struct js_value *b)
{

    if (a->type <= JS_NULL && b->type <= JS_NULL)
        return 1;

    if (a->type <= JS_NULL || b->type <= JS_NULL)
        return 0;

    if (a->type != b->type && a->type != JS_OBJECT && b->type != JS_OBJECT)
        return tonumber(js, a) == tonumber(js, b);

    return strictequal(js, a, b);

}

static void nexttoken(struct parser *parser)
{

    char *source = parser->source;
    struct token *token = &parser->token;
    unsigned int i;

    while (parser->offset < parser->count)
    {

        if (isspace(source[parser->offset]))
        {

            parser->offset++;

        }

        else if (parser->offset + 1 < parser->count && source[parser->offset] == '/' && source[parser->offset + 1] == '/')
        {

            while (parser->offset < parser->count && source[parser->offset] != '\n')
                parser->offset++;

        }

        else if (parser->offset + 1 < parser->count && source[parser->offset] == '/' && source[parser->offset + 1] == '*')
        {

            parser->offset += 2;

            while (parser->offset + 1 < parser->count && !(source[parser->offset] == '*' && source[parser->offset + 1] == '/'))
                parser->offset++;

            parser->offset += 2;

        }

        else
        {

            break;

        }

    }

    token->start = parser->offset;
    token->length = 0;
    token->value = 0;

    if (parser->offset >= parser->count)
    {

        token->type = TOKEN_END;

        return;

    }

    if (cstring_isdigit(source[parser->offset]) || (source[parser->offset] == '.' && parser->offset + 1 < parser->count && cstring_isdigit(source[parser->offset + 1])))
    {

        unsigned int base = 10;

        if (source[parser->offset] == '0' && parser->offset + 1 < parser->count && (source[parser->offset + 1] == 'x' || source[parser->offset + 1] == 'X'))
        {

            base = 16;
            parser->offset += 2;

        }

        while (parser->offset < parser->count)
        {

            char c = source[parser->offset];
            int digit = (c >= '0' && c <= '9') ? c - '0' : (base == 16 && c >= 'a' && c <= 'f') ? c - 'a' + 10 : (base == 16 && c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;

            if (digit < 0)
                break;

            token->value = token->value * (int)base + digit;
            parser->offset++;

        }

        while (parser->offset < parser->count && (cstring_isdigit(source[parser->offset]) || source[parser->offset] == '.' || source[parser->offset] == 'e' || source[parser->offset] == 'E'))
            parser->offset++;

        token->type = TOKEN_NUMBER;
        token->length = parser->offset - token->start;

        return;

    }

    if (source[parser->offset] == '"' || source[parser->offset] == '\'' || source[parser->offset] == '`')
    {

        char quote = source[parser->offset++];
        unsigned int start = parser->js->stringcount;

        while (parser->offset < parser->count && source[parser->offset] != quote)
        {

            char c = source[parser->offset++];

            if (c == '\\' && parser->offset < parser->count)
            {

                c = source[parser->offset++];

                if (c == 'n')
                    c = '\n';
                else if (c == 't')
                    c = '\t';
                else if (c == 'r')
                    c = '\r';

            }

            if (parser->js->stringcount + 2 >= JS_STRINGSIZE)
            {

                parser->failed = 1;

                return;

            }

            parser->js->strings[parser->js->stringcount++] = c;

        }

        parser->offset++;
        parser->js->strings[parser->js->stringcount++] = '\0';
        token->type = TOKEN_STRING;
        token->value = start;
        token->length = parser->offset - token->start;

        return;

    }

    if (isnamestart(source[parser->offset]))
    {

        while (parser->offset < parser->count && isnamechar(source[parser->offset]))
            parser->offset++;

        token->type = TOKEN_NAME;
        token->length = parser->offset - token->start;
        token->value = intern(parser->js, source + token->start, token->length);

        return;

    }

    token->type = TOKEN_PUNCTUATOR;

    for (i = 0; i < PUNCTUATORS; i++)
    {

        unsigned int length = cstring_length(punctuators[i]);

        if (parser->offset + length <= parser->count && buffer_match(source + parser->offset, punctuators[i], length))
        {

            token->length = length;
            parser->offset += length;

            return;

        }

    }

    if (buffer_findbyte(singles, cstring_length(singles), source[parser->offset]) < cstring_length(singles))
    {

        token->length = 1;
        parser->offset++;

        return;

    }

    parser->failed = 1;
    token->type = TOKEN_END;

}

static unsigned int ispunctuator(struct parser *parser, char *text)
{

    unsigned int length = cstring_length(text);

    return parser->token.type == TOKEN_PUNCTUATOR && parser->token.length == length && buffer_match(parser->source + parser->token.start, text, length);

}

static unsigned int isword(struct parser *parser, char *text)
{

    return parser->token.type == TOKEN_NAME && cstring_match(getstring(parser->js, parser->token.value), text);

}

static unsigned int accept(struct parser *parser, char *text)
{

    if (!ispunctuator(parser, text))
        return 0;

    nexttoken(parser);

    return 1;

}

static void expect(struct parser *parser, char *text)
{

    if (!accept(parser, text))
        parser->failed = 1;

}

static unsigned int newnode(struct parser *parser, unsigned int type)
{

    struct js_engine *js = parser->js;
    struct js_node *node;

    if (js->nodecount >= JS_NODES)
    {

        parser->failed = 1;

        return 0;

    }

    node = &js->nodes[js->nodecount];

    buffer_clear(node, sizeof (struct js_node));

    node->type = type;

    return js->nodecount++;

}

static unsigned int appendnode(struct parser *parser, unsigned int last, unsigned int node)
{

    if (last)
        parser->js->nodes[last].next = node;

    return node;

}

static unsigned int parsename(struct parser *parser)
{

    int name = parser->token.value;

    if (parser->token.type != TOKEN_NAME)
    {

        parser->failed = 1;

        return 0;

    }

    nexttoken(parser);

    return name;

}

static unsigned int parseparameters(struct parser *parser)
{

    unsigned int first = 0;
    unsigned int last = 0;

    expect(parser, "(");

    while (!parser->failed && !ispunctuator(parser, ")"))
    {

        unsigned int node = newnode(parser, NODE_NAME);

        if (!node)
            break;

        parser->js->nodes[node].value = parsename(parser);

        if (accept(parser, "="))
            parser->js->nodes[node].a = parseassignment(parser);

        last = appendnode(parser, last, node);
        first = (first) ? first : node;

        if (!accept(parser, ","))
            break;

    }

    expect(parser, ")");

    return first;

}

static unsigned int parsefunction(struct parser *parser)
{

    unsigned int node = newnode(parser, NODE_FUNCTION);

    if (!node)
        return 0;

    if (parser->token.type == TOKEN_NAME)
        parser->js->nodes[node].value = parsename(parser);

    parser->js->nodes[node].a = parseparameters(parser);
    parser->js->nodes[node].b = parseblock(parser);

    return node;

}

static unsigned int parsearrowbody(struct parser *parser, unsigned int node)
{

    if (ispunctuator(parser, "{"))
    {

        parser->js->nodes[node].b = parseblock(parser);

    }

    else
    {

        parser->js->nodes[node].op = 1;
        parser->js->nodes[node].b = parseassignment(parser);

    }

    return node;

}

static unsigned int tryarrow(struct parser *parser)
{

    unsigned int offset = parser->offset;
    unsigned int nodecount = parser->js->nodecount;
    struct token token;
    unsigned int parameters;

    buffer_copy(&token, &parser->token, sizeof (struct token));

    parameters = parseparameters(parser);

    if (!parser->failed && ispunctuator(parser, "=>"))
    {

        unsigned int node = newnode(parser, NODE_FUNCTION);

        nexttoken(parser);

        if (!node)
            return 0;

        parser->js->nodes[node].a = parameters;

        return parsearrowbody(parser, node);

    }

    parser->failed = 0;
    parser->offset = offset;
    parser->js->nodecount = nodecount;

    buffer_copy(&parser->token, &token, sizeof (struct token));

    return 0;

}

static unsigned int parseprimary(struct parser *parser)
{

    struct js_engine *js = parser->js;
    unsigned int node;

    if (parser->token.type == TOKEN_NUMBER || parser->token.type == TOKEN_STRING)
    {

        node = newnode(parser, (parser->token.type == TOKEN_NUMBER) ? NODE_NUMBER : NODE_STRING);

        if (node)
            js->nodes[node].value = parser->token.value;

        nexttoken(parser);

        return node;

    }

    if (isword(parser, "function"))
    {

        nexttoken(parser);

        return parsefunction(parser);

    }

    if (parser->token.type == TOKEN_NAME)
    {

        unsigned int name = parsename(parser);

        if (ispunctuator(parser, "=>"))
        {

            unsigned int parameter = newnode(parser, NODE_NAME);

            nexttoken(parser);

            node = newnode(parser, NODE_FUNCTION);

            if (!node || !parameter)
                return 0;

            js->nodes[parameter].value = name;
            js->nodes[node].a = parameter;

            return parsearrowbody(parser, node);

        }

        node = newnode(parser, NODE_NAME);

        if (node)
            js->nodes[node].value = name;

        return node;

    }

    if (ispunctuator(parser, "("))
    {

        node = tryarrow(parser);

        if (node)
            return node;

        nexttoken(parser);

        node = parseexpression(parser);

        expect(parser, ")");

        return node;

    }

    if (accept(parser, "["))
    {

        unsigned int last = 0;

        node = newnode(parser, NODE_ARRAY);

        while (node && !parser->failed && !ispunctuator(parser, "]"))
        {

            last = appendnode(parser, last, parseassignment(parser));

            if (!js->nodes[node].a)
                js->nodes[node].a = last;

            if (!accept(parser, ","))
                break;

        }

        expect(parser, "]");

        return node;

    }

    if (accept(parser, "{"))
    {

        unsigned int last = 0;

        node = newnode(parser, NODE_OBJECT);

        while (node && !parser->failed && !ispunctuator(parser, "}"))
        {

            unsigned int property = newnode(parser, NODE_PROPERTY);

            if (!property)
                break;

            if (parser->token.type == TOKEN_STRING)
                js->nodes[property].value = intern(js, getstring(js, parser->token.value), cstring_length(getstring(js, parser->token.value)));
            else if (parser->token.type == TOKEN_NUMBER)
                js->nodes[property].value = numberstring(js, parser->token.value);
            else if (parser->token.type == TOKEN_NAME)
                js->nodes[property].value = parser->token.value;
            else
                parser->failed = 1;

            nexttoken(parser);

            if (accept(parser, ":"))
            {

                js->nodes[property].a = parseassignment(parser);

            }

            else if (ispunctuator(parser, "("))
            {

                unsigned int function = newnode(parser, NODE_FUNCTION);

                if (function)
                {

                    js->nodes[function].a = parseparameters(parser);
                    js->nodes[function].b = parseblock(parser);
                    js->nodes[property].a = function;

                }

            }

            else
            {

                unsigned int name = newnode(parser, NODE_NAME);

                if (name)
                    js->nodes[name].value = js->nodes[property].value;

                js->nodes[property].a = name;

            }

            last = appendnode(parser, last, property);

            if (!js->nodes[node].a)
                js->nodes[node].a = last;

            if (!accept(parser, ","))
                break;

        }

        expect(parser, "}");

        return node;

    }

    parser->failed = 1;

    return 0;

}

static unsigned int parsearguments(struct parser *parser)
{

    unsigned int first = 0;
    unsigned int last = 0;

    while (!parser->failed && !ispunctuator(parser, ")"))
    {

        last = appendnode(parser, last, parseassignment(parser));
        first = (first) ? first : last;

        if (!accept(parser, ","))
            break;

    }

    expect(parser, ")");

    return first;

}

static unsigned int parsepostfix(struct parser *parser)
{

    struct js_engine *js = parser->js;
    unsigned int node;

    if (isword(parser, "new"))
        nexttoken(parser);

    node = parseprimary(parser);

    while (!parser->failed)
    {

        unsigned int next;

        if (accept(parser, ".") || accept(parser, "?."))
        {

            next = newnode(parser, NODE_MEMBER);

            if (!next)
                break;

            js->nodes[next].a = node;
            js->nodes[next].value = parsename(parser);
            node = next;

        }

        else if (accept(parser, "["))
        {

            next = newnode(parser, NODE_INDEX);

            if (!next)
                break;

            js->nodes[next].a = node;
            js->nodes[next].b = parseexpression(parser);
            node = next;

            expect(parser, "]");

        }

        else if (accept(parser, "("))
        {

            next = newnode(parser, NODE_CALL);

            if (!next)
                break;

            js->nodes[next].a = node;
            js->nodes[next].b = parsearguments(parser);
            node = next;

        }

        else
        {

            break;

        }

    }

    if (ispunctuator(parser, "++") || ispunctuator(parser, "--"))
    {

        unsigned int update = newnode(parser, NODE_UPDATE);

        if (update)
        {

            js->nodes[update].op = (ispunctuator(parser, "++")) ? OP_INCREMENT : OP_DECREMENT;
            js->nodes[update].a = node;

        }

        nexttoken(parser);

        return update;

    }

    return node;

}

static unsigned int parseunary(struct parser *parser)
{

    unsigned int op = 0;
    unsigned int node;

    if (ispunctuator(parser, "!"))
        op = OP_NOT;
    else if (ispunctuator(parser, "-"))
        op = OP_NEGATE;
    else if (ispunctuator(parser, "+"))
        op = OP_PLUS;
    else if (ispunctuator(parser, "~"))
        op = OP_BITNOT;
    else if (isword(parser, "typeof"))
        op = OP_TYPEOF;
    else if (isword(parser, "void") || isword(parser, "delete"))
        op = OP_VOID;

    if (op)
    {

        nexttoken(parser);

        node = newnode(parser, NODE_UNARY);

        if (node)
        {

            parser->js->nodes[node].op = op;
            parser->js->nodes[node].a = parseunary(parser);

        }

        return node;

    }

    if (ispunctuator(parser, "++") || ispunctuator(parser, "--"))
    {

        op = (ispunctuator(parser, "++")) ? OP_INCREMENT : OP_DECREMENT;

        nexttoken(parser);

        node = newnode(parser, NODE_UPDATE);

        if (node)
        {

            parser->js->nodes[node].op = op;
            parser->js->nodes[node].value = 1;
            parser->js->nodes[node].a = parseunary(parser);

        }

        return node;

    }

    return parsepostfix(parser);

}

static struct binary *findbinary(struct parser *parser)
{

    unsigned int i;

    if (parser->token.type != TOKEN_PUNCTUATOR)
        return 0;

    for (i = 0; i < BINARIES; i++)
    {

        if (ispunctuator(parser, binaries[i].text))
            return &binaries[i];

    }

    return 0;

}

static unsigned int parsebinary(struct parser *parser, unsigned int minimum)
{

    unsigned int left = parseunary(parser);

    while (!parser->failed)
    {

        struct binary *binary = findbinary(parser);
        unsigned int node;

        if (!binary || binary->precedence <= minimum)
            break;

        nexttoken(parser);

        node = newnode(parser, (binary->logical) ? NODE_LOGICAL : NODE_BINARY);

        if (!node)
            break;

        parser->js->nodes[node].op = binary->op;
        parser->js->nodes[node].a = left;
        parser->js->nodes[node].b = parsebinary(parser, binary->precedence);
        left = node;

    }

    return left;

}

static unsigned int parseassignment(struct parser *parser)
{

    struct js_engine *js = parser->js;
    unsigned int node = parsebinary(parser, 0);
    unsigned int op = 0;

    if (accept(parser, "?"))
    {

        unsigned int conditional = newnode(parser, NODE_CONDITIONAL);

        if (!conditional)
            return 0;

        js->nodes[conditional].a = node;
        js->nodes[conditional].b = parseassignment(parser);

        expect(parser, ":");

        js->nodes[conditional].c = parseassignment(parser);

        return conditional;

    }

    if (ispunctuator(parser, "="))
        op = OP_ASSIGN;
    else if (ispunctuator(parser, "+="))
        op = OP_ADD;
    else if (ispunctuator(parser, "-="))
        op = OP_SUBTRACT;
    else if (ispunctuator(parser, "*="))
        op = OP_MULTIPLY;
    else if (ispunctuator(parser, "/="))
        op = OP_DIVIDE;
    else if (ispunctuator(parser, "%="))
        op = OP_MODULO;
    else if (ispunctuator(parser, "|="))
        op = OP_BITOR;
    else if (ispunctuator(parser, "&="))
        op = OP_BITAND;

    if (op)
    {

        unsigned int assign;

        nexttoken(parser);

        assign = newnode(parser, NODE_ASSIGN);

        if (!assign)
            return 0;

        js->nodes[assign].op = op;
        js->nodes[assign].a = node;
        js->nodes[assign].b = parseassignment(parser);

        return assign;

    }

    return node;

}

static unsigned int parseexpression(struct parser *parser)
{

    unsigned int node = parseassignment(parser);

    while (!parser->failed && accept(parser, ","))
        node = parseassignment(parser);

    return node;

}

static void endstatement(struct parser *parser)
{

    accept(parser, ";");

}

static unsigned int parsevariables(struct parser *parser)
{

    unsigned int node = newnode(parser, NODE_VAR);
    unsigned int last = 0;

    nexttoken(parser);

    while (node && !parser->failed)
    {

        unsigned int declarator = newnode(parser, NODE_DECLARATOR);

        if (!declarator)
            break;

        if (ispunctuator(parser, "[") || ispunctuator(parser, "{"))
        {

            unsigned int pattern = (ispunctuator(parser, "[")) ? PATTERN_ARRAY : PATTERN_OBJECT;
            char *close = (pattern == PATTERN_ARRAY) ? "]" : "}";
            unsigned int lastname = 0;

            nexttoken(parser);

            parser->js->nodes[declarator].c = pattern;

            while (!parser->failed && !ispunctuator(parser, close))
            {

                unsigned int name = newnode(parser, NODE_NAME);

                if (!name)
                    break;

                if (!(pattern == PATTERN_ARRAY && ispunctuator(parser, ",")))
                    parser->js->nodes[name].value = parsename(parser);

                lastname = appendnode(parser, lastname, name);

                if (!parser->js->nodes[declarator].b)
                    parser->js->nodes[declarator].b = lastname;

                if (!accept(parser, ","))
                    break;

            }

            expect(parser, close);

        }

        else
        {

            parser->js->nodes[declarator].value = parsename(parser);

        }

        if (!last && (isword(parser, "of") || isword(parser, "in")))
        {

            parser->js->nodes[node].a = declarator;
            parser->js->nodes[node].value = (isword(parser, "of")) ? LOOP_OF : LOOP_IN;

            break;

        }

        if (accept(parser, "="))
            parser->js->nodes[declarator].a = parseassignment(parser);

        last = appendnode(parser, last, declarator);

        if (!parser->js->nodes[node].a)
            parser->js->nodes[node].a = last;

        if (!accept(parser, ","))
            break;

    }

    return node;

}

static unsigned int parselist(struct parser *parser, char *end)
{

    unsigned int first = 0;
    unsigned int last = 0;

    while (!parser->failed && parser->token.type != TOKEN_END && !(end && ispunctuator(parser, end)))
    {

        unsigned int node = parsestatement(parser);

        if (node)
        {

            last = appendnode(parser, last, node);
            first = (first) ? first : node;

        }

    }

    return first;

}

static unsigned int parseblock(struct parser *parser)
{

    unsigned int node = newnode(parser, NODE_BLOCK);

    expect(parser, "{");

    if (node)
        parser->js->nodes[node].a = parselist(parser, "}");

    expect(parser, "}");

    return node;

}

static unsigned int parsestatement(struct parser *parser)
{

    struct js_engine *js = parser->js;
    unsigned int node;

    if (ispunctuator(parser, "{"))
        return parseblock(parser);

    if (accept(parser, ";"))
        return newnode(parser, NODE_EMPTY);

    if (isword(parser, "var") || isword(parser, "let") || isword(parser, "const"))
    {

        node = parsevariables(parser);

        endstatement(parser);

        return node;

    }

    if (isword(parser, "function"))
    {

        nexttoken(parser);

        node = newnode(parser, NODE_DECLARATION);

        if (node)
            js->nodes[node].a = parsefunction(parser);

        return node;

    }

    if (isword(parser, "return"))
    {

        nexttoken(parser);

        node = newnode(parser, NODE_RETURN);

        if (node && !ispunctuator(parser, ";") && !ispunctuator(parser, "}") && parser->token.type != TOKEN_END)
            js->nodes[node].a = parseexpression(parser);

        endstatement(parser);

        return node;

    }

    if (isword(parser, "if"))
    {

        nexttoken(parser);
        expect(parser, "(");

        node = newnode(parser, NODE_IF);

        if (!node)
            return 0;

        js->nodes[node].a = parseexpression(parser);

        expect(parser, ")");

        js->nodes[node].b = parsestatement(parser);

        if (isword(parser, "else"))
        {

            nexttoken(parser);

            js->nodes[node].c = parsestatement(parser);

        }

        return node;

    }

    if (isword(parser, "while"))
    {

        nexttoken(parser);
        expect(parser, "(");

        node = newnode(parser, NODE_WHILE);

        if (!node)
            return 0;

        js->nodes[node].a = parseexpression(parser);

        expect(parser, ")");

        js->nodes[node].b = parsestatement(parser);

        return node;

    }

    if (isword(parser, "do"))
    {

        nexttoken(parser);

        node = newnode(parser, NODE_DO);

        if (!node)
            return 0;

        js->nodes[node].b = parsestatement(parser);

        if (!isword(parser, "while"))
            parser->failed = 1;

        nexttoken(parser);
        expect(parser, "(");

        js->nodes[node].a = parseexpression(parser);

        expect(parser, ")");
        endstatement(parser);

        return node;

    }

    if (isword(parser, "for"))
    {

        nexttoken(parser);
        expect(parser, "(");

        node = newnode(parser, NODE_FOR);

        if (!node)
            return 0;

        if (isword(parser, "var") || isword(parser, "let") || isword(parser, "const"))
            js->nodes[node].a = parsevariables(parser);
        else if (!ispunctuator(parser, ";"))
            js->nodes[node].a = parseexpression(parser);

        if (isword(parser, "of") || isword(parser, "in"))
        {

            js->nodes[node].type = NODE_FOREACH;
            js->nodes[node].c = (isword(parser, "of")) ? LOOP_OF : LOOP_IN;

            nexttoken(parser);

            js->nodes[node].b = parseassignment(parser);

            expect(parser, ")");

            js->nodes[node].value = parsestatement(parser);

            return node;

        }

        expect(parser, ";");

        if (!ispunctuator(parser, ";"))
            js->nodes[node].b = parseexpression(parser);

        expect(parser, ";");

        if (!ispunctuator(parser, ")"))
            js->nodes[node].c = parseexpression(parser);

        expect(parser, ")");

        js->nodes[node].value = parsestatement(parser);

        return node;

    }

    if (isword(parser, "switch"))
    {

        unsigned int last = 0;

        nexttoken(parser);
        expect(parser, "(");

        node = newnode(parser, NODE_SWITCH);

        if (!node)
            return 0;

        js->nodes[node].a = parseexpression(parser);

        expect(parser, ")");
        expect(parser, "{");

        while (!parser->failed && !ispunctuator(parser, "}") && parser->token.type != TOKEN_END)
        {

            unsigned int clause = newnode(parser, NODE_CASE);
            unsigned int statement = 0;

            if (!clause)
                break;

            if (isword(parser, "case"))
            {

                nexttoken(parser);

                js->nodes[clause].a = parseexpression(parser);

            }

            else if (isword(parser, "default"))
            {

                nexttoken(parser);

                js->nodes[clause].c = 1;

            }

            else
            {

                parser->failed = 1;

                break;

            }

            expect(parser, ":");

            while (!parser->failed && !isword(parser, "case") && !isword(parser, "default") && !ispunctuator(parser, "}") && parser->token.type != TOKEN_END)
            {

                unsigned int parsed = parsestatement(parser);

                if (!parsed)
                    break;

                statement = appendnode(parser, statement, parsed);

                if (!js->nodes[clause].b)
                    js->nodes[clause].b = statement;

            }

            last = appendnode(parser, last, clause);

            if (!js->nodes[node].b)
                js->nodes[node].b = last;

        }

        expect(parser, "}");

        return node;

    }

    if (isword(parser, "try"))
    {

        nexttoken(parser);

        node = newnode(parser, NODE_TRY);

        if (!node)
            return 0;

        js->nodes[node].a = parseblock(parser);

        if (isword(parser, "catch"))
        {

            nexttoken(parser);

            if (accept(parser, "("))
            {

                js->nodes[node].value = parsename(parser);

                expect(parser, ")");

            }

            js->nodes[node].b = parseblock(parser);

        }

        if (isword(parser, "finally"))
        {

            nexttoken(parser);

            js->nodes[node].c = parseblock(parser);

        }

        return node;

    }

    if (isword(parser, "throw"))
    {

        nexttoken(parser);

        node = newnode(parser, NODE_THROW);

        if (node)
            js->nodes[node].a = parseexpression(parser);

        endstatement(parser);

        return node;

    }

    if (isword(parser, "break") || isword(parser, "continue"))
    {

        node = newnode(parser, (isword(parser, "break")) ? NODE_BREAK : NODE_CONTINUE);

        nexttoken(parser);
        endstatement(parser);

        return node;

    }

    node = newnode(parser, NODE_EXPRESSION);

    if (node)
        js->nodes[node].a = parseexpression(parser);

    endstatement(parser);

    return node;

}

static unsigned int getelement(struct js_engine *js, unsigned int node)
{

    if (!node || node >= HTML_NODES || js->document->nodes[node].type != HTML_NODE_ELEMENT)
        return 0;

    if (!js->elements[node])
        js->elements[node] = newobject(js, KIND_ELEMENT, 0, node);

    return js->elements[node];

}

static struct js_value elementvalue(struct js_engine *js, unsigned int node)
{

    unsigned int object = getelement(js, node);

    return (object) ? value(JS_OBJECT, object) : value(JS_NULL, 0);

}

static unsigned int nativefor(struct js_engine *js, unsigned int id)
{

    return js->natives[id];

}

static char *attributename(char *name)
{

    if (cstring_match(name, "className"))
        return "class";

    if (cstring_match(name, "id") || cstring_match(name, "href") || cstring_match(name, "src") || cstring_match(name, "title") || cstring_match(name, "value") || cstring_match(name, "name") || cstring_match(name, "type") || cstring_match(name, "alt"))
        return name;

    return 0;

}

static unsigned int mediamatches(struct js_engine *js, char *query)
{

    unsigned int length = cstring_length(query);
    unsigned int i;

    for (i = 0; i + 4 <= length; i++)
    {

        if (buffer_match(query + i, "dark", 4))
            return js->dark;

        if (i + 5 <= length && buffer_match(query + i, "light", 5))
            return !js->dark;

    }

    return 0;

}

static struct js_value getproperty(struct js_engine *js, struct js_value *target, unsigned int name)
{

    char *text = getstring(js, name);
    struct js_object *object;
    struct js_property *property;

    if (target->type == JS_STRING)
    {

        if (cstring_match(text, "length"))
            return value(JS_NUMBER, cstring_length(getstring(js, target->number)));

        if (cstring_match(text, "indexOf"))
            return value(JS_OBJECT, nativefor(js, NATIVE_INDEXOF));

        if (cstring_match(text, "toUpperCase"))
            return value(JS_OBJECT, nativefor(js, NATIVE_TOUPPERCASE));

        if (cstring_match(text, "toLowerCase"))
            return value(JS_OBJECT, nativefor(js, NATIVE_TOLOWERCASE));

        return undefined();

    }

    if (target->type != JS_OBJECT)
    {

        throwerror(js, "Cannot read properties of undefined or null");

        return undefined();

    }

    object = &js->objects[target->number];

    switch (object->kind)
    {

    case KIND_ARRAY:
        if (cstring_match(text, "length"))
            return value(JS_NUMBER, object->data);

        if (cstring_match(text, "indexOf"))
            return value(JS_OBJECT, nativefor(js, NATIVE_INDEXOF));

        if (cstring_match(text, "push"))
            return value(JS_OBJECT, nativefor(js, NATIVE_PUSH));

        if (cstring_match(text, "join"))
            return value(JS_OBJECT, nativefor(js, NATIVE_JOIN));

        break;

    case KIND_ELEMENT:
        if (cstring_match(text, "setAttribute"))
            return value(JS_OBJECT, nativefor(js, NATIVE_SETATTRIBUTE));

        if (cstring_match(text, "getAttribute"))
            return value(JS_OBJECT, nativefor(js, NATIVE_GETATTRIBUTE));

        if (cstring_match(text, "removeAttribute"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVEATTRIBUTE));

        if (cstring_match(text, "hasAttribute"))
            return value(JS_OBJECT, nativefor(js, NATIVE_HASATTRIBUTE));

        if (cstring_match(text, "addEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_ADDEVENTLISTENER));

        if (cstring_match(text, "tagName"))
            return value(JS_STRING, internstring(js, html_getstring(js->document, js->document->nodes[object->data].name)));

        if (attributename(text))
        {

            char *attribute = html_getattribute(js->document, object->data, attributename(text));

            return (attribute) ? stringvalue(js, attribute) : value(JS_STRING, internstring(js, ""));

        }

        break;

    case KIND_DOCUMENT:
        if (cstring_match(text, "documentElement"))
            return elementvalue(js, js->document->html);

        if (cstring_match(text, "body"))
            return elementvalue(js, js->document->body);

        if (cstring_match(text, "head"))
            return elementvalue(js, js->document->head);

        if (cstring_match(text, "getElementById"))
            return value(JS_OBJECT, nativefor(js, NATIVE_GETELEMENTBYID));

        if (cstring_match(text, "querySelector"))
            return value(JS_OBJECT, nativefor(js, NATIVE_QUERYSELECTOR));

        if (cstring_match(text, "addEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_ADDEVENTLISTENER));

        break;

    case KIND_WINDOW:
        if (cstring_match(text, "addEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_ADDEVENTLISTENER));

        break;

    case KIND_STORAGE:
        if (cstring_match(text, "getItem"))
            return value(JS_OBJECT, nativefor(js, NATIVE_GETITEM));

        if (cstring_match(text, "setItem"))
            return value(JS_OBJECT, nativefor(js, NATIVE_SETITEM));

        if (cstring_match(text, "removeItem"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVEITEM));

        break;

    case KIND_MEDIA:
        if (cstring_match(text, "matches"))
            return value(JS_BOOLEAN, mediamatches(js, getstring(js, object->data)));

        if (cstring_match(text, "media"))
            return value(JS_STRING, object->data);

        if (cstring_match(text, "addEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_ADDEVENTLISTENER));

        if (cstring_match(text, "addListener") || cstring_match(text, "removeEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_NOOP));

        break;

    case KIND_CONSOLE:
        return value(JS_OBJECT, nativefor(js, NATIVE_NOOP));

    }

    property = findproperty(js, target->number, name);

    return (property) ? property->value : undefined();

}

static void setproperty(struct js_engine *js, struct js_value *target, unsigned int name, struct js_value *newvalue)
{

    struct js_object *object;

    if (target->type != JS_OBJECT)
    {

        if (target->type <= JS_NULL)
            throwerror(js, "Cannot set properties of undefined or null");

        return;

    }

    object = &js->objects[target->number];

    if (object->kind == KIND_ELEMENT && attributename(getstring(js, name)))
    {

        html_setattribute(js->document, object->data, attributename(getstring(js, name)), getstring(js, tostring(js, newvalue)));

        js->changed = 1;

        return;

    }

    if (object->kind == KIND_ARRAY && cstring_isdigit(getstring(js, name)[0]))
    {

        unsigned int index = parseinteger(getstring(js, name));

        if (index >= object->data)
            object->data = index + 1;

    }

    putproperty(js, target->number, name, newvalue);

}

static unsigned int findscope(struct js_engine *js, unsigned int scope, unsigned int name)
{

    while (scope)
    {

        if (findproperty(js, scope, name))
            return scope;

        scope = js->objects[scope].parent;

    }

    return 0;

}

static struct js_value lookup(struct js_engine *js, unsigned int scope, unsigned int name)
{

    char *text = getstring(js, name);
    unsigned int found = findscope(js, scope, name);

    if (found)
        return findproperty(js, found, name)->value;


    if (cstring_match(text, "undefined"))
        return undefined();

    if (cstring_match(text, "null"))
        return value(JS_NULL, 0);

    if (cstring_match(text, "true") || cstring_match(text, "false"))
        return value(JS_BOOLEAN, cstring_match(text, "true"));

    if (cstring_match(text, "this"))
        return value(JS_OBJECT, js->global);

    throwerror(js, "Variable is not defined");

    return undefined();

}

static void assignname(struct js_engine *js, unsigned int scope, unsigned int name, struct js_value *newvalue)
{

    unsigned int found = findscope(js, scope, name);

    putproperty(js, (found) ? found : js->global, name, newvalue);

}

static void declare(struct js_engine *js, unsigned int scope, unsigned int name, struct js_value *newvalue)
{

    putproperty(js, scope, name, newvalue);

}

static struct js_value newfunction(struct js_engine *js, unsigned int node, unsigned int scope)
{

    unsigned int object = newobject(js, KIND_FUNCTION, scope, node);

    if (!object)
    {

        throwerror(js, "Out of objects");

        return undefined();

    }

    return value(JS_OBJECT, object);

}

static void hoist(struct js_engine *js, unsigned int first, unsigned int scope)
{

    unsigned int statement;

    for (statement = first; statement; statement = js->nodes[statement].next)
    {

        if (js->nodes[statement].type == NODE_DECLARATION)
        {

            unsigned int function = js->nodes[statement].a;
            struct js_value created = newfunction(js, function, scope);

            declare(js, scope, js->nodes[function].value, &created);

        }

    }

}

static struct js_value callfunction(struct js_engine *js, struct call *call);

static struct js_value callnative(struct js_engine *js, struct call *call)
{

    struct js_object *native = &js->objects[call->function.number];
    struct js_value *self = &call->self;
    struct js_value *first = &call->arguments[0];
    struct js_value *second = &call->arguments[1];
    unsigned int kind = (self->type == JS_OBJECT) ? js->objects[self->number].kind : 0;
    unsigned int element = (kind == KIND_ELEMENT) ? js->objects[self->number].data : 0;
    unsigned int node;
    unsigned int i;

    switch (native->data)
    {

    case NATIVE_GETELEMENTBYID:
        for (node = 1; node < js->document->nodecount; node++)
        {

            char *id = (js->document->nodes[node].type == HTML_NODE_ELEMENT) ? html_getattribute(js->document, node, "id") : 0;

            if (id && cstring_match(id, getstring(js, tostring(js, first))))
                return elementvalue(js, node);

        }

        return value(JS_NULL, 0);

    case NATIVE_QUERYSELECTOR:
        {

            char *selector = getstring(js, tostring(js, first));

            for (node = 1; node < js->document->nodecount; node++)
            {

                struct html_node *candidate = &js->document->nodes[node];
                char *id = (candidate->type == HTML_NODE_ELEMENT) ? html_getattribute(js->document, node, "id") : 0;

                if (candidate->type != HTML_NODE_ELEMENT)
                    continue;

                if ((selector[0] == '#' && id && cstring_match(id, selector + 1)) || cstring_match(html_getstring(js->document, candidate->name), selector))
                    return elementvalue(js, node);

            }

        }

        return value(JS_NULL, 0);

    case NATIVE_ADDEVENTLISTENER:
        if (call->count >= 2 && isfunction(js, second) && js->listenercount < JS_LISTENERS)
        {

            struct js_listener *listener = &js->listeners[js->listenercount++];

            listener->node = (kind == KIND_MEDIA) ? JS_MEDIA : element;
            listener->type = internstring(js, getstring(js, tostring(js, first)));
            listener->function = second->number;

        }

        return undefined();

    case NATIVE_SETATTRIBUTE:
        if (element)
        {

            html_setattribute(js->document, element, getstring(js, tostring(js, first)), getstring(js, tostring(js, second)));

            js->changed = 1;

        }

        return undefined();

    case NATIVE_GETATTRIBUTE:
        {

            char *attribute = (element) ? html_getattribute(js->document, element, getstring(js, tostring(js, first))) : 0;

            return (attribute) ? stringvalue(js, attribute) : value(JS_NULL, 0);

        }

    case NATIVE_HASATTRIBUTE:
        return value(JS_BOOLEAN, element && html_getattribute(js->document, element, getstring(js, tostring(js, first))) != 0);

    case NATIVE_REMOVEATTRIBUTE:
        if (element)
        {

            html_removeattribute(js->document, element, getstring(js, tostring(js, first)));

            js->changed = 1;

        }

        return undefined();

    case NATIVE_MATCHMEDIA:
        return value(JS_OBJECT, newobject(js, KIND_MEDIA, 0, tostring(js, first)));

    case NATIVE_GETITEM:
        {

            struct js_property *property = findproperty(js, js->storage, internstring(js, getstring(js, tostring(js, first))));

            return (property) ? property->value : value(JS_NULL, 0);

        }

    case NATIVE_SETITEM:
        {

            struct js_value text = value(JS_STRING, tostring(js, second));

            putproperty(js, js->storage, internstring(js, getstring(js, tostring(js, first))), &text);

        }

        return undefined();

    case NATIVE_REMOVEITEM:
        {

            struct js_property *property = findproperty(js, js->storage, internstring(js, getstring(js, tostring(js, first))));

            if (property)
                property->value = value(JS_NULL, 0);

        }

        return undefined();

    case NATIVE_INDEXOF:
        if (self->type == JS_STRING)
        {

            char *haystack = getstring(js, self->number);
            char *needle = getstring(js, tostring(js, first));
            unsigned int length = cstring_length(needle);

            for (i = 0; i + length <= cstring_length(haystack); i++)
            {

                if (buffer_match(haystack + i, needle, length))
                    return value(JS_NUMBER, i);

            }

            return value(JS_NUMBER, -1);

        }

        if (self->type == JS_OBJECT && js->objects[self->number].kind == KIND_ARRAY)
        {

            for (i = 0; i < js->objects[self->number].data; i++)
            {

                struct js_property *property = findproperty(js, self->number, numberstring(js, i));

                if (property && strictequal(js, &property->value, first))
                    return value(JS_NUMBER, i);

            }

        }

        return value(JS_NUMBER, -1);

    case NATIVE_PUSH:
        if (self->type == JS_OBJECT && js->objects[self->number].kind == KIND_ARRAY)
        {

            for (i = 0; i < call->count; i++)
                setproperty(js, self, numberstring(js, js->objects[self->number].data), &call->arguments[i]);

            return value(JS_NUMBER, js->objects[self->number].data);

        }

        return undefined();

    case NATIVE_JOIN:
        if (self->type == JS_OBJECT && js->objects[self->number].kind == KIND_ARRAY)
            return value(JS_STRING, joinarray(js, self->number, (call->count) ? getstring(js, tostring(js, first)) : ","));

        return undefined();

    case NATIVE_TOUPPERCASE:
    case NATIVE_TOLOWERCASE:
        if (self->type == JS_STRING)
        {

            struct js_value result = stringvalue(js, getstring(js, self->number));
            char *text = getstring(js, result.number);

            for (i = 0; text[i]; i++)
            {

                if (native->data == NATIVE_TOUPPERCASE && text[i] >= 'a' && text[i] <= 'z')
                    text[i] -= 32;

                if (native->data == NATIVE_TOLOWERCASE && text[i] >= 'A' && text[i] <= 'Z')
                    text[i] += 32;

            }

            return result;

        }

        return undefined();

    }

    return undefined();

}

static struct js_value callfunction(struct js_engine *js, struct call *call)
{

    struct js_object *function;
    struct js_node *node;
    struct completion completion;
    struct js_value result;
    unsigned int scope;
    unsigned int parameter;
    unsigned int i = 0;

    if (!isfunction(js, &call->function))
    {

        throwerror(js, "Value is not a function");

        return undefined();

    }

    function = &js->objects[call->function.number];

    if (function->kind == KIND_NATIVE)
        return callnative(js, call);

    if (js->depth >= DEPTHLIMIT)
    {

        throwerror(js, "Too much recursion");

        return undefined();

    }

    node = &js->nodes[function->data];
    scope = newobject(js, KIND_SCOPE, function->parent, 0);

    if (!scope)
    {

        throwerror(js, "Out of objects");

        return undefined();

    }

    declare(js, scope, internstring(js, "this"), &call->self);

    for (parameter = node->a; parameter; parameter = js->nodes[parameter].next)
    {

        struct js_value argument = undefined();

        if (i < call->count)
            copyvalue(&argument, &call->arguments[i]);

        if (argument.type == JS_UNDEFINED && js->nodes[parameter].a)
            argument = evaluate(js, js->nodes[parameter].a, scope);

        declare(js, scope, js->nodes[parameter].value, &argument);

        i++;

    }

    js->depth++;

    if (node->op)
    {

        result = evaluate(js, node->b, scope);

    }

    else
    {

        completion.type = COMPLETION_NORMAL;

        execlist(js, js->nodes[node->b].a, scope, &completion);

        result = (completion.type == COMPLETION_RETURN) ? completion.value : undefined();

    }

    js->depth--;

    return result;

}

static void collectarguments(struct js_engine *js, struct call *call, unsigned int first, unsigned int scope)
{

    unsigned int argument;

    call->count = 0;

    for (argument = first; argument && !js->throwing; argument = js->nodes[argument].next)
    {

        struct js_value evaluated = evaluate(js, argument, scope);

        if (call->count < ARGUMENTS)
            copyvalue(&call->arguments[call->count++], &evaluated);

    }

    for (argument = call->count; argument < ARGUMENTS; argument++)
        call->arguments[argument] = undefined();

}

static unsigned int propertyname(struct js_engine *js, unsigned int node, unsigned int scope)
{

    struct js_node *member = &js->nodes[node];
    struct js_value key;

    if (member->type == NODE_MEMBER)
        return member->value;

    key = evaluate(js, member->b, scope);

    return internstring(js, getstring(js, tostring(js, &key)));

}

static int arithmetic(unsigned int op, int a, int b)
{

    switch (op)
    {

    case OP_SUBTRACT:
        return a - b;

    case OP_MULTIPLY:
        return a * b;

    case OP_DIVIDE:
        return (b) ? a / b : 0;

    case OP_MODULO:
        return (b) ? a % b : 0;

    case OP_BITAND:
        return a & b;

    case OP_BITOR:
        return a | b;

    case OP_BITXOR:
        return a ^ b;

    case OP_SHIFTLEFT:
        return a << (b & 31);

    case OP_SHIFTRIGHT:
        return a >> (b & 31);

    }

    return a + b;

}

static struct js_value combine(struct js_engine *js, unsigned int op, struct js_value *a, struct js_value *b)
{

    switch (op)
    {

    case OP_ADD:
        if (a->type == JS_STRING || b->type == JS_STRING || a->type == JS_OBJECT || b->type == JS_OBJECT)
            return value(JS_STRING, concat(js, tostring(js, a), tostring(js, b)));

        return value(JS_NUMBER, tonumber(js, a) + tonumber(js, b));

    case OP_EQUAL:
        return value(JS_BOOLEAN, looseequal(js, a, b));

    case OP_NOTEQUAL:
        return value(JS_BOOLEAN, !looseequal(js, a, b));

    case OP_STRICTEQUAL:
        return value(JS_BOOLEAN, strictequal(js, a, b));

    case OP_STRICTNOTEQUAL:
        return value(JS_BOOLEAN, !strictequal(js, a, b));

    case OP_LESS:
    case OP_GREATER:
    case OP_LESSEQUAL:
    case OP_GREATEREQUAL:
        {

            int x = tonumber(js, a);
            int y = tonumber(js, b);

            if (op == OP_LESS)
                return value(JS_BOOLEAN, x < y);

            if (op == OP_GREATER)
                return value(JS_BOOLEAN, x > y);

            if (op == OP_LESSEQUAL)
                return value(JS_BOOLEAN, x <= y);

            return value(JS_BOOLEAN, x >= y);

        }

    }

    return value(JS_NUMBER, arithmetic(op, tonumber(js, a), tonumber(js, b)));

}

static void store(struct js_engine *js, unsigned int target, unsigned int scope, struct js_value *newvalue)
{

    struct js_node *node = &js->nodes[target];

    if (node->type == NODE_NAME)
    {

        assignname(js, scope, node->value, newvalue);

    }

    else if (node->type == NODE_MEMBER || node->type == NODE_INDEX)
    {

        struct js_value object = evaluate(js, node->a, scope);
        unsigned int name = propertyname(js, target, scope);

        if (!js->throwing)
            setproperty(js, &object, name, newvalue);

    }

    else
    {

        throwerror(js, "Invalid assignment target");

    }

}

static struct js_value typeofvalue(struct js_engine *js, struct js_value *operand)
{

    char *name = "undefined";

    switch (operand->type)
    {

    case JS_NULL:
    case JS_OBJECT:
        name = (isfunction(js, operand)) ? "function" : "object";

        break;

    case JS_BOOLEAN:
        name = "boolean";

        break;

    case JS_NUMBER:
        name = "number";

        break;

    case JS_STRING:
        name = "string";

        break;

    }

    return value(JS_STRING, internstring(js, name));

}

static struct js_value evaluatecall(struct js_engine *js, struct js_node *node, unsigned int scope)
{

    struct js_node *callee = &js->nodes[node->a];
    struct call call;

    call.self = value(JS_OBJECT, js->global);

    if (callee->type == NODE_MEMBER || callee->type == NODE_INDEX)
    {

        unsigned int name;

        call.self = evaluate(js, callee->a, scope);
        name = propertyname(js, node->a, scope);

        if (js->throwing)
            return undefined();

        call.function = getproperty(js, &call.self, name);

    }

    else
    {

        call.function = evaluate(js, node->a, scope);

    }

    if (js->throwing)
        return undefined();

    collectarguments(js, &call, node->b, scope);

    if (js->throwing)
        return undefined();

    return callfunction(js, &call);

}

static struct js_value evaluate(struct js_engine *js, unsigned int index, unsigned int scope)
{

    struct js_node *node = &js->nodes[index];
    struct js_value a;
    struct js_value b;
    unsigned int child;
    unsigned int object;
    unsigned int count;

    if (!index || js->throwing)
        return undefined();

    if (++js->steps > STEPLIMIT)
    {

        throwerror(js, "Script took too long");

        return undefined();

    }

    switch (node->type)
    {

    case NODE_NUMBER:
        return value(JS_NUMBER, node->value);

    case NODE_STRING:
        return value(JS_STRING, node->value);

    case NODE_NAME:
        return lookup(js, scope, node->value);

    case NODE_FUNCTION:
        return newfunction(js, index, scope);

    case NODE_ARRAY:
        object = newobject(js, KIND_ARRAY, 0, 0);
        count = 0;

        for (child = node->a; child && object; child = js->nodes[child].next)
        {

            a = evaluate(js, child, scope);
            b = value(JS_OBJECT, object);

            setproperty(js, &b, numberstring(js, count++), &a);

        }

        return (object) ? value(JS_OBJECT, object) : undefined();

    case NODE_OBJECT:
        object = newobject(js, KIND_PLAIN, 0, 0);

        for (child = node->a; child && object; child = js->nodes[child].next)
        {

            a = evaluate(js, js->nodes[child].a, scope);

            putproperty(js, object, js->nodes[child].value, &a);

        }

        return (object) ? value(JS_OBJECT, object) : undefined();

    case NODE_MEMBER:
    case NODE_INDEX:
        a = evaluate(js, node->a, scope);
        child = propertyname(js, index, scope);

        return (js->throwing) ? undefined() : getproperty(js, &a, child);

    case NODE_CALL:
        return evaluatecall(js, node, scope);

    case NODE_ASSIGN:
        if (node->op == OP_ASSIGN)
        {

            b = evaluate(js, node->b, scope);

        }

        else
        {

            a = evaluate(js, node->a, scope);
            b = evaluate(js, node->b, scope);
            b = combine(js, node->op, &a, &b);

        }

        if (!js->throwing)
            store(js, node->a, scope, &b);

        return b;

    case NODE_BINARY:
        a = evaluate(js, node->a, scope);
        b = evaluate(js, node->b, scope);

        return combine(js, node->op, &a, &b);

    case NODE_LOGICAL:
        a = evaluate(js, node->a, scope);

        if (node->op == OP_AND)
            return (truthy(js, &a)) ? evaluate(js, node->b, scope) : a;

        if (node->op == OP_OR)
            return (truthy(js, &a)) ? a : evaluate(js, node->b, scope);

        return (a.type > JS_NULL) ? a : evaluate(js, node->b, scope);

    case NODE_UNARY:
        if (node->op == OP_TYPEOF && js->nodes[node->a].type == NODE_NAME && !findscope(js, scope, js->nodes[node->a].value))
        {

            char *name = getstring(js, js->nodes[node->a].value);

            if (!cstring_match(name, "true") && !cstring_match(name, "false") && !cstring_match(name, "null") && !cstring_match(name, "this"))
                return value(JS_STRING, internstring(js, "undefined"));

        }

        a = evaluate(js, node->a, scope);

        switch (node->op)
        {

        case OP_NOT:
            return value(JS_BOOLEAN, !truthy(js, &a));

        case OP_NEGATE:
            return value(JS_NUMBER, -tonumber(js, &a));

        case OP_PLUS:
            return value(JS_NUMBER, tonumber(js, &a));

        case OP_BITNOT:
            return value(JS_NUMBER, ~tonumber(js, &a));

        case OP_TYPEOF:
            return typeofvalue(js, &a);

        }

        return undefined();

    case NODE_UPDATE:
        a = evaluate(js, node->a, scope);
        b = value(JS_NUMBER, tonumber(js, &a) + ((node->op == OP_INCREMENT) ? 1 : -1));

        if (!js->throwing)
            store(js, node->a, scope, &b);

        return (node->value) ? b : value(JS_NUMBER, tonumber(js, &a));

    case NODE_CONDITIONAL:
        a = evaluate(js, node->a, scope);

        return evaluate(js, (truthy(js, &a)) ? node->b : node->c, scope);

    }

    return undefined();

}

static void bindpattern(struct js_engine *js, unsigned int scope, unsigned int declarator, struct js_value *source)
{

    struct js_node *node = &js->nodes[declarator];
    unsigned int name;
    unsigned int index = 0;

    if (!node->c)
    {

        declare(js, scope, node->value, source);

        return;

    }

    for (name = node->b; name && !js->throwing; name = js->nodes[name].next, index++)
    {

        struct js_value item;

        if (!js->nodes[name].value)
            continue;

        item = getproperty(js, source, (node->c == PATTERN_ARRAY) ? numberstring(js, index) : (unsigned int)js->nodes[name].value);

        declare(js, scope, js->nodes[name].value, &item);

    }

}

static unsigned int loopstep(struct js_engine *js, unsigned int body, unsigned int scope, struct completion *completion)
{

    completion->type = COMPLETION_NORMAL;

    execute(js, body, scope, completion);

    if (completion->type == COMPLETION_BREAK)
    {

        completion->type = COMPLETION_NORMAL;

        return 0;

    }

    if (completion->type == COMPLETION_CONTINUE)
        completion->type = COMPLETION_NORMAL;

    return completion->type == COMPLETION_NORMAL && !js->throwing;

}

static void bindloop(struct js_engine *js, struct js_node *node, unsigned int scope, struct js_value *item)
{

    if (js->nodes[node->a].type == NODE_VAR)
        bindpattern(js, scope, js->nodes[node->a].a, item);
    else
        store(js, node->a, scope, item);

}

static void executeforeach(struct js_engine *js, struct js_node *node, unsigned int scope, struct completion *completion)
{

    struct js_value source = evaluate(js, node->b, scope);
    struct js_value item;
    unsigned int i;

    if (js->throwing)
        return;

    if (node->c == LOOP_OF && source.type == JS_STRING)
    {

        for (i = 0; i < cstring_length(getstring(js, source.number)); i++)
        {

            item = value(JS_STRING, addstring(js, getstring(js, source.number) + i, 1));

            bindloop(js, node, scope, &item);

            if (!loopstep(js, (unsigned int)node->value, scope, completion))
                break;

        }

        return;

    }

    if (source.type != JS_OBJECT)
        return;

    if (node->c == LOOP_OF || js->objects[source.number].kind == KIND_ARRAY)
    {

        for (i = 0; js->objects[source.number].kind == KIND_ARRAY && i < js->objects[source.number].data; i++)
        {

            item = (node->c == LOOP_OF) ? getproperty(js, &source, numberstring(js, i)) : value(JS_STRING, numberstring(js, i));

            bindloop(js, node, scope, &item);

            if (!loopstep(js, (unsigned int)node->value, scope, completion))
                break;

        }

        return;

    }

    /* properties are kept newest first, so they are walked from the end to keep their order */
    for (i = 0;; i++)
    {

        unsigned int property = js->objects[source.number].property;
        unsigned int count = 0;
        unsigned int position;

        while (property)
        {

            count++;
            property = js->properties[property].next;

        }

        if (i >= count)
            break;

        property = js->objects[source.number].property;

        for (position = 0; position + i + 1 < count; position++)
            property = js->properties[property].next;

        item = value(JS_STRING, js->properties[property].name);

        bindloop(js, node, scope, &item);

        if (!loopstep(js, (unsigned int)node->value, scope, completion))
            break;

    }

}

static void executeswitch(struct js_engine *js, struct js_node *node, unsigned int scope, struct completion *completion)
{

    struct js_value subject = evaluate(js, node->a, scope);
    unsigned int start = 0;
    unsigned int fallback = 0;
    unsigned int clause;

    for (clause = node->b; clause && !start && !js->throwing; clause = js->nodes[clause].next)
    {

        struct js_value test;

        if (js->nodes[clause].c)
        {

            fallback = clause;

            continue;

        }

        test = evaluate(js, js->nodes[clause].a, scope);

        if (strictequal(js, &subject, &test))
            start = clause;

    }

    if (js->throwing)
        return;

    for (clause = (start) ? start : fallback; clause && !js->throwing && completion->type == COMPLETION_NORMAL; clause = js->nodes[clause].next)
    {

        unsigned int statement;

        for (statement = js->nodes[clause].b; statement && !js->throwing && completion->type == COMPLETION_NORMAL; statement = js->nodes[statement].next)
            execute(js, statement, scope, completion);

    }

    if (completion->type == COMPLETION_BREAK)
        completion->type = COMPLETION_NORMAL;

}

static void execute(struct js_engine *js, unsigned int index, unsigned int scope, struct completion *completion)
{

    struct js_node *node = &js->nodes[index];
    struct js_value result;
    unsigned int child;

    if (!index || js->throwing)
        return;

    switch (node->type)
    {

    case NODE_EXPRESSION:
        evaluate(js, node->a, scope);

        break;

    case NODE_VAR:
        for (child = node->a; child && !js->throwing; child = js->nodes[child].next)
        {

            result = evaluate(js, js->nodes[child].a, scope);

            bindpattern(js, scope, child, &result);

        }

        break;

    case NODE_RETURN:
        completion->value = evaluate(js, node->a, scope);
        completion->type = COMPLETION_RETURN;

        break;

    case NODE_IF:
        result = evaluate(js, node->a, scope);

        if (!js->throwing)
            execute(js, (truthy(js, &result)) ? node->b : node->c, scope, completion);

        break;

    case NODE_BLOCK:
        execlist(js, node->a, scope, completion);

        break;

    case NODE_WHILE:
    case NODE_DO:
    case NODE_FOR:
        if (node->type == NODE_FOR)
            execute(js, (js->nodes[node->a].type == NODE_VAR) ? node->a : 0, scope, completion);

        if (node->type == NODE_FOR && js->nodes[node->a].type != NODE_VAR)
            evaluate(js, node->a, scope);

        while (!js->throwing)
        {

            if (node->type != NODE_DO || completion->type == COMPLETION_CONTINUE)
            {

                result = (node->type == NODE_FOR) ? ((node->b) ? evaluate(js, node->b, scope) : value(JS_BOOLEAN, 1)) : evaluate(js, node->a, scope);

                if (!truthy(js, &result) || js->throwing)
                    break;

            }

            completion->type = COMPLETION_NORMAL;

            execute(js, (node->type == NODE_FOR) ? (unsigned int)node->value : node->b, scope, completion);

            if (completion->type == COMPLETION_BREAK)
            {

                completion->type = COMPLETION_NORMAL;

                break;

            }

            if (completion->type == COMPLETION_RETURN)
                break;

            if (node->type == NODE_FOR)
                evaluate(js, node->c, scope);

            completion->type = COMPLETION_CONTINUE;

        }

        if (completion->type == COMPLETION_CONTINUE)
            completion->type = COMPLETION_NORMAL;

        break;

    case NODE_TRY:
        execute(js, node->a, scope, completion);

        if (js->throwing && node->b)
        {

            struct js_value thrown;

            copyvalue(&thrown, &js->thrown);

            js->throwing = 0;

            if (node->value)
                declare(js, scope, node->value, &thrown);

            execute(js, node->b, scope, completion);

        }

        if (node->c)
        {

            unsigned int throwing = js->throwing;

            js->throwing = 0;

            execute(js, node->c, scope, completion);

            js->throwing = js->throwing || throwing;

        }

        break;

    case NODE_THROW:
        result = evaluate(js, node->a, scope);

        if (!js->throwing)
        {

            js->throwing = 1;

            copyvalue(&js->thrown, &result);

        }

        break;

    case NODE_FOREACH:
        executeforeach(js, node, scope, completion);

        break;

    case NODE_SWITCH:
        executeswitch(js, node, scope, completion);

        break;

    case NODE_BREAK:
        completion->type = COMPLETION_BREAK;

        break;

    case NODE_CONTINUE:
        completion->type = COMPLETION_CONTINUE;

        break;

    }

}

static struct js_value execlist(struct js_engine *js, unsigned int first, unsigned int scope, struct completion *completion)
{

    unsigned int statement;

    hoist(js, first, scope);

    for (statement = first; statement && !js->throwing; statement = js->nodes[statement].next)
    {

        if (js->nodes[statement].type == NODE_DECLARATION)
            continue;

        execute(js, statement, scope, completion);

        if (completion->type != COMPLETION_NORMAL)
            break;

    }

    return undefined();

}

static void addglobal(struct js_engine *js, char *name, unsigned int object)
{

    struct js_value entry = value(JS_OBJECT, object);

    putproperty(js, js->global, internstring(js, name), &entry);

}

void js_reset(struct js_engine *js, struct html_document *document, unsigned int dark)
{

    unsigned int i;

    js->document = document;
    js->nodecount = 1;
    js->objectcount = 1;
    js->propertycount = 1;
    js->stringcount = 0;
    js->interncount = 0;
    js->listenercount = 0;
    js->dark = dark;
    js->steps = 0;
    js->depth = 0;
    js->throwing = 0;
    js->changed = 0;
    js->errors = 0;

    buffer_clear(js->elements, sizeof (unsigned short) * HTML_NODES);
    internstring(js, "");

    for (i = 0; i < JS_NATIVES; i++)
        js->natives[i] = newobject(js, KIND_NATIVE, 0, i);

    js->global = newobject(js, KIND_WINDOW, 0, 0);
    js->documentobject = newobject(js, KIND_DOCUMENT, 0, 0);
    js->storage = newobject(js, KIND_STORAGE, 0, 0);

    addglobal(js, "window", js->global);
    addglobal(js, "self", js->global);
    addglobal(js, "globalThis", js->global);
    addglobal(js, "document", js->documentobject);
    addglobal(js, "localStorage", js->storage);
    addglobal(js, "sessionStorage", js->storage);
    addglobal(js, "console", newobject(js, KIND_CONSOLE, 0, 0));
    addglobal(js, "matchMedia", js->natives[NATIVE_MATCHMEDIA]);

}

void js_run(struct js_engine *js, char *source, unsigned int count)
{

    struct parser parser;
    struct completion completion;
    unsigned int program;

    parser.js = js;
    parser.source = source;
    parser.count = count;
    parser.offset = 0;
    parser.failed = 0;

    nexttoken(&parser);

    program = parselist(&parser, 0);

    if (parser.failed)
    {

        js->errors++;

        return;

    }

    js->steps = 0;
    js->throwing = 0;
    completion.type = COMPLETION_NORMAL;

    execlist(js, program, js->global, &completion);

    if (js->throwing)
        js->errors++;

    js->throwing = 0;

}

unsigned int js_haslistener(struct js_engine *js, unsigned int node, char *type)
{

    unsigned int name = internstring(js, type);
    unsigned int i;

    for (i = 0; i < js->listenercount; i++)
    {

        if (js->listeners[i].node == node && js->listeners[i].type == name)
            return 1;

    }

    return 0;

}

void js_dispatch(struct js_engine *js, unsigned int node, char *type)
{

    unsigned int name = internstring(js, type);
    unsigned int count = js->listenercount;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct js_listener *listener = &js->listeners[i];

        if (listener->node == node && listener->type == name)
        {

            struct call call;
            unsigned int event = newobject(js, KIND_PLAIN, 0, 0);
            struct js_value eventtype = value(JS_STRING, name);
            unsigned int argument;

            if (event)
                putproperty(js, event, internstring(js, "type"), &eventtype);

            call.function = value(JS_OBJECT, listener->function);
            call.self = (node && node != JS_MEDIA) ? elementvalue(js, node) : value(JS_OBJECT, js->documentobject);
            call.count = 1;
            call.arguments[0] = (event) ? value(JS_OBJECT, event) : undefined();

            for (argument = 1; argument < ARGUMENTS; argument++)
                call.arguments[argument] = undefined();

            js->steps = 0;
            js->throwing = 0;

            callfunction(js, &call);

            if (js->throwing)
                js->errors++;

            js->throwing = 0;

        }

    }

}
