#define JS_NODES                        16384
#define JS_OBJECTS                      4096
#define JS_PROPERTIES                   16384
#define JS_STRINGSIZE                   0x20000
#define JS_INTERNS                      4096
#define JS_LISTENERS                    64
#define JS_NATIVES                      24
#define JS_MEDIA                        HTML_NODES
#define JS_UNDEFINED                    0
#define JS_NULL                         1
#define JS_BOOLEAN                      2
#define JS_NUMBER                       3
#define JS_STRING                       4
#define JS_OBJECT                       5

struct js_value
{

    unsigned int type;
    int number;

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
    unsigned int nodecount;
    unsigned int objectcount;
    unsigned int propertycount;
    unsigned int stringcount;
    unsigned int interncount;
    unsigned int listenercount;
    unsigned int global;
    unsigned int documentobject;
    unsigned int storage;
    unsigned int dark;
    unsigned int steps;
    unsigned int depth;
    unsigned int throwing;
    struct js_value thrown;
    unsigned int changed;
    unsigned int errors;

};

void js_reset(struct js_engine *js, struct html_document *document, unsigned int dark);
void js_run(struct js_engine *js, char *source, unsigned int count);
unsigned int js_haslistener(struct js_engine *js, unsigned int node, char *type);
void js_dispatch(struct js_engine *js, unsigned int node, char *type);
