#include <fudge.h>
#include "html.h"
#include "js.h"

#define ARGUMENTS                       16
#define STEPLIMIT                       2000000
#define DEPTHLIMIT                      24
#define NUMBERSIZE                      16
#define PUNCTUATORS                     46
#define BINARIES                        23
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
#define NODE_SEQUENCE                   38
#define NODE_CLASS                      39
#define NODE_SPREAD                     40
#define NODE_REGEX                      41
#define FUNCTION_DERIVED                1
#define FUNCTION_ARROW                  2
#define FUNCTION_ARGUMENTS              4
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
#define OP_INSTANCEOF                   31
#define OP_IN                           32
#define TOKEN_END                       0
#define TOKEN_NUMBER                    1
#define TOKEN_STRING                    2
#define TOKEN_NAME                      3
#define TOKEN_PUNCTUATOR                4
#define TOKEN_TEMPLATE                  5
#define TOKEN_REGEX                     6
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
#define KIND_CLASSLIST                  12
#define KIND_STYLE                      13
#define KIND_BOUND                      14
#define KIND_REGEX                      15
#define KIND_DATASET                    16
#define KIND_MAP                        17
#define KIND_SET                        18
#define KIND_GETTER                     19
#define ACCESSOR_GET                    1
#define ACCESSOR_SET                    2
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
#define NATIVE_CLASSADD                 17
#define NATIVE_CLASSREMOVE              18
#define NATIVE_CLASSTOGGLE              19
#define NATIVE_CLASSCONTAINS            20
#define NATIVE_QUERYSELECTORALL         21
#define NATIVE_BYCLASS                  22
#define NATIVE_BYTAG                    23
#define NATIVE_FOREACH                  24
#define NATIVE_CLOSEST                  25
#define NATIVE_MATCHES                  26
#define NATIVE_STARTSWITH               27
#define NATIVE_ENDSWITH                 28
#define NATIVE_INCLUDES                 29
#define NATIVE_TRIM                     30
#define NATIVE_SPLIT                    31
#define NATIVE_SLICE                    32
#define NATIVE_REPLACE                  33
#define NATIVE_MAX                      34
#define NATIVE_MIN                      35
#define NATIVE_ROUND                    36
#define NATIVE_ABS                      37
#define NATIVE_SETTIMEOUT               38
#define NATIVE_CHARAT                   39
#define NATIVE_CREATEELEMENT            40
#define NATIVE_CREATETEXTNODE           41
#define NATIVE_APPEND                   42
#define NATIVE_PREPEND                  43
#define NATIVE_INSERTBEFORE             44
#define NATIVE_REMOVECHILD              45
#define NATIVE_REMOVE                   46
#define NATIVE_INSERTADJACENTHTML       47
#define NATIVE_CALL                     48
#define NATIVE_APPLY                    49
#define NATIVE_BIND                     50
#define NATIVE_HASOWNPROPERTY           51
#define NATIVE_ARRAY                    52
#define NATIVE_ISARRAY                  53
#define NATIVE_ARRAYFROM                54
#define NATIVE_OBJECT                   55
#define NATIVE_KEYS                     56
#define NATIVE_VALUES                   57
#define NATIVE_ASSIGN                   58
#define NATIVE_CREATE                   59
#define NATIVE_IDENTITY                 60
#define NATIVE_DEFINEPROPERTY           61
#define NATIVE_STRINGIFY                62
#define NATIVE_PARSE                    63
#define NATIVE_SQRT                     65
#define NATIVE_POW                      66
#define NATIVE_CLEARTIMER               67
#define NATIVE_PARSEINT                 68
#define NATIVE_NUMBER                   69
#define NATIVE_STRING                   70
#define NATIVE_BOOLEAN                  71
#define NATIVE_ERROR                    72
#define NATIVE_DATE                     73
#define NATIVE_ZERO                     74
#define NATIVE_TOFIXED                  75
#define NATIVE_TOSTRING                 76
#define NATIVE_REPEAT                   77
#define NATIVE_PADSTART                 78
#define NATIVE_PADEND                   79
#define NATIVE_CHARCODEAT               80
#define NATIVE_FROMCHARCODE             81
#define NATIVE_LASTINDEXOF              82
#define NATIVE_MAP                      83
#define NATIVE_FILTER                   84
#define NATIVE_FIND                     85
#define NATIVE_FINDINDEX                86
#define NATIVE_SOME                     87
#define NATIVE_EVERY                    88
#define NATIVE_REDUCE                   89
#define NATIVE_SORT                     90
#define NATIVE_REVERSE                  91
#define NATIVE_CONCAT                   92
#define NATIVE_POP                      93
#define NATIVE_SHIFT                    94
#define NATIVE_UNSHIFT                  95
#define NATIVE_SPLICE                   96
#define NATIVE_ISNAN                    97
#define NATIVE_SUBSTR                   98
#define NATIVE_ARRAYITEM                100
#define NATIVE_TEST                     101
#define NATIVE_EXEC                     102
#define NATIVE_MATCH                    103
#define NATIVE_SEARCH                   104
#define NATIVE_REPLACEALL               105
#define NATIVE_REGEXP                   106
#define NATIVE_FUNCTION                 107
#define NATIVE_DOMPARSER                108
#define NATIVE_PARSEFROMSTRING          109
#define NATIVE_REPLACECHILDREN          110
#define NATIVE_CLICK                    111
#define NATIVE_FULLYEAR                 112
#define NATIVE_REMOVEEVENTLISTENER      113
#define NATIVE_NEWMAP                   114
#define NATIVE_NEWSET                   115
#define NATIVE_COLLECTIONSET            116
#define NATIVE_COLLECTIONGET            117
#define NATIVE_COLLECTIONHAS            118
#define NATIVE_COLLECTIONDELETE         119
#define NATIVE_COLLECTIONCLEAR          120
#define NATIVE_COLLECTIONFOREACH        121
#define NATIVE_COLLECTIONKEYS           122
#define NATIVE_COLLECTIONVALUES         123
#define NATIVE_COLLECTIONENTRIES        124
#define NATIVE_ENTRIES                  125
#define NATIVE_SYMBOL                   126
#define NATIVE_ELEMENT                  127
#define NATIVE_OBJECTTOSTRING           128
#define NATIVE_EVENT                    129
#define NATIVE_DISPATCHEVENT            130
#define NATIVE_COMPUTEDSTYLE            131
#define NATIVE_PROPERTYVALUE            132
#define NATIVE_FILL                     99
#define SELECTORSIZE                    128
#define STYLESIZE                       512
#define TEXTSIZE                        1024
#define VOIDS                           13
#define STRINGMETHODS                   25
#define ARRAYMETHODS                    25
#define SORTSIZE                        256
#define DATEFIELDS                      10
#define LOCATIONFIELDS                  8
#define COLLECTIONMETHODS               11
#define SYMBOLS                         4
#define REGEXNODES                      128
#define CAPTURES                        10
#define REPEATMAX                       0x7FFF
#define REGEXDEPTH                      2048
#define RE_CHAR                         1
#define RE_ANY                          2
#define RE_CLASS                        3
#define RE_START                        4
#define RE_END                          5
#define RE_BOUNDARY                     6
#define RE_NOTBOUNDARY                  7
#define RE_GROUP                        8
#define RE_ALTERNATIVE                  9
#define RE_DIGIT                        10
#define RE_WORD                         11
#define RE_SPACE                        12
#define JSONDEPTH                       16

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
    unsigned int more;
    unsigned int noin;
    unsigned int arguments;

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

struct declaration
{

    unsigned int start;
    unsigned int length;

};

struct method
{

    char *name;
    unsigned int native;

};

struct renode
{

    unsigned int type;
    unsigned int value;
    unsigned int negated;
    unsigned int min;
    unsigned int max;
    unsigned int lazy;
    unsigned int child;
    unsigned int alternative;
    unsigned int next;

};

struct capture
{

    int start;
    int end;

};

struct regex
{

    char *pattern;
    char *text;
    int length;
    unsigned int offset;
    unsigned int ignorecase;
    unsigned int failed;
    unsigned int groups;
    unsigned int steps;
    unsigned int depth;
    struct renode nodes[REGEXNODES];
    unsigned int nodecount;
    struct capture captures[CAPTURES];

};

struct frame
{

    unsigned int node;
    unsigned int count;
    int start;
    struct frame *parent;

};

struct output
{

    struct js_engine *js;
    char *data;
    unsigned int count;
    unsigned int size;
    unsigned int full;

};

struct reader
{

    char *text;
    unsigned int offset;
    unsigned int failed;

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
static struct method stringmethods[STRINGMETHODS] = {
    {"indexOf", NATIVE_INDEXOF}, {"lastIndexOf", NATIVE_LASTINDEXOF}, {"toUpperCase", NATIVE_TOUPPERCASE}, {"toLowerCase", NATIVE_TOLOWERCASE},
    {"startsWith", NATIVE_STARTSWITH}, {"endsWith", NATIVE_ENDSWITH}, {"includes", NATIVE_INCLUDES}, {"trim", NATIVE_TRIM},
    {"split", NATIVE_SPLIT}, {"slice", NATIVE_SLICE}, {"substring", NATIVE_SLICE}, {"substr", NATIVE_SUBSTR},
    {"replace", NATIVE_REPLACE}, {"charAt", NATIVE_CHARAT}, {"charCodeAt", NATIVE_CHARCODEAT}, {"repeat", NATIVE_REPEAT},
    {"padStart", NATIVE_PADSTART}, {"padEnd", NATIVE_PADEND}, {"concat", NATIVE_CONCAT}, {"toString", NATIVE_TOSTRING},
    {"match", NATIVE_MATCH}, {"search", NATIVE_SEARCH}, {"replaceAll", NATIVE_REPLACEALL}, {"trimStart", NATIVE_TRIM}, {"trimEnd", NATIVE_TRIM}
};
static struct method arraymethods[ARRAYMETHODS] = {
    {"indexOf", NATIVE_INDEXOF}, {"lastIndexOf", NATIVE_LASTINDEXOF}, {"push", NATIVE_PUSH}, {"join", NATIVE_JOIN},
    {"forEach", NATIVE_FOREACH}, {"includes", NATIVE_INCLUDES}, {"slice", NATIVE_SLICE}, {"map", NATIVE_MAP},
    {"filter", NATIVE_FILTER}, {"find", NATIVE_FIND}, {"findIndex", NATIVE_FINDINDEX}, {"some", NATIVE_SOME},
    {"every", NATIVE_EVERY}, {"reduce", NATIVE_REDUCE}, {"sort", NATIVE_SORT}, {"reverse", NATIVE_REVERSE},
    {"concat", NATIVE_CONCAT}, {"pop", NATIVE_POP}, {"shift", NATIVE_SHIFT}, {"unshift", NATIVE_UNSHIFT},
    {"splice", NATIVE_SPLICE}, {"fill", NATIVE_FILL}, {"toString", NATIVE_TOSTRING}, {"item", NATIVE_ARRAYITEM}, {"keys", NATIVE_KEYS}
};
static char *locationfields[LOCATIONFIELDS] = {"href", "hash", "search", "pathname", "host", "hostname", "origin", "protocol"};
static struct method collectionmethods[COLLECTIONMETHODS] = {
    {"set", NATIVE_COLLECTIONSET}, {"add", NATIVE_COLLECTIONSET}, {"get", NATIVE_COLLECTIONGET}, {"has", NATIVE_COLLECTIONHAS},
    {"delete", NATIVE_COLLECTIONDELETE}, {"clear", NATIVE_COLLECTIONCLEAR}, {"forEach", NATIVE_COLLECTIONFOREACH}, {"keys", NATIVE_COLLECTIONKEYS},
    {"values", NATIVE_COLLECTIONVALUES}, {"entries", NATIVE_COLLECTIONENTRIES}, {"toString", NATIVE_TOSTRING}
};
static char *symbols[SYMBOLS] = {"iterator", "toStringTag", "asyncIterator", "hasInstance"};
static char *datefields[DATEFIELDS] = {"getTime", "getFullYear", "getMonth", "getDate", "getDay", "getHours", "getMinutes", "getSeconds", "getMilliseconds", "getTimezoneOffset"};
static char *voids[VOIDS] = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "source", "track", "wbr"};
static struct binary binaries[BINARIES] = {
    {"??", OP_NULLISH, 1, 1}, {"||", OP_OR, 2, 1}, {"&&", OP_AND, 3, 1}, {"|", OP_BITOR, 4, 0}, {"^", OP_BITXOR, 5, 0}, {"&", OP_BITAND, 6, 0},
    {"===", OP_STRICTEQUAL, 7, 0}, {"!==", OP_STRICTNOTEQUAL, 7, 0}, {"==", OP_EQUAL, 7, 0}, {"!=", OP_NOTEQUAL, 7, 0},
    {"<=", OP_LESSEQUAL, 8, 0}, {">=", OP_GREATEREQUAL, 8, 0}, {"<", OP_LESS, 8, 0}, {">", OP_GREATER, 8, 0},
    {"<<", OP_SHIFTLEFT, 9, 0}, {">>", OP_SHIFTRIGHT, 9, 0}, {"+", OP_ADD, 10, 0}, {"-", OP_SUBTRACT, 10, 0},
    {"*", OP_MULTIPLY, 11, 0}, {"/", OP_DIVIDE, 11, 0}, {"%", OP_MODULO, 11, 0}, {"instanceof", OP_INSTANCEOF, 8, 0}, {"in", OP_IN, 8, 0}
};

static struct js_value execlist(struct js_engine *js, unsigned int first, unsigned int scope, struct completion *completion);
static void execute(struct js_engine *js, unsigned int index, unsigned int scope, struct completion *completion);
static struct js_value evaluate(struct js_engine *js, unsigned int node, unsigned int scope);
static unsigned int parseexpression(struct parser *parser);
static unsigned int parseassignment(struct parser *parser);
static unsigned int parsestatement(struct parser *parser);
static unsigned int parseblock(struct parser *parser);
static struct js_value callfunction(struct js_engine *js, struct call *call);
static void bindpattern(struct js_engine *js, unsigned int scope, unsigned int declarator, struct js_value *source);
static unsigned int bubble(struct js_engine *js, unsigned int target, unsigned int name);
static unsigned int parsepostfix(struct parser *parser);
static void parsepattern(struct parser *parser, unsigned int declarator);
static unsigned int parseitem(struct parser *parser);
static unsigned int parseclass(struct parser *parser);

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

    return value->type == JS_OBJECT && (js->objects[value->number].kind == KIND_FUNCTION || js->objects[value->number].kind == KIND_NATIVE || js->objects[value->number].kind == KIND_BOUND);

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

static char readescaped(struct parser *parser)
{

    char c = parser->source[parser->offset++];

    if (c != '\\' || parser->offset >= parser->count)
        return c;

    c = parser->source[parser->offset++];

    if (c == 'n')
        return '\n';

    if (c == 't')
        return '\t';

    if (c == 'r')
        return '\r';

    return c;

}

static void scantemplate(struct parser *parser)
{

    struct js_engine *js = parser->js;
    char *source = parser->source;
    unsigned int start = js->stringcount;

    parser->more = 0;

    while (parser->offset < parser->count && source[parser->offset] != '`')
    {

        char c;

        if (source[parser->offset] == '$' && parser->offset + 1 < parser->count && source[parser->offset + 1] == '{')
        {

            parser->more = 1;
            parser->offset++;

            break;

        }

        c = readescaped(parser);

        if (js->stringcount + 2 >= JS_STRINGSIZE)
        {

            parser->failed = 1;

            return;

        }

        js->strings[js->stringcount++] = c;

    }

    parser->offset++;
    js->strings[js->stringcount++] = '\0';
    parser->token.type = TOKEN_TEMPLATE;
    parser->token.value = start;
    parser->token.length = parser->offset - parser->token.start;

}

static unsigned int startsexpression(struct parser *parser)
{

    struct token *token = &parser->token;
    char *text = parser->source + token->start;

    if (token->type == TOKEN_END)
        return 1;

    if (token->type == TOKEN_PUNCTUATOR)
        return !(token->length == 1 && (text[0] == ')' || text[0] == ']' || text[0] == '}'));

    if (token->type == TOKEN_NAME)
    {

        char *word = getstring(parser->js, token->value);

        return cstring_match(word, "return") || cstring_match(word, "typeof") || cstring_match(word, "case") || cstring_match(word, "in") || cstring_match(word, "of") || cstring_match(word, "throw") || cstring_match(word, "else") || cstring_match(word, "do") || cstring_match(word, "void") || cstring_match(word, "delete") || cstring_match(word, "new");

    }

    return 0;

}

static void scanregex(struct parser *parser)
{

    struct js_engine *js = parser->js;
    char *source = parser->source;
    unsigned int start = js->stringcount;
    unsigned int inclass = 0;

    parser->offset++;

    while (parser->offset < parser->count && source[parser->offset] != '\n' && (inclass || source[parser->offset] != '/'))
    {

        char c = source[parser->offset++];

        if (js->stringcount + 3 >= JS_STRINGSIZE)
        {

            parser->failed = 1;

            return;

        }

        js->strings[js->stringcount++] = c;

        if (c == '\\' && parser->offset < parser->count)
            js->strings[js->stringcount++] = source[parser->offset++];
        else if (c == '[')
            inclass = 1;
        else if (c == ']')
            inclass = 0;

    }

    if (parser->offset >= parser->count || source[parser->offset] != '/')
    {

        parser->failed = 1;

        return;

    }

    parser->offset++;
    js->strings[js->stringcount++] = '\0';

    /* the flags follow the pattern as a second string */
    while (parser->offset < parser->count && cstring_isalpha(source[parser->offset]) && js->stringcount + 2 < JS_STRINGSIZE)
        js->strings[js->stringcount++] = source[parser->offset++];

    js->strings[js->stringcount++] = '\0';
    parser->token.type = TOKEN_REGEX;
    parser->token.value = start;
    parser->token.length = parser->offset - parser->token.start;

}

static void nexttoken(struct parser *parser)
{

    char *source = parser->source;
    struct token *token = &parser->token;
    unsigned int regexallowed = startsexpression(parser);
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

    if (source[parser->offset] == '`')
    {

        parser->offset++;

        scantemplate(parser);

        return;

    }

    if (source[parser->offset] == '/' && regexallowed)
    {

        scanregex(parser);

        return;

    }

    if (source[parser->offset] == '"' || source[parser->offset] == '\'')
    {

        char quote = source[parser->offset++];
        unsigned int start = parser->js->stringcount;

        while (parser->offset < parser->count && source[parser->offset] != quote)
        {

            char c = readescaped(parser);

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

        unsigned int node = newnode(parser, (ispunctuator(parser, "[") || ispunctuator(parser, "{")) ? NODE_DECLARATOR : NODE_NAME);

        if (!node)
            break;

        if (accept(parser, "..."))
            parser->js->nodes[node].op = 1;

        if (parser->js->nodes[node].type == NODE_DECLARATOR)
            parsepattern(parser, node);
        else
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

static unsigned int parsebody(struct parser *parser, unsigned int node)
{

    unsigned int outer = parser->arguments;
    unsigned int noin = parser->noin;

    parser->arguments = 0;
    parser->noin = 0;

    if (node)
    {

        parser->js->nodes[node].a = parseparameters(parser);
        parser->js->nodes[node].b = parseblock(parser);

        if (parser->arguments)
            parser->js->nodes[node].c |= FUNCTION_ARGUMENTS;

    }

    parser->arguments = outer;
    parser->noin = noin;

    return node;

}

static unsigned int parsefunction(struct parser *parser)
{

    unsigned int node = newnode(parser, NODE_FUNCTION);

    if (!node)
        return 0;

    if (parser->token.type == TOKEN_NAME)
        parser->js->nodes[node].value = parsename(parser);

    return parsebody(parser, node);

}

static unsigned int parsearrowbody(struct parser *parser, unsigned int node)
{

    parser->js->nodes[node].c |= FUNCTION_ARROW;

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

static unsigned int joinnodes(struct parser *parser, unsigned int left, unsigned int right)
{

    unsigned int node = newnode(parser, NODE_BINARY);

    if (node)
    {

        parser->js->nodes[node].op = OP_ADD;
        parser->js->nodes[node].a = left;
        parser->js->nodes[node].b = right;

    }

    return node;

}

static unsigned int parsetemplate(struct parser *parser)
{

    unsigned int node = newnode(parser, NODE_STRING);

    if (!node)
        return 0;

    parser->js->nodes[node].value = parser->token.value;

    while (parser->more && !parser->failed)
    {

        unsigned int part;

        nexttoken(parser);

        node = joinnodes(parser, node, parseexpression(parser));

        if (!ispunctuator(parser, "}"))
        {

            parser->failed = 1;

            return 0;

        }

        parser->offset = parser->token.start + 1;
        parser->token.start = parser->offset;

        scantemplate(parser);

        part = newnode(parser, NODE_STRING);

        if (!part)
            return 0;

        parser->js->nodes[part].value = parser->token.value;
        node = joinnodes(parser, node, part);

    }

    nexttoken(parser);

    return node;

}

static unsigned int parseaccessor(struct parser *parser)
{

    unsigned int accessor = (isword(parser, "get")) ? ACCESSOR_GET : (isword(parser, "set")) ? ACCESSOR_SET : 0;
    unsigned int offset = parser->offset;
    struct token token;

    if (!accessor)
        return 0;

    buffer_copy(&token, &parser->token, sizeof (struct token));
    nexttoken(parser);

    /* get and set are only accessors when a name follows; otherwise they are names themselves */
    if (parser->token.type == TOKEN_NAME || parser->token.type == TOKEN_STRING)
        return accessor;

    parser->offset = offset;

    buffer_copy(&parser->token, &token, sizeof (struct token));

    return 0;

}

static unsigned int parseclass(struct parser *parser)
{

    struct js_engine *js = parser->js;
    unsigned int node = newnode(parser, NODE_CLASS);
    unsigned int last = 0;

    nexttoken(parser);

    if (!node)
        return 0;

    if (parser->token.type == TOKEN_NAME && !isword(parser, "extends"))
        js->nodes[node].value = parsename(parser);

    if (isword(parser, "extends"))
    {

        nexttoken(parser);

        js->nodes[node].c = parsepostfix(parser);

    }

    expect(parser, "{");

    while (!parser->failed && !ispunctuator(parser, "}") && parser->token.type != TOKEN_END)
    {

        unsigned int member;
        unsigned int isstatic = 0;
        unsigned int accessor;

        if (accept(parser, ";"))
            continue;

        while (isword(parser, "static") || isword(parser, "async"))
        {

            isstatic = isstatic || isword(parser, "static");

            nexttoken(parser);

        }

        accessor = parseaccessor(parser);
        member = newnode(parser, NODE_PROPERTY);

        if (!member || (parser->token.type != TOKEN_NAME && parser->token.type != TOKEN_STRING))
        {

            parser->failed = 1;

            break;

        }

        js->nodes[member].value = (parser->token.type == TOKEN_NAME) ? parser->token.value : (int)internstring(js, getstring(js, parser->token.value));
        js->nodes[member].c = isstatic;
        js->nodes[member].op = accessor;

        nexttoken(parser);

        if (!ispunctuator(parser, "("))
        {

            /* fields are skipped: only methods and the constructor are kept */
            if (accept(parser, "="))
                parseassignment(parser);

            accept(parser, ";");

            continue;

        }

        js->nodes[member].a = parsebody(parser, newnode(parser, NODE_FUNCTION));

        if (!isstatic && cstring_match(getstring(js, js->nodes[member].value), "constructor"))
        {

            js->nodes[node].a = js->nodes[member].a;

            continue;

        }

        last = appendnode(parser, last, member);

        if (!js->nodes[node].b)
            js->nodes[node].b = last;

    }

    expect(parser, "}");

    if (!js->nodes[node].a)
    {

        unsigned int constructor = newnode(parser, NODE_FUNCTION);

        if (!constructor)
            return 0;

        js->nodes[constructor].b = newnode(parser, NODE_BLOCK);
        js->nodes[constructor].c = (js->nodes[node].c) ? FUNCTION_DERIVED : 0;
        js->nodes[node].a = constructor;

    }

    return node;

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

    if (parser->token.type == TOKEN_TEMPLATE)
        return parsetemplate(parser);

    if (parser->token.type == TOKEN_REGEX)
    {

        node = newnode(parser, NODE_REGEX);

        if (node)
            js->nodes[node].value = parser->token.value;

        nexttoken(parser);

        return node;

    }

    if (isword(parser, "class"))
        return parseclass(parser);

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

        if (cstring_match(getstring(js, name), "arguments"))
            parser->arguments = 1;

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

            last = appendnode(parser, last, parseitem(parser));

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

            if (accept(parser, "..."))
            {

                js->nodes[property].c = 1;
                js->nodes[property].a = parseassignment(parser);
                last = appendnode(parser, last, property);

                if (!js->nodes[node].a)
                    js->nodes[node].a = last;

                if (!accept(parser, ","))
                    break;

                continue;

            }

            if (isword(parser, "async"))
            {

                unsigned int offset = parser->offset;
                struct token token;

                buffer_copy(&token, &parser->token, sizeof (struct token));
                nexttoken(parser);

                /* async before a method name is dropped: the method runs like any other */
                if (parser->token.type != TOKEN_NAME)
                {

                    parser->offset = offset;

                    buffer_copy(&parser->token, &token, sizeof (struct token));

                }

            }

            js->nodes[property].op = parseaccessor(parser);

            if (accept(parser, "["))
            {

                js->nodes[property].c = 2;
                js->nodes[property].b = parseassignment(parser);

                if (!ispunctuator(parser, "]"))
                    parser->failed = 1;

            }

            else if (parser->token.type == TOKEN_STRING)
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

                js->nodes[property].a = parsebody(parser, newnode(parser, NODE_FUNCTION));

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

static unsigned int parseitem(struct parser *parser)
{

    unsigned int node;

    if (!accept(parser, "..."))
        return parseassignment(parser);

    node = newnode(parser, NODE_SPREAD);

    if (node)
        parser->js->nodes[node].a = parseassignment(parser);

    return node;

}

static unsigned int parsearguments(struct parser *parser)
{

    unsigned int first = 0;
    unsigned int last = 0;

    while (!parser->failed && !ispunctuator(parser, ")"))
    {

        last = appendnode(parser, last, parseitem(parser));
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
    {

        unsigned int construct = newnode(parser, NODE_CALL);

        nexttoken(parser);

        node = parseprimary(parser);

        while (!parser->failed && (ispunctuator(parser, ".") || ispunctuator(parser, "[")))
        {

            unsigned int next = newnode(parser, (ispunctuator(parser, ".")) ? NODE_MEMBER : NODE_INDEX);

            if (!next)
                return 0;

            js->nodes[next].a = node;

            if (accept(parser, "."))
            {

                js->nodes[next].value = parsename(parser);

            }

            else
            {

                nexttoken(parser);

                js->nodes[next].b = parseexpression(parser);

                expect(parser, "]");

            }

            node = next;

        }

        if (!construct)
            return 0;

        js->nodes[construct].op = 1;
        js->nodes[construct].a = node;

        if (accept(parser, "("))
            js->nodes[construct].b = parsearguments(parser);

        node = construct;

    }

    else
    {

        node = parseprimary(parser);

    }

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

    if (parser->token.type != TOKEN_PUNCTUATOR && parser->token.type != TOKEN_NAME)
        return 0;

    for (i = 0; i < BINARIES; i++)
    {

        if ((parser->token.type == TOKEN_NAME) ? isword(parser, binaries[i].text) && !(parser->noin && binaries[i].op == OP_IN) : ispunctuator(parser, binaries[i].text))
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
    {

        unsigned int sequence = newnode(parser, NODE_SEQUENCE);

        if (!sequence)
            break;

        parser->js->nodes[sequence].a = node;
        parser->js->nodes[sequence].b = parseassignment(parser);
        node = sequence;

    }

    return node;

}

static void endstatement(struct parser *parser)
{

    accept(parser, ";");

}

static void parsepattern(struct parser *parser, unsigned int declarator)
{

    unsigned int pattern = (ispunctuator(parser, "[")) ? PATTERN_ARRAY : PATTERN_OBJECT;
    char *close = (pattern == PATTERN_ARRAY) ? "]" : "}";
    unsigned int lastname = 0;

    nexttoken(parser);

    parser->js->nodes[declarator].c = pattern;

    while (!parser->failed && !ispunctuator(parser, close))
    {

        unsigned int rest = accept(parser, "...");
        unsigned int key = 0;
        unsigned int name;

        /* in an object pattern the key comes first and may be followed by : and a target */
        if (pattern == PATTERN_OBJECT && !rest)
        {

            if (parser->token.type == TOKEN_NAME)
                key = parser->token.value;
            else if (parser->token.type == TOKEN_STRING)
                key = internstring(parser->js, getstring(parser->js, parser->token.value));
            else
                parser->failed = 1;

            nexttoken(parser);

            if (!accept(parser, ":"))
            {

                name = newnode(parser, NODE_NAME);

                if (!name)
                    break;

                parser->js->nodes[name].value = key;
                key = 0;

            }

            else
            {

                name = newnode(parser, (ispunctuator(parser, "[") || ispunctuator(parser, "{")) ? NODE_DECLARATOR : NODE_NAME);

            }

        }

        else
        {

            name = newnode(parser, (ispunctuator(parser, "[") || ispunctuator(parser, "{")) ? NODE_DECLARATOR : NODE_NAME);

        }

        if (!name)
            break;

        if (parser->js->nodes[name].type == NODE_DECLARATOR)
            parsepattern(parser, name);
        else if (!parser->js->nodes[name].value && !(pattern == PATTERN_ARRAY && ispunctuator(parser, ",")))
            parser->js->nodes[name].value = parsename(parser);

        /* a renamed name keeps its key in b; a nested pattern keeps it in value, as b holds its names */
        if (parser->js->nodes[name].type == NODE_DECLARATOR)
            parser->js->nodes[name].value = key;
        else
            parser->js->nodes[name].b = key;

        parser->js->nodes[name].op = rest;

        if (accept(parser, "="))
            parser->js->nodes[name].a = parseassignment(parser);

        lastname = appendnode(parser, lastname, name);

        if (!parser->js->nodes[declarator].b)
            parser->js->nodes[declarator].b = lastname;

        if (!accept(parser, ","))
            break;

    }

    expect(parser, close);

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
            parsepattern(parser, declarator);
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

    if (isword(parser, "class"))
    {

        unsigned int declarator = newnode(parser, NODE_DECLARATOR);

        node = newnode(parser, NODE_VAR);

        if (!node || !declarator)
            return 0;

        js->nodes[declarator].a = parseclass(parser);
        js->nodes[declarator].value = js->nodes[js->nodes[declarator].a].value;
        js->nodes[node].a = declarator;

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

        parser->noin = 1;

        if (isword(parser, "var") || isword(parser, "let") || isword(parser, "const"))
            js->nodes[node].a = parsevariables(parser);
        else if (!ispunctuator(parser, ";"))
            js->nodes[node].a = parseexpression(parser);

        parser->noin = 0;

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

    if (!node || node >= HTML_NODES || js->document->nodes[node].type == HTML_NODE_DOCUMENT || html_isgenerated(js->document, node))
        return 0;

    if (!js->elements[node])
        js->elements[node] = newobject(js, KIND_ELEMENT, 0, node);

    return js->elements[node];

}

static struct js_value nodevalue(struct js_engine *js, unsigned int node)
{

    unsigned int object = getelement(js, node);

    return (object) ? value(JS_OBJECT, object) : value(JS_NULL, 0);

}

static struct js_value elementvalue(struct js_engine *js, unsigned int node)
{

    if (node && js->document->nodes[node].type != HTML_NODE_ELEMENT)
        return value(JS_NULL, 0);

    return nodevalue(js, node);

}

static unsigned int nodeof(struct js_engine *js, struct js_value *target)
{

    if (target->type != JS_OBJECT || js->objects[target->number].kind != KIND_ELEMENT)
        return 0;

    return js->objects[target->number].data;

}

static unsigned int argumentnode(struct js_engine *js, struct js_value *argument)
{

    if (argument->type == JS_OBJECT)
        return nodeof(js, argument);

    if (argument->type == JS_UNDEFINED || argument->type == JS_NULL)
        return 0;

    return html_createtext(js->document, getstring(js, tostring(js, argument)));

}

static void moveall(struct js_engine *js, unsigned int from, unsigned int parent, unsigned int before)
{

    while (js->document->nodes[from].first)
    {

        if (!html_insert(js->document, parent, js->document->nodes[from].first, before))
            break;

    }

}

static void put(struct output *output, char *text, unsigned int length)
{

    if (output->full || output->count + length >= output->size)
    {

        output->full = 1;

        return;

    }

    buffer_copy(output->data + output->count, text, length);

    output->count += length;

}

static void putescaped(struct output *output, char *text, unsigned int length, unsigned int attribute)
{

    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i < length; i++)
    {

        char *entity = (text[i] == '&') ? "&amp;" : (text[i] == '<' && !attribute) ? "&lt;" : (text[i] == '>' && !attribute) ? "&gt;" : (text[i] == '"' && attribute) ? "&quot;" : 0;

        if (!entity)
            continue;

        put(output, text + start, i - start);
        put(output, entity, cstring_length(entity));

        start = i + 1;

    }

    put(output, text + start, length - start);

}

static unsigned int isvoid(char *name)
{

    unsigned int i;

    for (i = 0; i < VOIDS; i++)
    {

        if (cstring_match(voids[i], name))
            return 1;

    }

    return 0;

}

static void serialize(struct output *output, unsigned int root, unsigned int depth)
{

    struct html_document *document = output->js->document;
    unsigned int raw = document->nodes[root].tag == HTML_TAG_SCRIPT || document->nodes[root].tag == HTML_TAG_STYLE;
    unsigned int node;

    if (depth > HTML_DEPTH)
        return;

    for (node = document->nodes[root].first; node; node = document->nodes[node].next)
    {

        struct html_node *child = &document->nodes[node];
        char *name;
        unsigned int i;

        if (html_isgenerated(document, node))
            continue;

        if (child->type == HTML_NODE_TEXT)
        {

            if (raw)
                put(output, html_getstring(document, child->text), child->length);
            else
                putescaped(output, html_getstring(document, child->text), child->length, 0);

            continue;

        }

        name = html_getstring(document, child->name);

        put(output, "<", 1);
        put(output, name, cstring_length(name));

        for (i = 0; i < child->attributes; i++)
        {

            struct html_attribute *attribute = &document->attributes[child->attribute + i];
            char *text = html_getstring(document, attribute->value);

            put(output, " ", 1);
            put(output, html_getstring(document, attribute->name), cstring_length(html_getstring(document, attribute->name)));
            put(output, "=\"", 2);
            putescaped(output, text, cstring_length(text), 1);
            put(output, "\"", 1);

        }

        put(output, ">", 1);

        if (isvoid(name))
            continue;

        serialize(output, node, depth + 1);
        put(output, "</", 2);
        put(output, name, cstring_length(name));
        put(output, ">", 1);

    }

}

static struct js_value innerhtml(struct js_engine *js, unsigned int node)
{

    struct output output;
    unsigned int start = js->stringcount;

    /* nothing else adds strings while the markup is written, so it goes straight after the last one */
    output.js = js;
    output.data = js->strings + start;
    output.count = 0;
    output.size = JS_STRINGSIZE - start;
    output.full = 0;

    serialize(&output, node, 0);

    if (output.full)
    {

        throwerror(js, "Out of string memory");

        return value(JS_STRING, internstring(js, ""));

    }

    js->strings[start + output.count] = '\0';
    js->stringcount += output.count + 1;

    return value(JS_STRING, start);

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

static unsigned int isselectorend(char c)
{

    return c == '#' || c == '.' || c == '[' || c == ':';

}

static unsigned int equalspan(char *text, char *span, unsigned int length)
{

    return cstring_length(text) == length && buffer_match(text, span, length);

}

static unsigned int hasword(char *list, char *word, unsigned int length)
{

    unsigned int i = 0;

    while (list && list[i])
    {

        unsigned int start;

        while (list[i] && isspace(list[i]))
            i++;

        start = i;

        while (list[i] && !isspace(list[i]))
            i++;

        if (i - start == length && length && buffer_match(list + start, word, length))
            return 1;

    }

    return 0;

}

static unsigned int matchattribute(struct js_engine *js, unsigned int node, char *text, unsigned int length)
{

    unsigned int equal = buffer_findbyte(text, length, '=');
    char name[SELECTORSIZE];
    char *attribute;

    if (equal >= SELECTORSIZE)
        return 0;

    buffer_copy(name, text, equal);

    name[equal] = '\0';
    attribute = html_getattribute(js->document, node, name);

    if (!attribute || equal >= length)
        return attribute != 0;

    text += equal + 1;
    length -= equal + 1;

    if (length >= 2 && (text[0] == '"' || text[0] == '\''))
    {

        text++;
        length -= 2;

    }

    return equalspan(attribute, text, length);

}

static unsigned int matchcompound(struct js_engine *js, unsigned int node, char *text, unsigned int length)
{

    struct html_document *document = js->document;
    unsigned int i = 0;

    if (!node || document->nodes[node].type != HTML_NODE_ELEMENT)
        return 0;

    while (i < length && !isselectorend(text[i]))
        i++;

    if (i && !(i == 1 && text[0] == '*') && !equalspan(html_getstring(document, document->nodes[node].name), text, i))
        return 0;

    while (i < length)
    {

        char kind = text[i++];
        unsigned int start = i;

        if (kind == '[')
        {

            while (i < length && text[i] != ']')
                i++;

            if (!matchattribute(js, node, text + start, i - start))
                return 0;

            i++;

            continue;

        }

        while (i < length && !isselectorend(text[i]))
            i++;

        if (kind == '#' && !equalspan(html_getattribute(document, node, "id") ? html_getattribute(document, node, "id") : "", text + start, i - start))
            return 0;

        if (kind == '.' && !hasword(html_getattribute(document, node, "class"), text + start, i - start))
            return 0;

        if (kind == ':')
            return 0;

    }

    return 1;

}

static unsigned int matchselector(struct js_engine *js, unsigned int node, char *text, unsigned int length)
{

    struct html_document *document = js->document;
    unsigned int end = length;
    unsigned int start;
    unsigned int child = 0;

    while (end && isspace(text[end - 1]))
        end--;

    start = end;

    while (start && !isspace(text[start - 1]) && text[start - 1] != '>')
        start--;

    if (start == end || !matchcompound(js, node, text + start, end - start))
        return 0;

    while (start && isspace(text[start - 1]))
        start--;

    if (start && text[start - 1] == '>')
    {

        child = 1;
        start--;

        while (start && isspace(text[start - 1]))
            start--;

    }

    if (!start)
        return 1;

    for (node = document->nodes[node].parent; node; node = document->nodes[node].parent)
    {

        if (matchselector(js, node, text, start))
            return 1;

        if (child)
            return 0;

    }

    return 0;

}

static unsigned int matchlist(struct js_engine *js, unsigned int node, char *text)
{

    unsigned int length = cstring_length(text);
    unsigned int start = 0;

    while (start < length)
    {

        unsigned int end = start + buffer_findbyte(text + start, length - start, ',');

        while (start < end && isspace(text[start]))
            start++;

        if (end > start && matchselector(js, node, text + start, end - start))
            return 1;

        start = end + 1;

    }

    return 0;

}

static struct js_value newarray(struct js_engine *js)
{

    unsigned int object = newobject(js, KIND_ARRAY, 0, 0);

    if (!object)
        throwerror(js, "Out of objects");

    return (object) ? value(JS_OBJECT, object) : undefined();

}

static void pusharray(struct js_engine *js, struct js_value *array, struct js_value *item)
{

    if (array->type == JS_OBJECT)
        putproperty(js, array->number, numberstring(js, js->objects[array->number].data++), item);

}

static struct js_value query(struct js_engine *js, unsigned int root, char *selector, unsigned int all)
{

    struct js_value result = (all) ? newarray(js) : value(JS_NULL, 0);
    unsigned int node;

    for (node = html_next(js->document, root, root); node; node = html_next(js->document, node, root))
    {

        if (!html_isgenerated(js->document, node) && matchlist(js, node, selector))
        {

            struct js_value element = elementvalue(js, node);

            if (!all)
                return element;

            pusharray(js, &result, &element);

        }

    }

    return result;

}

static unsigned int collecttext(struct js_engine *js, unsigned int root, char *out, unsigned int size)
{

    struct html_document *document = js->document;
    unsigned int count = 0;
    unsigned int node;

    for (node = html_next(document, root, root); node; node = html_next(document, node, root))
    {

        struct html_node *text = &document->nodes[node];

        if (text->type == HTML_NODE_TEXT && !html_isgenerated(document, node) && count + text->length < size)
        {

            buffer_copy(out + count, html_getstring(document, text->text), text->length);

            count += text->length;

        }

    }

    out[count] = '\0';

    return count;

}

static unsigned int stylename(char *name, char *out)
{

    unsigned int count = 0;
    unsigned int i;

    if (cstring_match(name, "cssFloat"))
        name = "float";

    for (i = 0; name[i] && count + 2 < SELECTORSIZE; i++)
    {

        if (name[i] >= 'A' && name[i] <= 'Z')
        {

            out[count++] = '-';
            out[count++] = name[i] + 32;

        }

        else
        {

            out[count++] = name[i];

        }

    }

    out[count] = '\0';

    return count;

}

static struct declaration findstyle(char *style, char *name, unsigned int length)
{

    struct declaration found;
    unsigned int i = 0;

    found.start = 0;
    found.length = 0;

    while (style && style[i])
    {

        unsigned int start;
        unsigned int end;

        while (style[i] && (isspace(style[i]) || style[i] == ';'))
            i++;

        start = i;

        while (style[i] && style[i] != ';')
            i++;

        end = start + buffer_findbyte(style + start, i - start, ':');

        while (end > start && isspace(style[end - 1]))
            end--;

        if (end - start == length && buffer_match(style + start, name, length))
        {

            found.start = start;
            found.length = i - start;

            break;

        }

    }

    return found;

}

static void setstyle(struct js_engine *js, unsigned int node, char *property, char *newvalue)
{

    char *style = html_getattribute(js->document, node, "style");
    char name[SELECTORSIZE];
    char out[STYLESIZE];
    unsigned int length = stylename(property, name);
    unsigned int count = 0;
    unsigned int i = 0;

    while (style && style[i])
    {

        unsigned int start;
        unsigned int end;

        while (style[i] && (isspace(style[i]) || style[i] == ';'))
            i++;

        start = i;

        while (style[i] && style[i] != ';')
            i++;

        end = start + buffer_findbyte(style + start, i - start, ':');

        while (end > start && isspace(style[end - 1]))
            end--;

        if (i > start && !(end - start == length && buffer_match(style + start, name, length)) && count + i - start + 2 < STYLESIZE)
        {

            buffer_copy(out + count, style + start, i - start);

            count += i - start;
            out[count++] = ';';

        }

    }

    if (newvalue[0])
        count += cstring_write_fmt(out, STYLESIZE - 1, count, "%s:%s;", name, newvalue);

    out[count] = '\0';

    html_setattribute(js->document, node, "style", out);

    js->changed = 1;

}

static struct js_value getstyle(struct js_engine *js, unsigned int node, char *property)
{

    char *style = html_getattribute(js->document, node, "style");
    char name[SELECTORSIZE];
    struct declaration found = findstyle(style, name, stylename(property, name));
    unsigned int colon;

    if (!found.length)
        return value(JS_STRING, internstring(js, ""));

    colon = buffer_findbyte(style + found.start, found.length, ':') + 1;

    while (colon < found.length && isspace(style[found.start + colon]))
        colon++;

    return value(JS_STRING, addstring(js, style + found.start + colon, found.length - colon));

}

static void setclass(struct js_engine *js, unsigned int node, char *word, unsigned int add)
{

    char *list = html_getattribute(js->document, node, "class");
    char out[STYLESIZE];
    unsigned int length = cstring_length(word);
    unsigned int count = 0;
    unsigned int i = 0;

    if (!length || hasword(list, word, length) == add)
        return;

    while (list && list[i])
    {

        unsigned int start;

        while (list[i] && isspace(list[i]))
            i++;

        start = i;

        while (list[i] && !isspace(list[i]))
            i++;

        if (i > start && !(i - start == length && buffer_match(list + start, word, length)) && count + i - start + 2 < STYLESIZE)
        {

            if (count)
                out[count++] = ' ';

            buffer_copy(out + count, list + start, i - start);

            count += i - start;

        }

    }

    if (add && count + length + 2 < STYLESIZE)
    {

        if (count)
            out[count++] = ' ';

        buffer_copy(out + count, word, length);

        count += length;

    }

    out[count] = '\0';

    html_setattribute(js->document, node, "class", out);

    js->changed = 1;

}

static char *regexflags(struct js_engine *js, struct js_value *regex)
{

    char *pattern = getstring(js, js->objects[regex->number].data);

    return pattern + cstring_length(pattern) + 1;

}

static unsigned int isregex(struct js_engine *js, struct js_value *candidate)
{

    return candidate->type == JS_OBJECT && js->objects[candidate->number].kind == KIND_REGEX;

}

static unsigned int isglobal(struct js_engine *js, struct js_value *regex)
{

    char *flags = regexflags(js, regex);

    return buffer_findbyte(flags, cstring_length(flags), 'g') < cstring_length(flags);

}

static void addnative(struct js_engine *js, unsigned int object, char *name, unsigned int native)
{

    struct js_value method = value(JS_OBJECT, js->natives[native]);

    putproperty(js, object, internstring(js, name), &method);

}

static struct js_value selectvalue(struct js_engine *js, unsigned int node)
{

    struct html_document *document = js->document;
    unsigned int option = 0;
    unsigned int current;
    char content[TEXTSIZE];

    /* a select gives the value of its selected option, or of the first one; an option without a value gives its text */
    for (current = (document->nodes[node].tag == HTML_TAG_OPTION) ? node : html_next(document, node, node); current; current = html_next(document, current, node))
    {

        if (document->nodes[current].tag != HTML_TAG_OPTION)
            continue;

        if (!option || html_getattribute(document, current, "selected"))
            option = current;

        if (current == node)
            break;

    }

    if (!option)
        return value(JS_STRING, internstring(js, ""));

    if (html_getattribute(document, option, "value"))
        return stringvalue(js, html_getattribute(document, option, "value"));

    return value(JS_STRING, addstring(js, content, collecttext(js, option, content, TEXTSIZE)));

}

static struct js_value findmethod(struct js_engine *js, struct method *methods, unsigned int count, char *name)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (cstring_match(methods[i].name, name))
            return value(JS_OBJECT, nativefor(js, methods[i].native));

    }

    return undefined();

}

static struct js_value cachedobject(struct js_engine *js, struct js_value *target, unsigned int name, unsigned int kind, unsigned int data)
{

    struct js_property *property = findproperty(js, target->number, name);
    struct js_value created;

    if (property)
        return property->value;

    created = value(JS_OBJECT, newobject(js, kind, 0, data));

    if (!created.number)
        return undefined();

    putproperty(js, target->number, name, &created);

    return created;

}

static struct js_value getproperty(struct js_engine *js, struct js_value *target, unsigned int name)
{

    char *text = getstring(js, name);
    struct js_object *object;

    if (target->type == JS_STRING)
    {

        if (cstring_match(text, "length"))
            return value(JS_NUMBER, cstring_length(getstring(js, target->number)));

        if (cstring_isdigit(text[0]))
        {

            unsigned int index = parseinteger(text);

            return (index < cstring_length(getstring(js, target->number))) ? value(JS_STRING, addstring(js, getstring(js, target->number) + index, 1)) : undefined();

        }

        return findmethod(js, stringmethods, STRINGMETHODS, text);

    }

    if (target->type == JS_NUMBER || target->type == JS_BOOLEAN)
    {

        if (cstring_match(text, "toFixed"))
            return value(JS_OBJECT, nativefor(js, NATIVE_TOFIXED));

        if (cstring_match(text, "toString"))
            return value(JS_OBJECT, nativefor(js, NATIVE_TOSTRING));

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

        if (!cstring_isdigit(text[0]) && findmethod(js, arraymethods, ARRAYMETHODS, text).type == JS_OBJECT)
            return findmethod(js, arraymethods, ARRAYMETHODS, text);

        break;

    case KIND_ELEMENT:
        if (cstring_match(text, "nodeType"))
            return value(JS_NUMBER, (js->document->nodes[object->data].type == HTML_NODE_TEXT) ? 3 : 1);

        if (cstring_match(text, "parentNode") || cstring_match(text, "parentElement"))
            return elementvalue(js, js->document->nodes[object->data].parent);

        if (cstring_match(text, "nextSibling") || cstring_match(text, "previousSibling"))
            return nodevalue(js, (text[0] == 'n') ? js->document->nodes[object->data].next : js->document->nodes[object->data].prev);

        if (cstring_match(text, "remove"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVE));

        if (js->document->nodes[object->data].type == HTML_NODE_TEXT)
        {

            struct html_node *node = &js->document->nodes[object->data];

            if (cstring_match(text, "textContent") || cstring_match(text, "nodeValue") || cstring_match(text, "data"))
                return value(JS_STRING, addstring(js, html_getstring(js->document, node->text), node->length));

            break;

        }

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

        if (cstring_match(text, "removeEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVEEVENTLISTENER));

        if (cstring_match(text, "tagName") || cstring_match(text, "nodeName"))
        {

            struct js_value name = stringvalue(js, html_getstring(js->document, js->document->nodes[object->data].name));
            char *upper = getstring(js, name.number);
            unsigned int i;

            for (i = 0; upper[i]; i++)
                upper[i] = (upper[i] >= 'a' && upper[i] <= 'z') ? upper[i] - 32 : upper[i];

            return value(JS_STRING, internstring(js, upper));

        }

        if (cstring_match(text, "appendChild") || cstring_match(text, "append"))
            return value(JS_OBJECT, nativefor(js, NATIVE_APPEND));

        if (cstring_match(text, "prepend"))
            return value(JS_OBJECT, nativefor(js, NATIVE_PREPEND));

        if (cstring_match(text, "replaceChildren"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REPLACECHILDREN));

        if (cstring_match(text, "click"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CLICK));

        if (cstring_match(text, "dispatchEvent"))
            return value(JS_OBJECT, nativefor(js, NATIVE_DISPATCHEVENT));

        /* nothing scrolls and there is no focus to move */
        if (cstring_match(text, "focus") || cstring_match(text, "blur") || cstring_match(text, "scrollIntoView"))
            return value(JS_OBJECT, nativefor(js, NATIVE_NOOP));

        if (cstring_match(text, "insertBefore"))
            return value(JS_OBJECT, nativefor(js, NATIVE_INSERTBEFORE));

        if (cstring_match(text, "removeChild"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVECHILD));

        if (cstring_match(text, "insertAdjacentHTML"))
            return value(JS_OBJECT, nativefor(js, NATIVE_INSERTADJACENTHTML));

        if (cstring_match(text, "innerHTML"))
            return innerhtml(js, object->data);

        if (cstring_match(text, "firstChild") || cstring_match(text, "lastChild"))
            return nodevalue(js, (text[0] == 'f') ? js->document->nodes[object->data].first : js->document->nodes[object->data].last);

        if (cstring_match(text, "childNodes"))
        {

            struct js_value children = newarray(js);
            unsigned int node;

            for (node = js->document->nodes[object->data].first; node; node = js->document->nodes[node].next)
            {

                struct js_value child = nodevalue(js, node);

                if (child.type == JS_OBJECT)
                    pusharray(js, &children, &child);

            }

            return children;

        }

        if (cstring_match(text, "classList"))
            return cachedobject(js, target, name, KIND_CLASSLIST, object->data);

        if (cstring_match(text, "style"))
            return cachedobject(js, target, name, KIND_STYLE, object->data);

        if (cstring_match(text, "dataset"))
            return cachedobject(js, target, name, KIND_DATASET, object->data);

        if (cstring_match(text, "querySelector"))
            return value(JS_OBJECT, nativefor(js, NATIVE_QUERYSELECTOR));

        if (cstring_match(text, "querySelectorAll"))
            return value(JS_OBJECT, nativefor(js, NATIVE_QUERYSELECTORALL));

        if (cstring_match(text, "getElementsByClassName"))
            return value(JS_OBJECT, nativefor(js, NATIVE_BYCLASS));

        if (cstring_match(text, "getElementsByTagName"))
            return value(JS_OBJECT, nativefor(js, NATIVE_BYTAG));

        if (cstring_match(text, "closest"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CLOSEST));

        if (cstring_match(text, "matches"))
            return value(JS_OBJECT, nativefor(js, NATIVE_MATCHES));

        if (cstring_match(text, "firstElementChild") || cstring_match(text, "nextElementSibling") || cstring_match(text, "previousElementSibling"))
        {

            unsigned int node = (text[0] == 'f') ? js->document->nodes[object->data].first : (text[0] == 'n') ? js->document->nodes[object->data].next : js->document->nodes[object->data].prev;

            while (node && (js->document->nodes[node].type != HTML_NODE_ELEMENT || html_isgenerated(js->document, node)))
                node = (text[0] == 'p') ? js->document->nodes[node].prev : js->document->nodes[node].next;

            return elementvalue(js, node);

        }

        if (cstring_match(text, "children"))
        {

            struct js_value children = newarray(js);
            unsigned int node;

            for (node = js->document->nodes[object->data].first; node; node = js->document->nodes[node].next)
            {

                struct js_value child = elementvalue(js, node);

                if (child.type == JS_OBJECT)
                    pusharray(js, &children, &child);

            }

            return children;

        }

        if (cstring_match(text, "textContent") || cstring_match(text, "innerText"))
        {

            char content[TEXTSIZE];
            unsigned int length = collecttext(js, object->data, content, TEXTSIZE);

            return value(JS_STRING, addstring(js, content, length));

        }

        if (cstring_match(text, "value") && (js->document->nodes[object->data].tag == HTML_TAG_SELECT || (js->document->nodes[object->data].tag == HTML_TAG_OPTION && !html_getattribute(js->document, object->data, "value"))))
            return selectvalue(js, object->data);

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

        if (cstring_match(text, "createElement"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CREATEELEMENT));

        if (cstring_match(text, "location"))
            return findproperty(js, js->global, name)->value;

        if (!findproperty(js, target->number, name))
        {

            unsigned int node;

            /* forms and images are reachable by name, as in document.myform */
            for (node = 1; node < js->document->nodecount; node++)
            {

                char *named = (js->document->nodes[node].type == HTML_NODE_ELEMENT && !html_isgenerated(js->document, node)) ? html_getattribute(js->document, node, "name") : 0;

                if (named && cstring_match(named, text) && (cstring_match(html_getstring(js->document, js->document->nodes[node].name), "form") || js->document->nodes[node].tag == HTML_TAG_IMG))
                    return elementvalue(js, node);

            }

        }

        if (cstring_match(text, "createTextNode"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CREATETEXTNODE));

        if (cstring_match(text, "querySelector"))
            return value(JS_OBJECT, nativefor(js, NATIVE_QUERYSELECTOR));

        if (cstring_match(text, "querySelectorAll"))
            return value(JS_OBJECT, nativefor(js, NATIVE_QUERYSELECTORALL));

        if (cstring_match(text, "getElementsByClassName"))
            return value(JS_OBJECT, nativefor(js, NATIVE_BYCLASS));

        if (cstring_match(text, "getElementsByTagName"))
            return value(JS_OBJECT, nativefor(js, NATIVE_BYTAG));

        if (cstring_match(text, "addEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_ADDEVENTLISTENER));

        if (cstring_match(text, "removeEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVEEVENTLISTENER));

        break;

    case KIND_WINDOW:
        if (cstring_match(text, "addEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_ADDEVENTLISTENER));

        if (cstring_match(text, "removeEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVEEVENTLISTENER));

        if (cstring_match(text, "setTimeout") || cstring_match(text, "requestAnimationFrame"))
            return value(JS_OBJECT, nativefor(js, NATIVE_SETTIMEOUT));

        if (cstring_match(text, "scrollTo") || cstring_match(text, "scroll"))
            return value(JS_OBJECT, nativefor(js, NATIVE_NOOP));

        /* the page is laid out as one picture that is never scrolled */
        if (cstring_match(text, "scrollY") || cstring_match(text, "scrollX") || cstring_match(text, "pageYOffset") || cstring_match(text, "pageXOffset"))
            return value(JS_NUMBER, 0);

        break;

    case KIND_CLASSLIST:
        if (cstring_match(text, "add"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CLASSADD));

        if (cstring_match(text, "remove"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CLASSREMOVE));

        if (cstring_match(text, "toggle"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CLASSTOGGLE));

        if (cstring_match(text, "contains"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CLASSCONTAINS));

        break;

    case KIND_DATASET:
        {

            char attribute[SELECTORSIZE + 5];
            char *found;

            buffer_copy(attribute, "data-", 5);
            stylename(text, attribute + 5);

            found = html_getattribute(js->document, object->data, attribute);

            return (found) ? stringvalue(js, found) : undefined();

        }

    case KIND_STYLE:
        if (cstring_match(text, "cssText"))
        {

            char *style = html_getattribute(js->document, object->data, "style");

            return (style) ? stringvalue(js, style) : value(JS_STRING, internstring(js, ""));

        }

        if (cstring_match(text, "getPropertyValue"))
            return value(JS_OBJECT, nativefor(js, NATIVE_PROPERTYVALUE));

        return getstyle(js, object->data, text);

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

        if (cstring_match(text, "removeEventListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_REMOVEEVENTLISTENER));

        if (cstring_match(text, "addListener"))
            return value(JS_OBJECT, nativefor(js, NATIVE_NOOP));

        break;

    case KIND_CONSOLE:
        return value(JS_OBJECT, nativefor(js, NATIVE_NOOP));

    case KIND_MAP:
    case KIND_SET:
        if (cstring_match(text, "size"))
            return value(JS_NUMBER, js->objects[object->data].data / 2);

        return findmethod(js, collectionmethods, COLLECTIONMETHODS, text);

    case KIND_REGEX:
        if (cstring_match(text, "test"))
            return value(JS_OBJECT, nativefor(js, NATIVE_TEST));

        if (cstring_match(text, "exec"))
            return value(JS_OBJECT, nativefor(js, NATIVE_EXEC));

        if (cstring_match(text, "source"))
            return value(JS_STRING, object->data);

        if (cstring_match(text, "flags"))
            return stringvalue(js, regexflags(js, target));

        if (cstring_match(text, "global"))
            return value(JS_BOOLEAN, isglobal(js, target));

        break;

    case KIND_FUNCTION:
        if (cstring_match(text, "prototype") && !findproperty(js, target->number, name))
        {

            struct js_value prototype = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

            if (!prototype.number)
                return undefined();

            putproperty(js, prototype.number, internstring(js, "constructor"), target);
            putproperty(js, target->number, name, &prototype);

            return prototype;

        }

    /* fall through */
    case KIND_NATIVE:
    case KIND_BOUND:
        if (cstring_match(text, "call"))
            return value(JS_OBJECT, nativefor(js, NATIVE_CALL));

        if (cstring_match(text, "apply"))
            return value(JS_OBJECT, nativefor(js, NATIVE_APPLY));

        if (cstring_match(text, "bind"))
            return value(JS_OBJECT, nativefor(js, NATIVE_BIND));

        break;

    }

    for (object = &js->objects[target->number]; object; object = (object->kind == KIND_PLAIN && object->parent) ? &js->objects[object->parent] : 0)
    {

        unsigned int index;

        for (index = object->property; index; index = js->properties[index].next)
        {

            struct js_value *found = &js->properties[index].value;

            if (js->properties[index].name != name)
                continue;

            if (found->type == JS_OBJECT && js->objects[found->number].kind == KIND_GETTER)
            {

                struct call call;
                unsigned int argument;

                call.function = value(JS_OBJECT, js->objects[found->number].data);
                call.count = 0;

                copyvalue(&call.self, target);

                for (argument = 0; argument < ARGUMENTS; argument++)
                    call.arguments[argument] = undefined();

                return callfunction(js, &call);

            }

            return js->properties[index].value;

        }

    }

    if (cstring_match(text, "hasOwnProperty"))
        return value(JS_OBJECT, nativefor(js, NATIVE_HASOWNPROPERTY));

    return undefined();

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

    if (object->kind == KIND_DATASET)
    {

        char attribute[SELECTORSIZE + 5];

        buffer_copy(attribute, "data-", 5);
        stylename(getstring(js, name), attribute + 5);
        html_setattribute(js->document, object->data, attribute, getstring(js, tostring(js, newvalue)));

        js->changed = 1;

        return;

    }

    if (object->kind == KIND_STYLE)
    {

        if (cstring_match(getstring(js, name), "cssText"))
        {

            html_setattribute(js->document, object->data, "style", getstring(js, tostring(js, newvalue)));

            js->changed = 1;

        }

        else
        {

            setstyle(js, object->data, getstring(js, name), getstring(js, tostring(js, newvalue)));

        }

        return;

    }

    if (object->kind == KIND_ELEMENT && cstring_match(getstring(js, name), "innerHTML"))
    {

        char *source = getstring(js, tostring(js, newvalue));

        html_parsefragment(js->document, object->data, source, cstring_length(source));

        js->changed = 1;

        return;

    }

    if (object->kind == KIND_ELEMENT && (cstring_match(getstring(js, name), "textContent") || cstring_match(getstring(js, name), "innerText") || (js->document->nodes[object->data].type == HTML_NODE_TEXT && (cstring_match(getstring(js, name), "nodeValue") || cstring_match(getstring(js, name), "data")))))
    {

        html_settext(js->document, object->data, getstring(js, tostring(js, newvalue)));

        js->changed = 1;

        return;

    }

    if (object->kind == KIND_ELEMENT && attributename(getstring(js, name)))
    {

        html_setattribute(js->document, object->data, attributename(getstring(js, name)), getstring(js, tostring(js, newvalue)));

        js->changed = 1;

        return;

    }

    if (object->kind == KIND_ARRAY && cstring_match(getstring(js, name), "length"))
    {

        unsigned int length = tonumber(js, newvalue);

        if (length < object->data)
            object->data = length;

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

static struct js_value itemat(struct js_engine *js, struct js_value *array, unsigned int index)
{

    return getproperty(js, array, numberstring(js, index));

}

static void setitem(struct js_engine *js, struct js_value *array, unsigned int index, struct js_value *item)
{

    setproperty(js, array, numberstring(js, index), item);

}

static unsigned int lengthof(struct js_engine *js, struct js_value *array)
{

    return (array->type == JS_OBJECT && js->objects[array->number].kind == KIND_ARRAY) ? js->objects[array->number].data : 0;

}

static int findentry(struct js_engine *js, struct js_value *entries, struct js_value *key)
{

    unsigned int i;

    for (i = 0; i < lengthof(js, entries); i += 2)
    {

        struct js_value item = itemat(js, entries, i);

        if (strictequal(js, &item, key))
            return i;

    }

    return -1;

}

static void putentry(struct js_engine *js, struct js_value *entries, struct js_value *key, struct js_value *item)
{

    int found = findentry(js, entries, key);

    if (found >= 0)
    {

        setitem(js, entries, found + 1, item);

        return;

    }

    pusharray(js, entries, key);
    pusharray(js, entries, item);

}

/* a map or set keeps its entries as key, value pairs in one array; a set stores each value twice */
static struct js_value listof(struct js_engine *js, struct js_value *source)
{

    struct js_value result;
    struct js_value entries;
    unsigned int kind = (source->type == JS_OBJECT) ? js->objects[source->number].kind : 0;
    unsigned int i;

    copyvalue(&result, source);

    if (kind != KIND_MAP && kind != KIND_SET)
        return result;

    result = newarray(js);
    entries = value(JS_OBJECT, js->objects[source->number].data);

    for (i = 0; i < lengthof(js, &entries); i += 2)
    {

        struct js_value item = itemat(js, &entries, i + 1);

        if (kind == KIND_MAP)
        {

            struct js_value pair = newarray(js);
            struct js_value key = itemat(js, &entries, i);

            pusharray(js, &pair, &key);
            pusharray(js, &pair, &item);

            item = pair;

        }

        pusharray(js, &result, &item);

    }

    return result;

}

static struct js_value callback(struct js_engine *js, struct js_value *function, struct js_value *array, unsigned int index, struct js_value *extra)
{

    struct call each;
    unsigned int argument;

    copyvalue(&each.function, function);

    each.self = undefined();
    each.count = 0;

    if (extra)
        copyvalue(&each.arguments[each.count++], extra);

    each.arguments[each.count++] = itemat(js, array, index);
    each.arguments[each.count++] = value(JS_NUMBER, index);
    copyvalue(&each.arguments[each.count++], array);

    for (argument = each.count; argument < ARGUMENTS; argument++)
        each.arguments[argument] = undefined();

    return callfunction(js, &each);

}

static struct js_value keysof(struct js_engine *js, unsigned int object, unsigned int values)
{

    struct js_value result = newarray(js);
    unsigned int count = 0;
    unsigned int property;
    unsigned int i;

    for (property = js->objects[object].property; property; property = js->properties[property].next)
        count++;

    /* properties are kept newest first, so they are walked from the end to keep their order */
    for (i = count; i > 0; i--)
    {

        unsigned int position;
        struct js_value item;

        property = js->objects[object].property;

        for (position = 1; position < i; position++)
            property = js->properties[property].next;

        item = (values) ? js->properties[property].value : value(JS_STRING, js->properties[property].name);

        pusharray(js, &result, &item);

    }

    return result;

}

static int compare(struct js_engine *js, struct js_value *function, struct js_value *a, struct js_value *b)
{

    if (isfunction(js, function))
    {

        struct call order;
        struct js_value result;
        unsigned int argument;

        copyvalue(&order.function, function);

        order.self = undefined();
        order.count = 2;
        copyvalue(&order.arguments[0], a);
        copyvalue(&order.arguments[1], b);

        for (argument = 2; argument < ARGUMENTS; argument++)
            order.arguments[argument] = undefined();

        result = callfunction(js, &order);

        return tonumber(js, &result);

    }

    {

        char *x = getstring(js, tostring(js, a));
        char *y = getstring(js, tostring(js, b));

        while (x[0] && x[0] == y[0])
        {

            x++;
            y++;

        }

        return (unsigned char)x[0] - (unsigned char)y[0];

    }

}

static void stringify(struct output *output, struct js_value *item, unsigned int depth)
{

    struct js_engine *js = output->js;

    if (depth > JSONDEPTH || isfunction(js, item) || item->type == JS_UNDEFINED)
    {

        put(output, "null", 4);

        return;

    }

    if (item->type == JS_STRING)
    {

        char *text = getstring(js, item->number);
        unsigned int i;

        put(output, "\"", 1);

        for (i = 0; text[i]; i++)
        {

            if (text[i] == '"' || text[i] == '\\')
                put(output, "\\", 1);

            if (text[i] == '\n')
                put(output, "\\n", 2);
            else
                put(output, text + i, 1);

        }

        put(output, "\"", 1);

        return;

    }

    if (item->type != JS_OBJECT)
    {

        char *text = getstring(js, tostring(js, item));

        put(output, text, cstring_length(text));

        return;

    }

    if (js->objects[item->number].kind == KIND_ARRAY)
    {

        unsigned int i;

        put(output, "[", 1);

        for (i = 0; i < js->objects[item->number].data; i++)
        {

            struct js_value element = itemat(js, item, i);

            if (i)
                put(output, ",", 1);

            stringify(output, &element, depth + 1);

        }

        put(output, "]", 1);

        return;

    }

    {

        unsigned int property = js->objects[item->number].property;
        unsigned int count = 0;
        unsigned int written = 0;
        unsigned int i;

        while (property)
        {

            count++;
            property = js->properties[property].next;

        }

        put(output, "{", 1);

        for (i = count; i > 0; i--)
        {

            unsigned int position;
            struct js_value name;

            property = js->objects[item->number].property;

            for (position = 1; position < i; position++)
                property = js->properties[property].next;

            if (isfunction(js, &js->properties[property].value) || js->properties[property].value.type == JS_UNDEFINED)
                continue;

            if (written++)
                put(output, ",", 1);

            name = value(JS_STRING, js->properties[property].name);

            stringify(output, &name, depth + 1);
            put(output, ":", 1);
            stringify(output, &js->properties[property].value, depth + 1);

        }

        put(output, "}", 1);

    }

}

static void skipjsonspace(struct reader *reader)
{

    while (isspace(reader->text[reader->offset]))
        reader->offset++;

}

static struct js_value readjson(struct js_engine *js, struct reader *reader, unsigned int depth)
{

    char *text = reader->text;
    char c;

    skipjsonspace(reader);

    c = text[reader->offset];

    if (depth > JSONDEPTH || reader->failed)
    {

        reader->failed = 1;

        return undefined();

    }

    if (c == '"')
    {

        unsigned int start = js->stringcount;

        reader->offset++;

        while (text[reader->offset] && text[reader->offset] != '"')
        {

            char next = text[reader->offset++];

            if (next == '\\' && text[reader->offset])
            {

                next = text[reader->offset++];
                next = (next == 'n') ? '\n' : (next == 't') ? '\t' : (next == 'r') ? '\r' : next;

            }

            if (js->stringcount + 2 >= JS_STRINGSIZE)
            {

                reader->failed = 1;

                return undefined();

            }

            js->strings[js->stringcount++] = next;

        }

        if (text[reader->offset] == '"')
            reader->offset++;
        else
            reader->failed = 1;

        js->strings[js->stringcount++] = '\0';

        return value(JS_STRING, start);

    }

    if (c == '[' || c == '{')
    {

        struct js_value result = (c == '[') ? newarray(js) : value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));
        char close = (c == '[') ? ']' : '}';

        reader->offset++;

        skipjsonspace(reader);

        if (text[reader->offset] == close)
        {

            reader->offset++;

            return result;

        }

        while (!reader->failed && result.type == JS_OBJECT && result.number)
        {

            struct js_value item;

            if (c == '{')
            {

                struct js_value key = readjson(js, reader, depth + 1);

                skipjsonspace(reader);

                if (key.type != JS_STRING || text[reader->offset] != ':')
                {

                    reader->failed = 1;

                    break;

                }

                reader->offset++;
                item = readjson(js, reader, depth + 1);

                putproperty(js, result.number, internstring(js, getstring(js, key.number)), &item);

            }

            else
            {

                item = readjson(js, reader, depth + 1);

                pusharray(js, &result, &item);

            }

            skipjsonspace(reader);

            if (text[reader->offset] == ',')
            {

                reader->offset++;

                continue;

            }

            if (text[reader->offset] == close)
                reader->offset++;
            else
                reader->failed = 1;

            break;

        }

        return result;

    }

    if (buffer_match(text + reader->offset, "true", 4) || buffer_match(text + reader->offset, "null", 4))
    {

        reader->offset += 4;

        return (c == 't') ? value(JS_BOOLEAN, 1) : value(JS_NULL, 0);

    }

    if (buffer_match(text + reader->offset, "false", 5))
    {

        reader->offset += 5;

        return value(JS_BOOLEAN, 0);

    }

    if (c == '-' || cstring_isdigit(c))
    {

        int number = parseinteger(text + reader->offset);

        reader->offset++;

        while (cstring_isdigit(text[reader->offset]) || text[reader->offset] == '.' || text[reader->offset] == 'e' || text[reader->offset] == 'E' || text[reader->offset] == '+' || text[reader->offset] == '-')
            reader->offset++;

        return value(JS_NUMBER, number);

    }

    reader->failed = 1;

    return undefined();

}

static unsigned int renew(struct regex *re, unsigned int type)
{

    struct renode *node;

    if (re->nodecount >= REGEXNODES)
    {

        re->failed = 1;

        return 0;

    }

    node = &re->nodes[re->nodecount];

    buffer_clear(node, sizeof (struct renode));

    node->type = type;
    node->min = 1;
    node->max = 1;

    return re->nodecount++;

}

static unsigned int recompilegroup(struct regex *re, unsigned int capture);

static void requantifier(struct regex *re, unsigned int index)
{

    struct renode *node = &re->nodes[index];
    char c = re->pattern[re->offset];

    if (c == '*' || c == '+' || c == '?')
    {

        node->min = (c == '+') ? 1 : 0;
        node->max = (c == '?') ? 1 : REPEATMAX;
        re->offset++;

    }

    else if (c == '{' && cstring_isdigit(re->pattern[re->offset + 1]))
    {

        re->offset++;
        node->min = parseinteger(re->pattern + re->offset);
        node->max = node->min;

        while (cstring_isdigit(re->pattern[re->offset]))
            re->offset++;

        if (re->pattern[re->offset] == ',')
        {

            re->offset++;
            node->max = (cstring_isdigit(re->pattern[re->offset])) ? (unsigned int)parseinteger(re->pattern + re->offset) : REPEATMAX;

            while (cstring_isdigit(re->pattern[re->offset]))
                re->offset++;

        }

        if (re->pattern[re->offset] == '}')
            re->offset++;
        else
            re->failed = 1;

    }

    else
    {

        return;

    }

    if (re->pattern[re->offset] == '?')
    {

        node->lazy = 1;
        re->offset++;

    }

}

static unsigned int reescape(struct regex *re, unsigned int index, char c)
{

    struct renode *node = &re->nodes[index];

    switch (c)
    {

    case 'd':
    case 'D':
        node->type = RE_DIGIT;

        break;

    case 'w':
    case 'W':
        node->type = RE_WORD;

        break;

    case 's':
    case 'S':
        node->type = RE_SPACE;

        break;

    case 'b':
        node->type = RE_BOUNDARY;

        break;

    case 'B':
        node->type = RE_NOTBOUNDARY;

        break;

    default:
        node->type = RE_CHAR;
        node->value = (c == 'n') ? '\n' : (c == 't') ? '\t' : (c == 'r') ? '\r' : (unsigned char)c;

        return index;

    }

    node->negated = (c >= 'A' && c <= 'Z');

    return index;

}

static unsigned int recompilesequence(struct regex *re)
{

    unsigned int first = 0;
    unsigned int last = 0;

    while (!re->failed && re->pattern[re->offset] && re->pattern[re->offset] != '|' && re->pattern[re->offset] != ')')
    {

        char c = re->pattern[re->offset++];
        unsigned int index;

        if (c == '(')
        {

            unsigned int capture = 0;

            if (re->pattern[re->offset] == '?')
            {

                /* only non-capturing groups; lookarounds are not supported */
                if (re->pattern[re->offset + 1] != ':')
                {

                    re->failed = 1;

                    break;

                }

                re->offset += 2;

            }

            else if (re->groups < CAPTURES)
            {

                capture = re->groups++;

            }

            index = recompilegroup(re, capture);

            if (re->pattern[re->offset] == ')')
                re->offset++;
            else
                re->failed = 1;

        }

        else
        {

            index = renew(re, RE_CHAR);

            if (!index)
                break;

            if (c == '.')
            {

                re->nodes[index].type = RE_ANY;

            }

            else if (c == '^' || c == '$')
            {

                re->nodes[index].type = (c == '^') ? RE_START : RE_END;

            }

            else if (c == '\\' && re->pattern[re->offset])
            {

                reescape(re, index, re->pattern[re->offset++]);

            }

            else if (c == '[')
            {

                re->nodes[index].type = RE_CLASS;
                re->nodes[index].value = re->offset;

                if (re->pattern[re->offset] == '^')
                {

                    re->nodes[index].negated = 1;
                    re->offset++;

                }

                re->nodes[index].child = re->offset;

                if (re->pattern[re->offset] == ']')
                    re->offset++;

                while (re->pattern[re->offset] && re->pattern[re->offset] != ']')
                    re->offset += (re->pattern[re->offset] == '\\' && re->pattern[re->offset + 1]) ? 2 : 1;

                if (re->pattern[re->offset] == ']')
                    re->offset++;
                else
                    re->failed = 1;

            }

            else
            {

                re->nodes[index].value = (unsigned char)c;

            }

        }

        if (!index)
            break;

        requantifier(re, index);

        if (last)
            re->nodes[last].next = index;
        else
            first = index;

        last = index;

    }

    return first;

}

static unsigned int recompilegroup(struct regex *re, unsigned int capture)
{

    unsigned int group = renew(re, RE_GROUP);
    unsigned int last = 0;

    if (!group)
        return 0;

    re->nodes[group].value = capture;

    while (!re->failed)
    {

        unsigned int alternative = renew(re, RE_ALTERNATIVE);

        if (!alternative)
            return 0;

        re->nodes[alternative].child = recompilesequence(re);

        if (last)
            re->nodes[last].alternative = alternative;
        else
            re->nodes[group].child = alternative;

        last = alternative;

        if (re->pattern[re->offset] != '|')
            break;

        re->offset++;

    }

    return group;

}

static char relower(char c)
{

    return (c >= 'A' && c <= 'Z') ? c + 32 : c;

}

static unsigned int iswordchar(char c)
{

    return cstring_isalpha(c) || cstring_isdigit(c) || c == '_';

}

static unsigned int reclass(struct regex *re, struct renode *node, char c)
{

    char *pattern = re->pattern;
    unsigned int i = node->child;
    unsigned int first = 1;

    while (pattern[i] && (pattern[i] != ']' || first))
    {

        char low = pattern[i];
        char high;

        first = 0;

        if (low == '\\' && pattern[i + 1])
        {

            char e = pattern[i + 1];

            i += 2;

            if ((e == 'd' && cstring_isdigit(c)) || (e == 'w' && iswordchar(c)) || (e == 's' && isspace(c)) || (e == 'D' && !cstring_isdigit(c)) || (e == 'W' && !iswordchar(c)) || (e == 'S' && !isspace(c)))
                return 1;

            low = (e == 'n') ? '\n' : (e == 't') ? '\t' : (e == 'd' || e == 'w' || e == 's' || e == 'D' || e == 'W' || e == 'S') ? 0 : e;

            if (low && relower(low) == c)
                return 1;

            continue;

        }

        i++;
        high = low;

        if (pattern[i] == '-' && pattern[i + 1] && pattern[i + 1] != ']')
        {

            high = pattern[i + 1];
            i += 2;

        }

        if (c >= low && c <= high)
            return 1;

        if (re->ignorecase && relower(c) >= relower(low) && relower(c) <= relower(high))
            return 1;

    }

    return 0;

}

static unsigned int rechar(struct regex *re, struct renode *node, char c)
{

    switch (node->type)
    {

    case RE_CHAR:
        return (re->ignorecase) ? relower(c) == relower((char)node->value) : c == (char)node->value;

    case RE_ANY:
        return c != '\n';

    case RE_CLASS:
        return reclass(re, node, (re->ignorecase) ? relower(c) : c) != node->negated;

    case RE_DIGIT:
        return cstring_isdigit(c) != node->negated;

    case RE_WORD:
        return iswordchar(c) != node->negated;

    case RE_SPACE:
        return isspace(c) != node->negated;

    }

    return 0;

}

static int rematch(struct regex *re, unsigned int index, int position, struct frame *frame);

static int rerepeat(struct regex *re, unsigned int index, int position, unsigned int count, struct frame *frame);

static int reone(struct regex *re, unsigned int index, int position, unsigned int count, struct frame *frame)
{

    struct renode *node = &re->nodes[index];
    unsigned int alternative;

    for (alternative = node->child; alternative; alternative = re->nodes[alternative].alternative)
    {

        struct capture saved[CAPTURES];
        struct frame inner;
        int result;

        buffer_copy(saved, re->captures, sizeof (struct capture) * CAPTURES);

        inner.node = index;
        inner.count = count;
        inner.start = position;
        inner.parent = frame;
        result = rematch(re, re->nodes[alternative].child, position, &inner);

        if (result >= 0)
            return result;

        buffer_copy(re->captures, saved, sizeof (struct capture) * CAPTURES);

    }

    return -1;

}

static int rerepeat(struct regex *re, unsigned int index, int position, unsigned int count, struct frame *frame)
{

    struct renode *node = &re->nodes[index];
    int result = -1;

    /* every character matched is a level of recursion, so long subjects give up instead of overflowing the stack */
    if (++re->steps > STEPLIMIT || re->depth >= REGEXDEPTH)
        return -1;

    re->depth++;

    /* a single character repeats in a loop, so only groups recurse per repetition */
    if (node->type != RE_GROUP)
    {

        unsigned int most = count;

        while (most < node->max && position + (int)(most - count) < re->length && rechar(re, node, re->text[position + most - count]))
            most++;

        if (node->lazy)
        {

            for (; count <= most && result < 0; count++)
            {

                if (count >= node->min)
                    result = rematch(re, node->next, position, frame);

                position++;

            }

        }

        else
        {

            for (; most + 1 > count && result < 0; most--)
            {

                if (most >= node->min)
                    result = rematch(re, node->next, position + (int)(most - count), frame);

            }

        }

        re->depth--;

        return result;

    }

    if (!node->lazy && count < node->max)
        result = reone(re, index, position, count, frame);

    if (result < 0 && count >= node->min)
        result = rematch(re, node->next, position, frame);

    if (result < 0 && node->lazy && count < node->max)
        result = reone(re, index, position, count, frame);

    re->depth--;

    return result;

}

static int rematch(struct regex *re, unsigned int index, int position, struct frame *frame)
{

    struct renode *node;

    if (!index)
    {

        struct renode *group;

        if (!frame)
            return position;

        group = &re->nodes[frame->node];

        if (group->value || frame->node == 1)
        {

            re->captures[group->value].start = frame->start;
            re->captures[group->value].end = position;

        }

        /* a repeated group that matched nothing stops repeating */
        if (position == frame->start)
            return (frame->count + 1 >= group->min) ? rematch(re, group->next, position, frame->parent) : -1;

        return rerepeat(re, frame->node, position, frame->count + 1, frame->parent);

    }

    node = &re->nodes[index];

    switch (node->type)
    {

    case RE_START:
        return (position == 0 || re->text[position - 1] == '\n') ? rematch(re, node->next, position, frame) : -1;

    case RE_END:
        return (position == re->length || re->text[position] == '\n') ? rematch(re, node->next, position, frame) : -1;

    case RE_BOUNDARY:
    case RE_NOTBOUNDARY:
        {

            unsigned int before = position > 0 && iswordchar(re->text[position - 1]);
            unsigned int after = position < re->length && iswordchar(re->text[position]);

            return ((before != after) == (node->type == RE_BOUNDARY)) ? rematch(re, node->next, position, frame) : -1;

        }

    }

    return rerepeat(re, index, position, 0, frame);

}

static unsigned int recompile(struct regex *re, char *pattern, char *flags)
{

    unsigned int i;

    re->pattern = pattern;
    re->offset = 0;
    re->failed = 0;
    re->groups = 1;
    re->nodecount = 1;
    re->ignorecase = 0;

    for (i = 0; flags[i]; i++)
    {

        if (flags[i] == 'i')
            re->ignorecase = 1;

    }

    recompilegroup(re, 0);

    if (re->pattern[re->offset])
        re->failed = 1;

    return !re->failed;

}

static int researchfrom(struct regex *re, char *text, int start)
{

    int position;

    re->text = text;
    re->length = cstring_length(text);
    re->steps = 0;
    re->depth = 0;

    for (position = start; position <= re->length; position++)
    {

        unsigned int i;

        for (i = 0; i < CAPTURES; i++)
        {

            re->captures[i].start = -1;
            re->captures[i].end = -1;

        }

        if (rematch(re, 1, position, 0) >= 0)
            return position;

        if (re->steps > STEPLIMIT)
            break;

    }

    return -1;

}

static struct js_value matchresult(struct js_engine *js, struct regex *re)
{

    struct js_value result = newarray(js);
    struct js_value index = value(JS_NUMBER, re->captures[0].start);
    unsigned int i;

    for (i = 0; i < re->groups; i++)
    {

        struct js_value item = (re->captures[i].start >= 0) ? value(JS_STRING, addstring(js, re->text + re->captures[i].start, re->captures[i].end - re->captures[i].start)) : undefined();

        pusharray(js, &result, &item);

    }

    if (result.type == JS_OBJECT)
        putproperty(js, result.number, internstring(js, "index"), &index);

    return result;

}

static unsigned int expandreplacement(struct js_engine *js, struct regex *re, struct js_value *replacement, unsigned int result)
{

    char *text;
    unsigned int i;
    unsigned int start = 0;

    if (isfunction(js, replacement))
    {

        struct call call;
        struct js_value returned;

        copyvalue(&call.function, replacement);
        call.self = undefined();
        call.count = 0;

        for (i = 0; i < re->groups && call.count < ARGUMENTS - 1; i++)
            call.arguments[call.count++] = (re->captures[i].start >= 0) ? value(JS_STRING, addstring(js, re->text + re->captures[i].start, re->captures[i].end - re->captures[i].start)) : undefined();

        call.arguments[call.count++] = value(JS_NUMBER, re->captures[0].start);

        for (i = call.count; i < ARGUMENTS; i++)
            call.arguments[i] = undefined();

        returned = callfunction(js, &call);

        return concat(js, result, tostring(js, &returned));

    }

    text = getstring(js, tostring(js, replacement));

    for (i = 0; text[i]; i++)
    {

        unsigned int group;

        if (text[i] != '$' || !(cstring_isdigit(text[i + 1]) || text[i + 1] == '&'))
            continue;

        group = (text[i + 1] == '&') ? 0 : (unsigned int)(text[i + 1] - '0');
        result = concat(js, result, addstring(js, getstring(js, tostring(js, replacement)) + start, i - start));

        if (group < re->groups && re->captures[group].start >= 0)
            result = concat(js, result, addstring(js, re->text + re->captures[group].start, re->captures[group].end - re->captures[group].start));

        text = getstring(js, tostring(js, replacement));
        start = i + 2;
        i++;

    }

    return concat(js, result, addstring(js, text + start, cstring_length(text + start)));

}

static struct js_value regexreplace(struct js_engine *js, struct js_value *subject, struct js_value *regex, struct js_value *replacement)
{

    struct regex re;
    unsigned int global = isglobal(js, regex);
    unsigned int result = internstring(js, "");
    unsigned int source = tostring(js, subject);
    int position = 0;

    if (!recompile(&re, getstring(js, js->objects[regex->number].data), regexflags(js, regex)))
        return value(JS_STRING, source);

    while (position <= (int)cstring_length(getstring(js, source)) && !js->throwing)
    {

        int found = researchfrom(&re, getstring(js, source), position);
        int end;

        if (found < 0)
            break;

        end = re.captures[0].end;
        result = concat(js, result, addstring(js, getstring(js, source) + position, found - position));
        re.text = getstring(js, source);
        result = expandreplacement(js, &re, replacement, result);
        re.text = getstring(js, source);

        if (end == found)
        {

            if (found < (int)cstring_length(getstring(js, source)))
                result = concat(js, result, addstring(js, getstring(js, source) + found, 1));

            end = found + 1;

        }

        position = end;

        if (!global)
            break;

    }

    if (position < (int)cstring_length(getstring(js, source)))
        result = concat(js, result, addstring(js, getstring(js, source) + position, cstring_length(getstring(js, source) + position)));

    return value(JS_STRING, result);

}

static struct js_value regexmatch(struct js_engine *js, struct js_value *subject, struct js_value *regex, unsigned int native)
{

    struct regex re;
    unsigned int source = tostring(js, subject);
    struct js_value lastindex = getproperty(js, regex, internstring(js, "lastIndex"));
    unsigned int global = isglobal(js, regex);
    int position = (global && native == NATIVE_EXEC) ? tonumber(js, &lastindex) : 0;
    struct js_value all = (global && native == NATIVE_MATCH) ? newarray(js) : value(JS_NULL, 0);
    int found;

    if (!recompile(&re, getstring(js, js->objects[regex->number].data), regexflags(js, regex)))
    {

        throwerror(js, "Unsupported regular expression");

        return undefined();

    }

    while (position <= (int)cstring_length(getstring(js, source)) && (found = researchfrom(&re, getstring(js, source), position)) >= 0)
    {

        struct js_value item;

        if (native == NATIVE_TEST)
            return value(JS_BOOLEAN, 1);

        if (native == NATIVE_SEARCH)
            return value(JS_NUMBER, found);

        if (!(global && native == NATIVE_MATCH))
        {

            if (global)
            {

                struct js_value next = value(JS_NUMBER, re.captures[0].end + (re.captures[0].end == found));

                setproperty(js, regex, internstring(js, "lastIndex"), &next);

            }

            return matchresult(js, &re);

        }

        item = value(JS_STRING, addstring(js, getstring(js, source) + found, re.captures[0].end - found));

        pusharray(js, &all, &item);

        position = (re.captures[0].end > found) ? re.captures[0].end : found + 1;

    }

    if (native == NATIVE_EXEC && global)
    {

        struct js_value zero = value(JS_NUMBER, 0);

        setproperty(js, regex, internstring(js, "lastIndex"), &zero);

    }

    if (native == NATIVE_TEST)
        return value(JS_BOOLEAN, 0);

    if (native == NATIVE_SEARCH)
        return value(JS_NUMBER, -1);

    return (lengthof(js, &all)) ? all : value(JS_NULL, 0);

}

static struct js_value split(struct js_engine *js, struct js_value *self, struct js_value *first)
{

    struct js_value result = newarray(js);
    char *separator = getstring(js, tostring(js, first));
    unsigned int size = cstring_length(separator);
    unsigned int start = 0;
    unsigned int i;

    for (i = 0; getstring(js, self->number)[i]; i++)
    {

        struct js_value piece;

        if (size && !buffer_match(getstring(js, self->number) + i, separator, size))
            continue;

        piece = value(JS_STRING, addstring(js, getstring(js, self->number) + start, (size) ? i - start : 1));

        pusharray(js, &result, &piece);

        start = (size) ? i + size : i + 1;
        i = (size) ? i + size - 1 : i;

    }

    if (size)
    {

        struct js_value piece = value(JS_STRING, addstring(js, getstring(js, self->number) + start, cstring_length(getstring(js, self->number) + start)));

        pusharray(js, &result, &piece);

    }

    return result;

}

static struct js_value regexsplit(struct js_engine *js, struct js_value *subject, struct js_value *regex)
{

    struct regex re;
    struct js_value result = newarray(js);
    unsigned int source = tostring(js, subject);
    int position = 0;
    int from = 0;
    struct js_value piece;

    if (!recompile(&re, getstring(js, js->objects[regex->number].data), regexflags(js, regex)))
        return result;

    while (position < (int)cstring_length(getstring(js, source)))
    {

        int found = researchfrom(&re, getstring(js, source), position);

        if (found < 0)
            break;

        if (re.captures[0].end == found)
        {

            position = found + 1;

            if (found == from)
                continue;

        }

        piece = value(JS_STRING, addstring(js, getstring(js, source) + from, found - from));

        pusharray(js, &result, &piece);

        from = re.captures[0].end;
        position = (re.captures[0].end > found) ? re.captures[0].end : found + 1;

    }

    piece = value(JS_STRING, addstring(js, getstring(js, source) + from, cstring_length(getstring(js, source) + from)));

    pusharray(js, &result, &piece);

    return result;

}

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
    case NATIVE_QUERYSELECTORALL:
        return query(js, element, getstring(js, tostring(js, first)), native->data == NATIVE_QUERYSELECTORALL);

    case NATIVE_BYCLASS:
    case NATIVE_BYTAG:
        {

            char selector[SELECTORSIZE];
            char *name = getstring(js, tostring(js, first));
            unsigned int count = 0;

            if (native->data == NATIVE_BYCLASS)
                selector[count++] = '.';

            for (i = 0; name[i] && count + 1 < SELECTORSIZE; i++)
                selector[count++] = (native->data == NATIVE_BYCLASS && name[i] == ' ') ? '.' : name[i];

            selector[count] = '\0';

            return query(js, element, selector, 1);

        }

    case NATIVE_CLOSEST:
        for (node = element; node && js->document->nodes[node].type == HTML_NODE_ELEMENT; node = js->document->nodes[node].parent)
        {

            if (matchlist(js, node, getstring(js, tostring(js, first))))
                return elementvalue(js, node);

        }

        return value(JS_NULL, 0);

    case NATIVE_MATCHES:
        return value(JS_BOOLEAN, element && matchlist(js, element, getstring(js, tostring(js, first))));

    case NATIVE_CLASSADD:
    case NATIVE_CLASSREMOVE:
        for (i = 0; kind == KIND_CLASSLIST && i < call->count; i++)
            setclass(js, js->objects[self->number].data, getstring(js, tostring(js, &call->arguments[i])), native->data == NATIVE_CLASSADD);

        return undefined();

    case NATIVE_CLASSTOGGLE:
    case NATIVE_CLASSCONTAINS:
        if (kind == KIND_CLASSLIST)
        {

            unsigned int owner = js->objects[self->number].data;
            char *word = getstring(js, tostring(js, first));
            unsigned int has = hasword(html_getattribute(js->document, owner, "class"), word, cstring_length(word));

            if (native->data == NATIVE_CLASSCONTAINS)
                return value(JS_BOOLEAN, has);

            has = (call->count >= 2) ? truthy(js, second) : !has;

            setclass(js, owner, word, has);

            return value(JS_BOOLEAN, has);

        }

        return value(JS_BOOLEAN, 0);

    case NATIVE_FOREACH:
        if (self->type == JS_OBJECT && js->objects[self->number].kind == KIND_ARRAY && isfunction(js, first))
        {

            for (i = 0; i < js->objects[self->number].data && !js->throwing; i++)
            {

                struct call each;
                struct js_value item = getproperty(js, self, numberstring(js, i));
                unsigned int argument;

                copyvalue(&each.function, first);
                copyvalue(&each.self, &item);
                copyvalue(&each.arguments[0], &item);
                copyvalue(&each.arguments[1], &item);
                copyvalue(&each.arguments[2], self);

                each.arguments[1].type = JS_NUMBER;
                each.arguments[1].number = i;
                each.count = 3;

                for (argument = 3; argument < ARGUMENTS; argument++)
                {

                    each.arguments[argument].type = JS_UNDEFINED;
                    each.arguments[argument].number = 0;

                }

                callfunction(js, &each);

            }

        }

        return undefined();

    case NATIVE_ARRAY:
        {

            struct js_value result = newarray(js);

            if (call->count == 1 && first->type == JS_NUMBER)
            {

                if (result.type == JS_OBJECT && first->number > 0)
                    js->objects[result.number].data = first->number;

                return result;

            }

            for (i = 0; i < call->count; i++)
                pusharray(js, &result, &call->arguments[i]);

            return result;

        }

    case NATIVE_ISARRAY:
        return value(JS_BOOLEAN, first->type == JS_OBJECT && js->objects[first->number].kind == KIND_ARRAY);

    case NATIVE_ARRAYFROM:
        {

            struct js_value result = newarray(js);
            unsigned int length = (first->type == JS_STRING) ? cstring_length(getstring(js, first->number)) : 0;

            call->arguments[0] = listof(js, first);

            if (first->type == JS_OBJECT)
            {

                struct js_value count = getproperty(js, first, internstring(js, "length"));

                length = tonumber(js, &count);

            }

            for (i = 0; i < length && !js->throwing; i++)
            {

                struct js_value item = (first->type == JS_STRING) ? value(JS_STRING, addstring(js, getstring(js, first->number) + i, 1)) : itemat(js, first, i);

                pusharray(js, &result, &item);

            }

            for (i = 0; i < length && isfunction(js, second) && !js->throwing; i++)
            {

                struct js_value item = callback(js, second, &result, i, 0);

                setitem(js, &result, i, &item);

            }

            return result;

        }

    case NATIVE_OBJECT:
        return (first->type == JS_OBJECT) ? call->arguments[0] : value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

    case NATIVE_KEYS:
    case NATIVE_VALUES:
        if (self->type == JS_OBJECT && js->objects[self->number].kind == KIND_ARRAY)
        {

            struct js_value result = newarray(js);

            for (i = 0; i < js->objects[self->number].data; i++)
            {

                struct js_value index = value(JS_NUMBER, i);

                pusharray(js, &result, &index);

            }

            return result;

        }

        if (first->type != JS_OBJECT)
            return newarray(js);

        if (js->objects[first->number].kind == KIND_ARRAY)
        {

            struct js_value result = newarray(js);

            for (i = 0; i < js->objects[first->number].data; i++)
            {

                struct js_value item = (native->data == NATIVE_VALUES) ? itemat(js, first, i) : value(JS_STRING, numberstring(js, i));

                pusharray(js, &result, &item);

            }

            return result;

        }

        return keysof(js, first->number, native->data == NATIVE_VALUES);

    case NATIVE_ENTRIES:
        {

            struct js_value result = newarray(js);
            struct js_value keys = (first->type == JS_OBJECT) ? keysof(js, first->number, 0) : newarray(js);

            for (i = 0; i < lengthof(js, &keys); i++)
            {

                struct js_value pair = newarray(js);
                struct js_value key = itemat(js, &keys, i);
                struct js_value item = getproperty(js, first, key.number);

                pusharray(js, &pair, &key);
                pusharray(js, &pair, &item);
                pusharray(js, &result, &pair);

            }

            return result;

        }

    case NATIVE_OBJECTTOSTRING:
        {

            char *name = (self->type == JS_UNDEFINED) ? "Undefined" : (self->type == JS_NULL) ? "Null" : (self->type == JS_STRING) ? "String" : (self->type == JS_NUMBER) ? "Number" : (self->type == JS_BOOLEAN) ? "Boolean" : (kind == KIND_ARRAY) ? "Array" : (isfunction(js, self)) ? "Function" : (kind == KIND_ELEMENT) ? "HTMLElement" : (kind == KIND_REGEX) ? "RegExp" : "Object";

            return value(JS_STRING, concat(js, concat(js, internstring(js, "[object "), internstring(js, name)), internstring(js, "]")));

        }

    case NATIVE_EVENT:
        {

            struct js_value event = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));
            struct js_value type = value(JS_STRING, tostring(js, first));

            if (!event.number)
                return undefined();

            putproperty(js, event.number, internstring(js, "type"), &type);

            if (second->type == JS_OBJECT)
            {

                struct js_value detail = getproperty(js, second, internstring(js, "detail"));

                putproperty(js, event.number, internstring(js, "detail"), &detail);

            }

            return event;

        }

    case NATIVE_DISPATCHEVENT:
        if (element && first->type == JS_OBJECT)
        {

            struct js_value type = getproperty(js, first, internstring(js, "type"));

            bubble(js, element, internstring(js, getstring(js, tostring(js, &type))));

        }

        return value(JS_BOOLEAN, 1);

    case NATIVE_COMPUTEDSTYLE:
        /* scripts cannot see the cascade, so the computed style is the inline one */
        return (nodeof(js, first)) ? getproperty(js, first, internstring(js, "style")) : undefined();

    case NATIVE_PROPERTYVALUE:
        if (kind == KIND_STYLE)
        {

            char name[SELECTORSIZE];
            char *text = getstring(js, tostring(js, first));
            unsigned int count = 0;

            /* the property comes as css-name and is looked up as cssName */
            for (i = 0; text[i] && count + 1 < SELECTORSIZE; i++)
            {

                if (text[i] == '-' && text[i + 1] && count)
                {

                    i++;
                    name[count++] = (text[i] >= 'a' && text[i] <= 'z') ? text[i] - 32 : text[i];

                }

                else
                {

                    name[count++] = text[i];

                }

            }

            name[count] = '\0';

            return getstyle(js, js->objects[self->number].data, name);

        }

        return value(JS_STRING, internstring(js, ""));

    case NATIVE_SYMBOL:
        /* symbols are strings that no ordinary key is likely to collide with */
        return value(JS_STRING, concat(js, concat(js, internstring(js, "@@symbol("), tostring(js, first)), internstring(js, ")")));

    case NATIVE_ASSIGN:
        for (i = 1; i < call->count && first->type == JS_OBJECT; i++)
        {

            unsigned int property;

            if (call->arguments[i].type != JS_OBJECT)
                continue;

            for (property = js->objects[call->arguments[i].number].property; property; property = js->properties[property].next)
            {

                if (!findproperty(js, first->number, js->properties[property].name))
                    putproperty(js, first->number, js->properties[property].name, &js->properties[property].value);

            }

        }

        return call->arguments[0];

    case NATIVE_CREATE:
        return value(JS_OBJECT, newobject(js, KIND_PLAIN, (first->type == JS_OBJECT) ? (unsigned int)first->number : 0, 0));

    case NATIVE_IDENTITY:
        return call->arguments[0];

    case NATIVE_DEFINEPROPERTY:
        if (first->type == JS_OBJECT && call->arguments[2].type == JS_OBJECT)
        {

            struct js_value item = getproperty(js, &call->arguments[2], internstring(js, "value"));

            setproperty(js, first, internstring(js, getstring(js, tostring(js, second))), &item);

        }

        return call->arguments[0];

    case NATIVE_STRINGIFY:
        {

            char buffer[TEXTSIZE * 4];
            struct output output;

            output.js = js;
            output.data = buffer;
            output.count = 0;
            output.size = TEXTSIZE * 4;
            output.full = 0;

            if (first->type == JS_UNDEFINED)
                return undefined();

            stringify(&output, first, 0);

            if (output.full)
            {

                throwerror(js, "Value too large to stringify");

                return undefined();

            }

            return value(JS_STRING, addstring(js, buffer, output.count));

        }

    case NATIVE_PARSE:
        {

            struct reader reader;
            struct js_value result;

            reader.text = getstring(js, tostring(js, first));
            reader.offset = 0;
            reader.failed = 0;
            result = readjson(js, &reader, 0);

            if (reader.failed)
                throwerror(js, "Invalid JSON");

            return result;

        }

    case NATIVE_SQRT:
        {

            int number = tonumber(js, first);
            int root = 0;

            while ((root + 1) * (root + 1) <= number)
                root++;

            return value(JS_NUMBER, root);

        }

    case NATIVE_POW:
        {

            int base = tonumber(js, first);
            int exponent = tonumber(js, second);
            int result = 1;

            while (exponent-- > 0)
                result *= base;

            return value(JS_NUMBER, (tonumber(js, second) < 0) ? 0 : result);

        }

    case NATIVE_CLEARTIMER:
        if (first->type == JS_NUMBER && first->number > 0 && (unsigned int)first->number <= js->timercount)
            js->timers[first->number - 1] = 0;

        return undefined();

    case NATIVE_PARSEINT:
    case NATIVE_NUMBER:
        return value(JS_NUMBER, tonumber(js, first));

    case NATIVE_ISNAN:
        return value(JS_BOOLEAN, first->type == JS_UNDEFINED || (first->type == JS_STRING && !cstring_isdigit(getstring(js, first->number)[0]) && getstring(js, first->number)[0] != '-'));

    case NATIVE_STRING:
        return value(JS_STRING, (call->count) ? tostring(js, first) : internstring(js, ""));

    case NATIVE_BOOLEAN:
        return value(JS_BOOLEAN, truthy(js, first));

    case NATIVE_ERROR:
        {

            struct js_value error = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));
            struct js_value message = value(JS_STRING, tostring(js, first));

            if (error.number)
                putproperty(js, error.number, internstring(js, "message"), &message);

            return error;

        }

    case NATIVE_DOMPARSER:
        {

            struct js_value parser = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

            if (parser.number)
                addnative(js, parser.number, "parseFromString", NATIVE_PARSEFROMSTRING);

            return parser;

        }

    case NATIVE_PARSEFROMSTRING:
        {

            struct js_value parsed = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));
            struct js_value body = elementvalue(js, html_createelement(js->document, "body"));
            char *source = getstring(js, tostring(js, first));

            if (!parsed.number || body.type != JS_OBJECT)
                return undefined();

            html_parsefragment(js->document, nodeof(js, &body), source, cstring_length(source));
            putproperty(js, parsed.number, internstring(js, "body"), &body);

            return parsed;

        }

    case NATIVE_REPLACECHILDREN:
        if (element)
        {

            while (js->document->nodes[element].first)
                html_remove(js->document, js->document->nodes[element].first);

            for (i = 0; i < call->count; i++)
                html_insert(js->document, element, argumentnode(js, &call->arguments[i]), 0);

            js->changed = 1;

        }

        return undefined();

    case NATIVE_CLICK:
        if (element)
            bubble(js, element, internstring(js, "click"));

        return undefined();

    case NATIVE_FULLYEAR:
        return value(JS_NUMBER, 1970);

    case NATIVE_DATE:
        /* there is no clock: every date is the epoch */
        return value(JS_OBJECT, js->date);

    case NATIVE_ZERO:
        return value(JS_NUMBER, 0);

    case NATIVE_TOFIXED:
        {

            struct js_value text = value(JS_STRING, tostring(js, self));
            int digits = tonumber(js, first);

            if (digits > 0)
                text.number = concat(js, concat(js, text.number, internstring(js, ".")), addstring(js, "0000000000", (digits > 10) ? 10 : digits));

            return text;

        }

    case NATIVE_TOSTRING:
        return value(JS_STRING, tostring(js, self));

    case NATIVE_REPEAT:
        {

            unsigned int text = tostring(js, self);
            unsigned int length = cstring_length(getstring(js, text));
            unsigned int start = js->stringcount;
            int count = tonumber(js, first);

            if (count < 0 || start + length * count + 1 > JS_STRINGSIZE)
            {

                throwerror(js, "Out of string memory");

                return undefined();

            }

            for (i = 0; i < (unsigned int)count; i++)
                buffer_copy(js->strings + start + i * length, getstring(js, text), length);

            js->strings[start + length * count] = '\0';
            js->stringcount += length * count + 1;

            return value(JS_STRING, start);

        }

    case NATIVE_PADSTART:
    case NATIVE_PADEND:
        {

            unsigned int result = tostring(js, self);
            unsigned int pad = (call->count >= 2) ? tostring(js, second) : internstring(js, " ");
            unsigned int size = cstring_length(getstring(js, pad));
            unsigned int length = tonumber(js, first);
            unsigned int filler = internstring(js, "");

            if (!size)
                return value(JS_STRING, result);

            for (i = cstring_length(getstring(js, result)); i < length && !js->throwing; i++)
                filler = concat(js, filler, addstring(js, getstring(js, pad) + (cstring_length(getstring(js, filler)) % size), 1));

            return value(JS_STRING, (native->data == NATIVE_PADSTART) ? concat(js, filler, result) : concat(js, result, filler));

        }

    case NATIVE_CHARCODEAT:
        if (self->type == JS_STRING)
        {

            unsigned int index = tonumber(js, first);

            return (index < cstring_length(getstring(js, self->number))) ? value(JS_NUMBER, (unsigned char)getstring(js, self->number)[index]) : value(JS_NUMBER, 0);

        }

        return undefined();

    case NATIVE_FROMCHARCODE:
        {

            unsigned int result = internstring(js, "");

            for (i = 0; i < call->count; i++)
            {

                char text[4];
                unsigned int length = utf8_write(text, 4, tonumber(js, &call->arguments[i]));

                result = concat(js, result, addstring(js, text, length));

            }

            return value(JS_STRING, result);

        }

    case NATIVE_SUBSTR:
        if (self->type == JS_STRING)
        {

            int length = cstring_length(getstring(js, self->number));
            int start = tonumber(js, first);
            int count = (call->count >= 2) ? tonumber(js, second) : length;

            start = (start < 0) ? ((start + length < 0) ? 0 : start + length) : ((start > length) ? length : start);
            count = (count < 0) ? 0 : (start + count > length) ? length - start : count;

            return value(JS_STRING, addstring(js, getstring(js, self->number) + start, count));

        }

        return undefined();

    case NATIVE_LASTINDEXOF:
        if (self->type == JS_STRING)
        {

            char *haystack = getstring(js, self->number);
            char *needle = getstring(js, tostring(js, first));
            int length = cstring_length(needle);
            int position;

            for (position = (int)cstring_length(haystack) - length; position >= 0; position--)
            {

                if (buffer_match(haystack + position, needle, length))
                    return value(JS_NUMBER, position);

            }

            return value(JS_NUMBER, -1);

        }

        for (i = lengthof(js, self); i > 0; i--)
        {

            struct js_value item = itemat(js, self, i - 1);

            if (strictequal(js, &item, first))
                return value(JS_NUMBER, i - 1);

        }

        return value(JS_NUMBER, -1);

    case NATIVE_ARRAYITEM:
        return itemat(js, self, tonumber(js, first));

    case NATIVE_MAP:
    case NATIVE_FILTER:
    case NATIVE_FIND:
    case NATIVE_FINDINDEX:
    case NATIVE_SOME:
    case NATIVE_EVERY:
        {

            struct js_value result = (native->data == NATIVE_MAP || native->data == NATIVE_FILTER) ? newarray(js) : undefined();
            unsigned int length = lengthof(js, self);

            if (!isfunction(js, first))
            {

                throwerror(js, "Value is not a function");

                return undefined();

            }

            for (i = 0; i < length && !js->throwing; i++)
            {

                struct js_value item = itemat(js, self, i);
                struct js_value answer = callback(js, first, self, i, 0);

                if (native->data == NATIVE_MAP)
                    pusharray(js, &result, &answer);
                else if (native->data == NATIVE_FILTER && truthy(js, &answer))
                    pusharray(js, &result, &item);
                else if (native->data == NATIVE_FIND && truthy(js, &answer))
                    return item;
                else if (native->data == NATIVE_FINDINDEX && truthy(js, &answer))
                    return value(JS_NUMBER, i);
                else if (native->data == NATIVE_SOME && truthy(js, &answer))
                    return value(JS_BOOLEAN, 1);
                else if (native->data == NATIVE_EVERY && !truthy(js, &answer))
                    return value(JS_BOOLEAN, 0);

            }

            if (native->data == NATIVE_FINDINDEX)
                return value(JS_NUMBER, -1);

            if (native->data == NATIVE_SOME || native->data == NATIVE_EVERY)
                return value(JS_BOOLEAN, native->data == NATIVE_EVERY);

            return result;

        }

    case NATIVE_REDUCE:
        {

            unsigned int length = lengthof(js, self);
            struct js_value total = (call->count >= 2) ? call->arguments[1] : itemat(js, self, 0);

            if (!isfunction(js, first))
            {

                throwerror(js, "Value is not a function");

                return undefined();

            }

            for (i = (call->count >= 2) ? 0 : 1; i < length && !js->throwing; i++)
                total = callback(js, first, self, i, &total);

            return total;

        }

    case NATIVE_SORT:
    case NATIVE_REVERSE:
        {

            struct js_value items[SORTSIZE];
            unsigned int length = lengthof(js, self);
            unsigned int j;

            if (length > SORTSIZE)
            {

                throwerror(js, "Array too long to sort");

                return call->self;

            }

            for (i = 0; i < length; i++)
                items[i] = itemat(js, self, (native->data == NATIVE_REVERSE) ? length - 1 - i : i);

            for (i = 1; i < length && native->data == NATIVE_SORT && !js->throwing; i++)
            {

                struct js_value item = items[i];

                for (j = i; j > 0 && compare(js, first, &items[j - 1], &item) > 0; j--)
                    items[j] = items[j - 1];

                items[j] = item;

            }

            for (i = 0; i < length; i++)
                setitem(js, self, i, &items[i]);

            return call->self;

        }

    case NATIVE_CONCAT:
        if (self->type == JS_STRING)
        {

            unsigned int result = self->number;

            for (i = 0; i < call->count; i++)
                result = concat(js, result, tostring(js, &call->arguments[i]));

            return value(JS_STRING, result);

        }

        {

            struct js_value result = newarray(js);
            unsigned int argument;

            for (i = 0; i < lengthof(js, self); i++)
            {

                struct js_value item = itemat(js, self, i);

                pusharray(js, &result, &item);

            }

            for (argument = 0; argument < call->count; argument++)
            {

                if (lengthof(js, &call->arguments[argument]) || (call->arguments[argument].type == JS_OBJECT && js->objects[call->arguments[argument].number].kind == KIND_ARRAY))
                {

                    for (i = 0; i < lengthof(js, &call->arguments[argument]); i++)
                    {

                        struct js_value item = itemat(js, &call->arguments[argument], i);

                        pusharray(js, &result, &item);

                    }

                }

                else
                {

                    pusharray(js, &result, &call->arguments[argument]);

                }

            }

            return result;

        }

    case NATIVE_POP:
    case NATIVE_SHIFT:
        {

            unsigned int length = lengthof(js, self);
            struct js_value item;

            if (!length)
                return undefined();

            item = itemat(js, self, (native->data == NATIVE_POP) ? length - 1 : 0);

            for (i = 1; i < length && native->data == NATIVE_SHIFT; i++)
            {

                struct js_value next = itemat(js, self, i);

                setitem(js, self, i - 1, &next);

            }

            js->objects[self->number].data--;

            return item;

        }

    case NATIVE_UNSHIFT:
    case NATIVE_SPLICE:
        {

            unsigned int length = lengthof(js, self);
            int start = (native->data == NATIVE_UNSHIFT) ? 0 : tonumber(js, first);
            int removed = (native->data == NATIVE_UNSHIFT) ? 0 : (call->count >= 2) ? tonumber(js, second) : (int)length;
            unsigned int skip = (native->data == NATIVE_UNSHIFT) ? 0 : 2;
            unsigned int added = (call->count > skip) ? call->count - skip : 0;
            struct js_value result = newarray(js);
            struct js_value items[SORTSIZE];
            unsigned int count = 0;

            if (self->type != JS_OBJECT || js->objects[self->number].kind != KIND_ARRAY)
                return undefined();

            if (length + added > SORTSIZE)
            {

                throwerror(js, "Array too long to splice");

                return result;

            }

            start = (start < 0) ? ((start + (int)length < 0) ? 0 : start + (int)length) : ((start > (int)length) ? (int)length : start);
            removed = (removed < 0) ? 0 : (start + removed > (int)length) ? (int)length - start : removed;

            for (i = 0; i < (unsigned int)start; i++)
                items[count++] = itemat(js, self, i);

            for (i = 0; i < added; i++)
                items[count++] = call->arguments[skip + i];

            for (i = start; i < (unsigned int)(start + removed); i++)
            {

                struct js_value item = itemat(js, self, i);

                pusharray(js, &result, &item);

            }

            for (i = start + removed; i < length; i++)
                items[count++] = itemat(js, self, i);

            for (i = 0; i < count; i++)
                setitem(js, self, i, &items[i]);

            js->objects[self->number].data = count;

            return (native->data == NATIVE_UNSHIFT) ? value(JS_NUMBER, count) : result;

        }

    case NATIVE_FILL:
        for (i = 0; i < lengthof(js, self); i++)
            setitem(js, self, i, first);

        return call->self;

    case NATIVE_TEST:
    case NATIVE_EXEC:
        if (isregex(js, self))
            return regexmatch(js, first, self, native->data);

        return undefined();

    case NATIVE_MATCH:
    case NATIVE_SEARCH:
        if (isregex(js, first))
            return regexmatch(js, self, first, native->data);

        return value(JS_NULL, 0);

    case NATIVE_REGEXP:
        {

            unsigned int pattern = (isregex(js, first)) ? js->objects[first->number].data : addstring(js, getstring(js, tostring(js, first)), cstring_length(getstring(js, tostring(js, first))));
            char *flags = (call->count >= 2) ? getstring(js, tostring(js, second)) : "";

            /* the flags must follow the pattern directly */
            if (isregex(js, first) || js->stringcount != pattern + cstring_length(getstring(js, pattern)) + 1)
                pattern = addstring(js, getstring(js, pattern), cstring_length(getstring(js, pattern)));

            addstring(js, flags, cstring_length(flags));

            return value(JS_OBJECT, newobject(js, KIND_REGEX, 0, pattern));

        }

    case NATIVE_REPLACEALL:
        if (self->type == JS_STRING && first->type == JS_STRING)
            return value(JS_STRING, joinarray(js, split(js, self, first).number, getstring(js, tostring(js, second))));

        return (isregex(js, first)) ? regexreplace(js, self, first, second) : call->self;

    case NATIVE_NEWMAP:
    case NATIVE_NEWSET:
        {

            struct js_value collection = value(JS_OBJECT, newobject(js, (native->data == NATIVE_NEWMAP) ? KIND_MAP : KIND_SET, 0, 0));
            struct js_value entries = newarray(js);
            struct js_value items = listof(js, first);

            if (!collection.number || entries.type != JS_OBJECT)
                return undefined();

            js->objects[collection.number].data = entries.number;

            if (first->type == JS_STRING)
            {

                struct js_value empty = value(JS_STRING, internstring(js, ""));

                items = split(js, first, &empty);

            }

            for (i = 0; i < lengthof(js, &items); i++)
            {

                struct js_value item = itemat(js, &items, i);

                if (native->data == NATIVE_NEWMAP)
                {

                    struct js_value key = itemat(js, &item, 0);
                    struct js_value entry = itemat(js, &item, 1);

                    putentry(js, &entries, &key, &entry);

                }

                else
                {

                    putentry(js, &entries, &item, &item);

                }

            }

            return collection;

        }

    case NATIVE_COLLECTIONSET:
    case NATIVE_COLLECTIONGET:
    case NATIVE_COLLECTIONHAS:
    case NATIVE_COLLECTIONDELETE:
    case NATIVE_COLLECTIONCLEAR:
    case NATIVE_COLLECTIONFOREACH:
    case NATIVE_COLLECTIONKEYS:
    case NATIVE_COLLECTIONVALUES:
    case NATIVE_COLLECTIONENTRIES:
        {

            struct js_value entries;
            int found;

            if (kind != KIND_MAP && kind != KIND_SET)
                return undefined();

            entries = value(JS_OBJECT, js->objects[self->number].data);
            found = findentry(js, &entries, first);

            switch (native->data)
            {

            case NATIVE_COLLECTIONSET:
                putentry(js, &entries, first, (kind == KIND_MAP) ? second : first);

                return call->self;

            case NATIVE_COLLECTIONGET:
                return (found >= 0) ? itemat(js, &entries, found + 1) : undefined();

            case NATIVE_COLLECTIONHAS:
                return value(JS_BOOLEAN, found >= 0);

            case NATIVE_COLLECTIONDELETE:
                if (found < 0)
                    return value(JS_BOOLEAN, 0);

                for (i = found; i + 2 < lengthof(js, &entries); i++)
                {

                    struct js_value next = itemat(js, &entries, i + 2);

                    setitem(js, &entries, i, &next);

                }

                js->objects[entries.number].data -= 2;

                return value(JS_BOOLEAN, 1);

            case NATIVE_COLLECTIONCLEAR:
                js->objects[entries.number].data = 0;

                return undefined();

            case NATIVE_COLLECTIONFOREACH:
                for (i = 0; i < lengthof(js, &entries) && isfunction(js, first) && !js->throwing; i += 2)
                {

                    struct call each;
                    unsigned int argument;

                    copyvalue(&each.function, first);

                    each.self = undefined();
                    each.count = 3;
                    each.arguments[0] = itemat(js, &entries, i + 1);
                    each.arguments[1] = itemat(js, &entries, i);
                    each.arguments[2] = call->self;

                    for (argument = 3; argument < ARGUMENTS; argument++)
                        each.arguments[argument] = undefined();

                    callfunction(js, &each);

                }

                return undefined();

            case NATIVE_COLLECTIONENTRIES:
                if (kind == KIND_MAP)
                    return listof(js, self);

            /* fall through */
            default:
                {

                    struct js_value result = newarray(js);

                    for (i = (native->data == NATIVE_COLLECTIONKEYS) ? 0 : 1; i < lengthof(js, &entries); i += 2)
                    {

                        struct js_value item = itemat(js, &entries, i);

                        if (native->data == NATIVE_COLLECTIONENTRIES)
                        {

                            struct js_value pair = newarray(js);

                            pusharray(js, &pair, &item);
                            pusharray(js, &pair, &item);

                            item = pair;

                        }

                        pusharray(js, &result, &item);

                    }

                    return result;

                }

            }

        }

    case NATIVE_CALL:
    case NATIVE_APPLY:
        {

            struct call inner;

            inner.function = call->self;
            inner.self = call->arguments[0];
            inner.count = 0;

            if (native->data == NATIVE_CALL)
            {

                for (i = 1; i < call->count; i++)
                    inner.arguments[inner.count++] = call->arguments[i];

            }

            else if (second->type == JS_OBJECT)
            {

                unsigned int count = (js->objects[second->number].kind == KIND_ARRAY) ? js->objects[second->number].data : 0;

                for (i = 0; i < count && inner.count < ARGUMENTS; i++)
                    inner.arguments[inner.count++] = getproperty(js, second, numberstring(js, i));

            }

            for (i = inner.count; i < ARGUMENTS; i++)
                inner.arguments[i] = undefined();

            return callfunction(js, &inner);

        }

    case NATIVE_BIND:
        if (isfunction(js, self))
        {

            struct js_value bound = value(JS_OBJECT, newobject(js, KIND_BOUND, 0, self->number));
            struct js_value arguments = newarray(js);

            if (!bound.number)
                return undefined();

            for (i = 1; i < call->count; i++)
                pusharray(js, &arguments, &call->arguments[i]);

            putproperty(js, bound.number, internstring(js, "this"), first);
            putproperty(js, bound.number, internstring(js, "arguments"), &arguments);

            return bound;

        }

        return undefined();

    case NATIVE_HASOWNPROPERTY:
        return value(JS_BOOLEAN, self->type == JS_OBJECT && findproperty(js, self->number, internstring(js, getstring(js, tostring(js, first)))) != 0);

    case NATIVE_CREATEELEMENT:
        return elementvalue(js, html_createelement(js->document, getstring(js, tostring(js, first))));

    case NATIVE_CREATETEXTNODE:
        return nodevalue(js, html_createtext(js->document, getstring(js, tostring(js, first))));

    case NATIVE_APPEND:
    case NATIVE_PREPEND:
        if (element)
        {

            unsigned int before = (native->data == NATIVE_PREPEND) ? js->document->nodes[element].first : 0;

            for (i = 0; i < call->count; i++)
                html_insert(js->document, element, argumentnode(js, &call->arguments[i]), before);

            js->changed = 1;

        }

        return (call->count) ? call->arguments[0] : undefined();

    case NATIVE_INSERTBEFORE:
        if (element)
        {

            html_insert(js->document, element, argumentnode(js, first), nodeof(js, second));

            js->changed = 1;

        }

        return call->arguments[0];

    case NATIVE_REMOVECHILD:
        if (element && nodeof(js, first) && js->document->nodes[nodeof(js, first)].parent == element)
        {

            html_remove(js->document, nodeof(js, first));

            js->changed = 1;

        }

        return call->arguments[0];

    case NATIVE_REMOVE:
        if (element)
        {

            html_remove(js->document, element);

            js->changed = 1;

        }

        return undefined();

    case NATIVE_INSERTADJACENTHTML:
        if (element)
        {

            char *where = getstring(js, tostring(js, first));
            char *source = getstring(js, tostring(js, second));
            unsigned int parent = js->document->nodes[element].parent;
            unsigned int holder = html_createelement(js->document, "div");

            if (!holder)
                return undefined();

            html_parsefragment(js->document, holder, source, cstring_length(source));

            if (cstring_match(where, "beforebegin"))
                moveall(js, holder, parent, element);
            else if (cstring_match(where, "afterbegin"))
                moveall(js, holder, element, js->document->nodes[element].first);
            else if (cstring_match(where, "afterend"))
                moveall(js, holder, parent, js->document->nodes[element].next);
            else
                moveall(js, holder, element, 0);

            js->changed = 1;

        }

        return undefined();

    case NATIVE_SETTIMEOUT:
        if (!isfunction(js, first) || js->timercount >= JS_TIMERS)
            return value(JS_NUMBER, 0);

        js->timers[js->timercount++] = first->number;

        return value(JS_NUMBER, js->timercount);

    case NATIVE_MAX:
    case NATIVE_MIN:
        {

            int result = (call->count) ? tonumber(js, first) : 0;

            for (i = 1; i < call->count; i++)
            {

                int number = tonumber(js, &call->arguments[i]);

                if ((native->data == NATIVE_MAX) ? number > result : number < result)
                    result = number;

            }

            return value(JS_NUMBER, result);

        }

    case NATIVE_ROUND:
        return value(JS_NUMBER, tonumber(js, first));

    case NATIVE_ABS:
        return value(JS_NUMBER, (tonumber(js, first) < 0) ? -tonumber(js, first) : tonumber(js, first));

    case NATIVE_STARTSWITH:
    case NATIVE_ENDSWITH:
    case NATIVE_INCLUDES:
        if (self->type == JS_STRING)
        {

            char *haystack = getstring(js, self->number);
            char *needle = getstring(js, tostring(js, first));
            unsigned int length = cstring_length(haystack);
            unsigned int size = cstring_length(needle);

            if (size > length)
                return value(JS_BOOLEAN, 0);

            if (native->data == NATIVE_STARTSWITH)
                return value(JS_BOOLEAN, buffer_match(haystack, needle, size));

            if (native->data == NATIVE_ENDSWITH)
                return value(JS_BOOLEAN, buffer_match(haystack + length - size, needle, size));

            for (i = 0; i + size <= length; i++)
            {

                if (buffer_match(haystack + i, needle, size))
                    return value(JS_BOOLEAN, 1);

            }

            return value(JS_BOOLEAN, 0);

        }

        if (native->data == NATIVE_INCLUDES && self->type == JS_OBJECT && js->objects[self->number].kind == KIND_ARRAY)
        {

            for (i = 0; i < js->objects[self->number].data; i++)
            {

                struct js_value item = getproperty(js, self, numberstring(js, i));

                if (strictequal(js, &item, first))
                    return value(JS_BOOLEAN, 1);

            }

        }

        return value(JS_BOOLEAN, 0);

    case NATIVE_TRIM:
        if (self->type == JS_STRING)
        {

            char *text = getstring(js, self->number);
            unsigned int end = cstring_length(text);
            unsigned int start = 0;

            while (start < end && isspace(text[start]))
                start++;

            while (end > start && isspace(text[end - 1]))
                end--;

            return value(JS_STRING, addstring(js, text + start, end - start));

        }

        return undefined();

    case NATIVE_CHARAT:
        if (self->type == JS_STRING)
        {

            unsigned int index = tonumber(js, first);

            return (index < cstring_length(getstring(js, self->number))) ? value(JS_STRING, addstring(js, getstring(js, self->number) + index, 1)) : value(JS_STRING, internstring(js, ""));

        }

        return undefined();

    case NATIVE_SLICE:
        {

            unsigned int array = self->type == JS_OBJECT && js->objects[self->number].kind == KIND_ARRAY;
            int length = (array) ? (int)js->objects[self->number].data : (self->type == JS_STRING) ? (int)cstring_length(getstring(js, self->number)) : 0;
            int start = (call->count) ? tonumber(js, first) : 0;
            int end = (call->count >= 2 && second->type != JS_UNDEFINED) ? tonumber(js, second) : length;
            struct js_value result;

            start = (start < 0) ? ((start + length < 0) ? 0 : start + length) : ((start > length) ? length : start);
            end = (end < 0) ? ((end + length < 0) ? 0 : end + length) : ((end > length) ? length : end);
            end = (end < start) ? start : end;

            if (self->type == JS_STRING)
                return value(JS_STRING, addstring(js, getstring(js, self->number) + start, end - start));

            if (!array)
                return undefined();

            result = newarray(js);

            for (; start < end; start++)
            {

                struct js_value item = getproperty(js, self, numberstring(js, start));

                pusharray(js, &result, &item);

            }

            return result;

        }

    case NATIVE_SPLIT:
        if (self->type == JS_STRING && isregex(js, first))
            return regexsplit(js, self, first);

        return (self->type == JS_STRING) ? split(js, self, first) : undefined();

    case NATIVE_REPLACE:
        if (self->type == JS_STRING && isregex(js, first))
            return regexreplace(js, self, first, second);

        if (self->type == JS_STRING && first->type == JS_STRING)
        {

            char *text = getstring(js, self->number);
            char *pattern = getstring(js, first->number);
            unsigned int size = cstring_length(pattern);

            for (i = 0; i + size <= cstring_length(text); i++)
            {

                if (buffer_match(text + i, pattern, size))
                {

                    unsigned int head = addstring(js, text, i);
                    unsigned int tail = concat(js, tostring(js, second), addstring(js, getstring(js, self->number) + i + size, cstring_length(getstring(js, self->number) + i + size)));

                    return value(JS_STRING, concat(js, head, tail));

                }

            }

        }

        return (self->type == JS_STRING) ? value(JS_STRING, self->number) : undefined();

    case NATIVE_ADDEVENTLISTENER:
        if (call->count >= 2 && isfunction(js, second) && js->listenercount < JS_LISTENERS)
        {

            struct js_listener *listener = &js->listeners[js->listenercount++];

            listener->node = (kind == KIND_MEDIA) ? JS_MEDIA : element;
            listener->type = internstring(js, getstring(js, tostring(js, first)));
            listener->function = second->number;

        }

        return undefined();

    case NATIVE_REMOVEEVENTLISTENER:
        {

            unsigned int node = (kind == KIND_MEDIA) ? JS_MEDIA : element;
            unsigned int type = internstring(js, getstring(js, tostring(js, first)));

            for (i = 0; i < js->listenercount; i++)
            {

                struct js_listener *listener = &js->listeners[i];

                if (listener->node == node && listener->type == type && second->type == JS_OBJECT && listener->function == (unsigned int)second->number)
                {

                    buffer_copy(listener, &js->listeners[js->listenercount - 1], sizeof (struct js_listener));

                    js->listenercount--;

                    break;

                }

            }

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

    if (function->kind == KIND_BOUND)
    {

        struct call bound;
        struct js_value boundthis = getproperty(js, &call->function, internstring(js, "this"));
        struct js_value boundarguments = getproperty(js, &call->function, internstring(js, "arguments"));
        unsigned int count = (boundarguments.type == JS_OBJECT) ? js->objects[boundarguments.number].data : 0;

        bound.function = value(JS_OBJECT, function->data);
        bound.self = boundthis;
        bound.count = 0;

        for (i = 0; i < count && bound.count < ARGUMENTS; i++)
            bound.arguments[bound.count++] = getproperty(js, &boundarguments, numberstring(js, i));

        for (i = 0; i < call->count && bound.count < ARGUMENTS; i++)
            bound.arguments[bound.count++] = call->arguments[i];

        for (i = bound.count; i < ARGUMENTS; i++)
            bound.arguments[i] = undefined();

        return callfunction(js, &bound);

    }

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

    if (!(node->c & FUNCTION_ARROW))
        declare(js, scope, internstring(js, "this"), &call->self);

    if (node->c & FUNCTION_ARGUMENTS)
    {

        struct js_value arguments = newarray(js);

        for (i = 0; i < call->count; i++)
            pusharray(js, &arguments, &call->arguments[i]);

        declare(js, scope, internstring(js, "arguments"), &arguments);

        i = 0;

    }

    if (node->c & FUNCTION_DERIVED)
    {

        struct call parent;

        buffer_copy(&parent, call, sizeof (struct call));

        parent.function = lookup(js, function->parent, internstring(js, "super"));

        if (!js->throwing)
            callfunction(js, &parent);

    }

    for (parameter = node->a; parameter; parameter = js->nodes[parameter].next)
    {

        struct js_value argument = undefined();

        if (i < call->count)
            copyvalue(&argument, &call->arguments[i]);

        if (js->nodes[parameter].op)
        {

            unsigned int rest;

            argument = newarray(js);

            for (rest = i; rest < call->count; rest++)
                pusharray(js, &argument, &call->arguments[rest]);

        }

        if (argument.type == JS_UNDEFINED && js->nodes[parameter].a)
            argument = evaluate(js, js->nodes[parameter].a, scope);

        if (js->nodes[parameter].type == NODE_DECLARATOR)
            bindpattern(js, scope, parameter, &argument);
        else
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

        struct js_value evaluated = evaluate(js, (js->nodes[argument].type == NODE_SPREAD) ? js->nodes[argument].a : argument, scope);
        unsigned int i;

        if (js->nodes[argument].type != NODE_SPREAD)
        {

            if (call->count < ARGUMENTS)
                copyvalue(&call->arguments[call->count++], &evaluated);

            continue;

        }

        evaluated = listof(js, &evaluated);

        for (i = 0; i < lengthof(js, &evaluated) && call->count < ARGUMENTS; i++)
            call->arguments[call->count++] = itemat(js, &evaluated, i);

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

static unsigned int instanceof(struct js_engine *js, struct js_value *object, struct js_value *constructor)
{

    struct js_value prototype;
    unsigned int current;

    if (object->type != JS_OBJECT || !isfunction(js, constructor))
        return 0;

    if (constructor->number == (int)nativefor(js, NATIVE_ARRAY))
        return js->objects[object->number].kind == KIND_ARRAY;

    if (constructor->number == (int)nativefor(js, NATIVE_ELEMENT))
        return js->objects[object->number].kind == KIND_ELEMENT;

    prototype = getproperty(js, constructor, internstring(js, "prototype"));

    if (prototype.type != JS_OBJECT)
        return 0;

    for (current = js->objects[object->number].parent; current && js->objects[object->number].kind == KIND_PLAIN; current = js->objects[current].parent)
    {

        if (current == (unsigned int)prototype.number)
            return 1;

        if (js->objects[current].kind != KIND_PLAIN)
            break;

    }

    return 0;

}

static struct js_value combine(struct js_engine *js, unsigned int op, struct js_value *a, struct js_value *b)
{

    switch (op)
    {

    case OP_ADD:
        if (a->type == JS_STRING || b->type == JS_STRING || a->type == JS_OBJECT || b->type == JS_OBJECT)
            return value(JS_STRING, concat(js, tostring(js, a), tostring(js, b)));

        return value(JS_NUMBER, tonumber(js, a) + tonumber(js, b));

    case OP_INSTANCEOF:
        return value(JS_BOOLEAN, instanceof(js, a, b));

    case OP_IN:
        if (b->type != JS_OBJECT)
        {

            throwerror(js, "Cannot use in on a non-object");

            return undefined();

        }

        return value(JS_BOOLEAN, getproperty(js, b, internstring(js, getstring(js, tostring(js, a)))).type != JS_UNDEFINED);

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

static struct js_value accessorvalue(struct js_engine *js, unsigned int accessor, struct js_value *function)
{

    /* setters are not kept, so assigning makes an own property that hides the getter */
    if (accessor != ACCESSOR_GET || function->type != JS_OBJECT)
        return undefined();

    return value(JS_OBJECT, newobject(js, KIND_GETTER, 0, function->number));

}

static struct js_value construct(struct js_engine *js, struct call *call)
{

    struct js_value prototype = getproperty(js, &call->function, internstring(js, "prototype"));
    struct js_value result;

    if (!isfunction(js, &call->function))
    {

        throwerror(js, "Value is not a constructor");

        return undefined();

    }

    call->self = value(JS_OBJECT, newobject(js, KIND_PLAIN, (prototype.type == JS_OBJECT) ? (unsigned int)prototype.number : 0, 0));

    if (!call->self.number)
    {

        throwerror(js, "Out of objects");

        return undefined();

    }

    result = callfunction(js, call);

    return (result.type == JS_OBJECT) ? result : call->self;

}

static struct js_value evaluatecall(struct js_engine *js, struct js_node *node, unsigned int scope)
{

    struct js_node *callee = &js->nodes[node->a];
    struct call call;

    call.self = value(JS_OBJECT, js->global);

    if (node->op)
    {

        call.function = evaluate(js, node->a, scope);

    }

    else if (callee->type == NODE_MEMBER && js->nodes[callee->a].type == NODE_NAME && cstring_match(getstring(js, js->nodes[callee->a].value), "super"))
    {

        struct js_value parent = lookup(js, scope, js->nodes[callee->a].value);
        struct js_value prototype = getproperty(js, &parent, internstring(js, "prototype"));

        call.self = lookup(js, scope, internstring(js, "this"));
        call.function = getproperty(js, &prototype, callee->value);

    }

    else if (callee->type == NODE_MEMBER || callee->type == NODE_INDEX)
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

        if (callee->type == NODE_NAME && cstring_match(getstring(js, callee->value), "super"))
            call.self = lookup(js, scope, internstring(js, "this"));

    }

    if (js->throwing)
        return undefined();

    collectarguments(js, &call, node->b, scope);

    if (js->throwing)
        return undefined();

    return (node->op) ? construct(js, &call) : callfunction(js, &call);

}

static struct js_value evaluateclass(struct js_engine *js, struct js_node *node, unsigned int scope)
{

    struct js_value parent = (node->c) ? evaluate(js, node->c, scope) : undefined();
    struct js_value parentprototype = undefined();
    struct js_value constructor;
    struct js_value prototype;
    unsigned int classscope;
    unsigned int member;

    if (js->throwing)
        return undefined();

    if (parent.type == JS_OBJECT)
        parentprototype = getproperty(js, &parent, internstring(js, "prototype"));

    classscope = newobject(js, KIND_SCOPE, scope, 0);
    prototype = value(JS_OBJECT, newobject(js, KIND_PLAIN, (parentprototype.type == JS_OBJECT) ? (unsigned int)parentprototype.number : 0, 0));

    if (!classscope || !prototype.number)
    {

        throwerror(js, "Out of objects");

        return undefined();

    }

    if (node->c)
        declare(js, classscope, internstring(js, "super"), &parent);

    constructor = newfunction(js, node->a, classscope);

    if (js->throwing)
        return undefined();

    putproperty(js, prototype.number, internstring(js, "constructor"), &constructor);
    putproperty(js, constructor.number, internstring(js, "prototype"), &prototype);

    /* statics are inherited by copying them when the class is made */
    if (parent.type == JS_OBJECT)
    {

        unsigned int property;

        for (property = js->objects[parent.number].property; property; property = js->properties[property].next)
        {

            if (!findproperty(js, constructor.number, js->properties[property].name))
                putproperty(js, constructor.number, js->properties[property].name, &js->properties[property].value);

        }

    }

    for (member = node->b; member && !js->throwing; member = js->nodes[member].next)
    {

        struct js_value method = newfunction(js, js->nodes[member].a, classscope);

        if (js->nodes[member].op == ACCESSOR_SET)
            continue;

        if (js->nodes[member].op)
            method = accessorvalue(js, js->nodes[member].op, &method);

        putproperty(js, (js->nodes[member].c) ? (unsigned int)constructor.number : (unsigned int)prototype.number, js->nodes[member].value, &method);

    }

    return constructor;

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

            unsigned int i;

            a = evaluate(js, (js->nodes[child].type == NODE_SPREAD) ? js->nodes[child].a : child, scope);
            b = value(JS_OBJECT, object);

            if (js->nodes[child].type != NODE_SPREAD)
            {

                setproperty(js, &b, numberstring(js, count++), &a);

                continue;

            }

            a = listof(js, &a);

            for (i = 0; i < lengthof(js, &a); i++)
            {

                struct js_value item = itemat(js, &a, i);

                setproperty(js, &b, numberstring(js, count++), &item);

            }

        }

        return (object) ? value(JS_OBJECT, object) : undefined();

    case NODE_OBJECT:
        object = newobject(js, KIND_PLAIN, 0, 0);

        for (child = node->a; child && object && !js->throwing; child = js->nodes[child].next)
        {

            unsigned int name = js->nodes[child].value;

            a = evaluate(js, js->nodes[child].a, scope);

            if (js->nodes[child].c == 1)
            {

                unsigned int property;

                for (property = (a.type == JS_OBJECT) ? js->objects[a.number].property : 0; property; property = js->properties[property].next)
                {

                    if (!findproperty(js, object, js->properties[property].name))
                        putproperty(js, object, js->properties[property].name, &js->properties[property].value);

                }

                continue;

            }

            if (js->nodes[child].c == 2)
            {

                b = evaluate(js, js->nodes[child].b, scope);
                name = internstring(js, getstring(js, tostring(js, &b)));

            }

            if (js->nodes[child].op)
                a = accessorvalue(js, js->nodes[child].op, &a);

            if (js->nodes[child].op != ACCESSOR_SET)
                putproperty(js, object, name, &a);

        }

        return (object) ? value(JS_OBJECT, object) : undefined();

    case NODE_MEMBER:
    case NODE_INDEX:
        a = evaluate(js, node->a, scope);
        child = propertyname(js, index, scope);

        return (js->throwing) ? undefined() : getproperty(js, &a, child);

    case NODE_CALL:
        return evaluatecall(js, node, scope);

    case NODE_CLASS:
        return evaluateclass(js, node, scope);

    case NODE_REGEX:
        object = newobject(js, KIND_REGEX, 0, node->value);

        return (object) ? value(JS_OBJECT, object) : undefined();

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

    case NODE_SEQUENCE:
        evaluate(js, node->a, scope);

        return (js->throwing) ? undefined() : evaluate(js, node->b, scope);

    }

    return undefined();

}

static struct js_value restof(struct js_engine *js, struct js_node *pattern, struct js_value *source, unsigned int index)
{

    struct js_value result;
    struct js_value keys;
    unsigned int i;

    if (pattern->c == PATTERN_ARRAY)
    {

        result = newarray(js);

        for (; index < lengthof(js, source); index++)
        {

            struct js_value item = itemat(js, source, index);

            pusharray(js, &result, &item);

        }

        return result;

    }

    result = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

    if (!result.number || source->type != JS_OBJECT)
        return result;

    keys = keysof(js, source->number, 0);

    for (i = 0; i < lengthof(js, &keys); i++)
    {

        struct js_value key = itemat(js, &keys, i);
        struct js_property *property = findproperty(js, source->number, key.number);
        unsigned int name;
        unsigned int listed = 0;

        for (name = pattern->b; name; name = js->nodes[name].next)
        {

            struct js_node *element = &js->nodes[name];
            unsigned int key = (element->type == NODE_DECLARATOR) ? (unsigned int)element->value : (element->b) ? element->b : (unsigned int)element->value;

            if (!element->op && key == property->name)
                listed = 1;

        }

        if (!listed)
            putproperty(js, result.number, property->name, &property->value);

    }

    return result;

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

    if (source->type <= JS_NULL)
    {

        throwerror(js, "Cannot destructure undefined or null");

        return;

    }

    for (name = node->b; name && !js->throwing; name = js->nodes[name].next, index++)
    {

        struct js_node *element = &js->nodes[name];
        unsigned int nested = element->type == NODE_DECLARATOR;
        unsigned int key = (nested) ? (unsigned int)element->value : (element->b) ? element->b : (unsigned int)element->value;
        struct js_value item;

        if (!nested && !element->value)
            continue;

        if (element->op)
            item = restof(js, node, source, index);
        else
            item = getproperty(js, source, (node->c == PATTERN_ARRAY) ? numberstring(js, index) : key);

        if (item.type == JS_UNDEFINED && element->a)
            item = evaluate(js, element->a, scope);

        if (nested)
            bindpattern(js, scope, name, &item);
        else
            declare(js, scope, element->value, &item);

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

    if (node->c == LOOP_OF)
        source = listof(js, &source);

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

static void addmethod(struct js_engine *js, char *owner, char *name, unsigned int native)
{

    struct js_value method = value(JS_OBJECT, js->natives[native]);
    struct js_property *property = findproperty(js, js->global, internstring(js, owner));

    if (property && property->value.type == JS_OBJECT && property->value.number)
        putproperty(js, property->value.number, internstring(js, name), &method);

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
    js->timercount = 0;

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
    addglobal(js, "setTimeout", js->natives[NATIVE_SETTIMEOUT]);
    addglobal(js, "requestAnimationFrame", js->natives[NATIVE_SETTIMEOUT]);
    addglobal(js, "Math", newobject(js, KIND_PLAIN, 0, 0));
    addmethod(js, "Math", "max", NATIVE_MAX);
    addmethod(js, "Math", "min", NATIVE_MIN);
    addmethod(js, "Math", "round", NATIVE_ROUND);
    addmethod(js, "Math", "floor", NATIVE_ROUND);
    addmethod(js, "Math", "ceil", NATIVE_ROUND);
    addmethod(js, "Math", "trunc", NATIVE_ROUND);
    addmethod(js, "Math", "abs", NATIVE_ABS);
    /* numbers are integers, so random is always 0: Math.floor(Math.random() * n) stays a valid index */
    addmethod(js, "Math", "random", NATIVE_ZERO);
    addmethod(js, "Math", "sqrt", NATIVE_SQRT);
    addmethod(js, "Math", "pow", NATIVE_POW);
    addglobal(js, "setInterval", js->natives[NATIVE_SETTIMEOUT]);
    addglobal(js, "clearTimeout", js->natives[NATIVE_CLEARTIMER]);
    addglobal(js, "clearInterval", js->natives[NATIVE_CLEARTIMER]);
    addglobal(js, "cancelAnimationFrame", js->natives[NATIVE_CLEARTIMER]);
    addglobal(js, "parseInt", js->natives[NATIVE_PARSEINT]);
    addglobal(js, "parseFloat", js->natives[NATIVE_PARSEINT]);
    addglobal(js, "isNaN", js->natives[NATIVE_ISNAN]);
    addglobal(js, "Number", js->natives[NATIVE_NUMBER]);
    addglobal(js, "String", js->natives[NATIVE_STRING]);
    addglobal(js, "Boolean", js->natives[NATIVE_BOOLEAN]);
    addglobal(js, "Error", js->natives[NATIVE_ERROR]);
    addglobal(js, "TypeError", js->natives[NATIVE_ERROR]);
    addglobal(js, "Array", js->natives[NATIVE_ARRAY]);
    addglobal(js, "Object", js->natives[NATIVE_OBJECT]);
    addglobal(js, "Date", js->natives[NATIVE_DATE]);
    addglobal(js, "RegExp", js->natives[NATIVE_REGEXP]);
    addglobal(js, "DOMParser", js->natives[NATIVE_DOMPARSER]);
    addglobal(js, "Map", js->natives[NATIVE_NEWMAP]);
    addglobal(js, "Symbol", js->natives[NATIVE_SYMBOL]);
    addglobal(js, "Element", js->natives[NATIVE_ELEMENT]);
    addglobal(js, "Event", js->natives[NATIVE_EVENT]);
    addglobal(js, "CustomEvent", js->natives[NATIVE_EVENT]);
    addglobal(js, "getComputedStyle", js->natives[NATIVE_COMPUTEDSTYLE]);
    addglobal(js, "HTMLElement", js->natives[NATIVE_ELEMENT]);
    addglobal(js, "Node", js->natives[NATIVE_ELEMENT]);
    addglobal(js, "Set", js->natives[NATIVE_NEWSET]);
    addglobal(js, "WeakMap", js->natives[NATIVE_NEWMAP]);
    addglobal(js, "WeakSet", js->natives[NATIVE_NEWSET]);
    addglobal(js, "navigator", newobject(js, KIND_PLAIN, 0, 0));
    addglobal(js, "location", newobject(js, KIND_PLAIN, 0, 0));
    addglobal(js, "Function", js->natives[NATIVE_FUNCTION]);
    addglobal(js, "NodeList", js->natives[NATIVE_ARRAY]);
    addglobal(js, "HTMLCollection", js->natives[NATIVE_ARRAY]);
    addglobal(js, "JSON", newobject(js, KIND_PLAIN, 0, 0));
    addmethod(js, "String", "fromCharCode", NATIVE_FROMCHARCODE);
    addmethod(js, "Array", "isArray", NATIVE_ISARRAY);
    addmethod(js, "Array", "from", NATIVE_ARRAYFROM);
    addmethod(js, "Object", "keys", NATIVE_KEYS);
    addmethod(js, "Object", "values", NATIVE_VALUES);
    addmethod(js, "Object", "entries", NATIVE_ENTRIES);
    addmethod(js, "Object", "assign", NATIVE_ASSIGN);
    addmethod(js, "Object", "create", NATIVE_CREATE);
    addmethod(js, "Object", "freeze", NATIVE_IDENTITY);
    addmethod(js, "Object", "defineProperty", NATIVE_DEFINEPROPERTY);
    addmethod(js, "Date", "now", NATIVE_ZERO);
    addmethod(js, "JSON", "stringify", NATIVE_STRINGIFY);
    addmethod(js, "JSON", "parse", NATIVE_PARSE);

    {

        struct js_value prototype = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

        putproperty(js, js->natives[NATIVE_ARRAY], internstring(js, "prototype"), &prototype);

        for (i = 0; i < ARRAYMETHODS; i++)
            addnative(js, prototype.number, arraymethods[i].name, arraymethods[i].native);

        prototype = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

        putproperty(js, js->natives[NATIVE_FUNCTION], internstring(js, "prototype"), &prototype);
        addnative(js, prototype.number, "toString", NATIVE_OBJECTTOSTRING);
        addnative(js, prototype.number, "call", NATIVE_CALL);
        addnative(js, prototype.number, "apply", NATIVE_APPLY);
        addnative(js, prototype.number, "bind", NATIVE_BIND);

        for (i = 0; i < SYMBOLS; i++)
        {

            struct js_value symbol = value(JS_STRING, concat(js, internstring(js, "@@"), internstring(js, symbols[i])));

            putproperty(js, js->natives[NATIVE_SYMBOL], internstring(js, symbols[i]), &symbol);

        }

        for (i = 0; i < LOCATIONFIELDS; i++)
        {

            struct js_value empty = value(JS_STRING, internstring(js, ""));
            struct js_property *location = findproperty(js, js->global, internstring(js, "location"));

            putproperty(js, location->value.number, internstring(js, locationfields[i]), &empty);

        }

        prototype = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

        putproperty(js, js->natives[NATIVE_ELEMENT], internstring(js, "prototype"), &prototype);
        addnative(js, prototype.number, "querySelector", NATIVE_QUERYSELECTOR);
        addnative(js, prototype.number, "querySelectorAll", NATIVE_QUERYSELECTORALL);
        addnative(js, prototype.number, "matches", NATIVE_MATCHES);
        addnative(js, prototype.number, "closest", NATIVE_CLOSEST);

        prototype = value(JS_OBJECT, newobject(js, KIND_PLAIN, 0, 0));

        putproperty(js, js->natives[NATIVE_OBJECT], internstring(js, "prototype"), &prototype);
        addnative(js, prototype.number, "toString", NATIVE_OBJECTTOSTRING);
        addnative(js, prototype.number, "hasOwnProperty", NATIVE_HASOWNPROPERTY);

        js->date = newobject(js, KIND_PLAIN, 0, 0);

        for (i = 0; i < DATEFIELDS; i++)
            addnative(js, js->date, datefields[i], NATIVE_ZERO);

        addnative(js, js->date, "getFullYear", NATIVE_FULLYEAR);

    }

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
    parser.token.type = TOKEN_END;
    parser.more = 0;
    parser.noin = 0;
    parser.arguments = 0;

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

static unsigned int dispatch(struct js_engine *js, unsigned int node, unsigned int target, unsigned int name)
{

    unsigned int count = js->listenercount;
    unsigned int handled = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct js_listener *listener = &js->listeners[i];
        struct call call;
        struct js_value eventtype = value(JS_STRING, name);
        struct js_value eventtarget = (target && target != JS_MEDIA) ? nodevalue(js, target) : value(JS_OBJECT, js->documentobject);
        unsigned int event;
        unsigned int argument;

        if (listener->node != node || listener->type != name)
            continue;

        event = newobject(js, KIND_PLAIN, 0, 0);
        call.function = value(JS_OBJECT, listener->function);
        call.self = (node && node != JS_MEDIA) ? nodevalue(js, node) : value(JS_OBJECT, js->documentobject);

        if (event)
        {

            putproperty(js, event, internstring(js, "type"), &eventtype);
            putproperty(js, event, internstring(js, "target"), &eventtarget);
            putproperty(js, event, internstring(js, "currentTarget"), &call.self);
            addnative(js, event, "preventDefault", NATIVE_NOOP);
            addnative(js, event, "stopPropagation", NATIVE_NOOP);
            addnative(js, event, "stopImmediatePropagation", NATIVE_NOOP);

        }

        call.count = 1;
        call.arguments[0] = (event) ? value(JS_OBJECT, event) : undefined();

        for (argument = 1; argument < ARGUMENTS; argument++)
            call.arguments[argument] = undefined();

        js->throwing = 0;

        callfunction(js, &call);

        /* a listener that throws is counted, as a browser would report it, and the rest still run */
        if (js->throwing)
            js->errors++;

        js->throwing = 0;
        handled++;

    }

    return handled;

}

static unsigned int bubble(struct js_engine *js, unsigned int target, unsigned int name)
{

    unsigned int handled = 0;
    unsigned int node;

    for (node = target; node; node = js->document->nodes[node].parent)
        handled += dispatch(js, node, target, name);

    /* the document listens last */
    return handled + dispatch(js, 0, target, name);

}

void js_dispatch(struct js_engine *js, unsigned int node, char *type)
{

    js->steps = 0;

    dispatch(js, node, node, internstring(js, type));

}

unsigned int js_click(struct js_engine *js, unsigned int node)
{

    js->steps = 0;

    return bubble(js, node, internstring(js, "click"));

}

void js_runtimers(struct js_engine *js)
{

    unsigned int i;

    /* timers set by timers run too; the table never shrinks here, so this ends */
    for (i = 0; i < js->timercount; i++)
    {

        struct call call;
        unsigned int argument;

        if (!js->timers[i])
            continue;

        call.function = value(JS_OBJECT, js->timers[i]);
        call.self = undefined();
        call.count = 0;

        for (argument = 0; argument < ARGUMENTS; argument++)
            call.arguments[argument] = undefined();

        js->steps = 0;
        js->throwing = 0;

        callfunction(js, &call);

        if (js->throwing)
            js->errors++;

        js->throwing = 0;

    }

    js->timercount = 0;

}

