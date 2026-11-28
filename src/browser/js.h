#define JS_NODES                        131072
#define JS_OBJECTS                      32768
#define JS_PROPERTIES                   65536
#define JS_STRINGSIZE                   0x80000
#define JS_REALS                        4096
#define JS_INTERNS                      65536
#define JS_LISTENERS                    64
#define JS_NATIVES                      320
#define JS_TIMERS                       32
#define JS_SCRATCH                      8192
#define JS_TEXTSIZE                     4096
#define JS_REGEXNODES                   128
#define JS_CAPTURES                     10
#define JS_REGEXNEST                    4
#define JS_MEDIA                        HTML_NODES
#define JS_UNDEFINED                    0
#define JS_NULL                         1
#define JS_BOOLEAN                      2
#define JS_NUMBER                       3
#define JS_STRING                       4
#define JS_OBJECT                       5

struct js_rect
{

    int x;
    int y;
    int w;
    int h;

};

struct js_value
{

    unsigned int type;
    int number;
    double real;

};

struct js_node
{

    unsigned short type;
    unsigned short op;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int next;
    int value;

};

struct js_object
{

    unsigned int kind;
    unsigned int property;
    unsigned int parent;
    unsigned int data;

};

struct js_property
{

    unsigned int name;
    unsigned int next;
    struct js_value value;

};

struct js_listener
{

    unsigned int node;
    unsigned int type;
    unsigned int function;

};

struct js_renode
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

struct js_capture
{

    int start;
    int end;

};

struct js_regex
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
    struct js_renode nodes[JS_REGEXNODES];
    unsigned int nodecount;
    struct js_capture captures[JS_CAPTURES];

};

struct js_engine
{

    struct html_document *document;
    struct js_node nodes[JS_NODES];
    struct js_object objects[JS_OBJECTS];
    struct js_property properties[JS_PROPERTIES];
    char strings[JS_STRINGSIZE];
    unsigned int interns[JS_INTERNS];
    struct js_listener listeners[JS_LISTENERS];
    unsigned short elements[HTML_NODES];
    unsigned int natives[JS_NATIVES];
    unsigned int timers[JS_TIMERS];
    struct js_value scratch[JS_SCRATCH];
    unsigned int scratchbusy;
    struct js_regex regexes[JS_REGEXNEST];
    unsigned int regexcount;
    char text[JS_TEXTSIZE];
    unsigned int timercount;
    unsigned int nodecount;
    unsigned int objectcount;
    unsigned int propertycount;
    unsigned int stringcount;
    unsigned int interncount;
    unsigned int listenercount;
    unsigned int global;
    unsigned int documentobject;
    unsigned int storage;
    unsigned int date;
    unsigned int dark;
    unsigned int steps;
    unsigned int depth;
    unsigned int generator;
    unsigned int generatorstop;
    unsigned int seed;
    unsigned int label;
    double now;
    double reals[JS_REALS];
    unsigned int realcount;
    double ticks;
    struct js_rect (*measure)(unsigned int node);
    unsigned int constructing;
    unsigned int throwing;
    struct js_value thrown;
    unsigned int changed;
    unsigned int errors;

};

void js_reset(struct js_engine *js, struct html_document *document, unsigned int dark, unsigned int unixtime);
void js_setview(struct js_engine *js, unsigned int width, unsigned int height, struct js_rect (*measure)(unsigned int node));
void js_run(struct js_engine *js, char *source, unsigned int count);
void js_dispatch(struct js_engine *js, unsigned int node, char *type);
unsigned int js_click(struct js_engine *js, unsigned int node);
void js_runtimers(struct js_engine *js);
