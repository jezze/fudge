#define DEBUG_ASSERT(l, t)                  debug_assert(l, t, __FILE__, __LINE__)
#define DEBUG_NONE                      0
#define DEBUG_CRITICAL                  1
#define DEBUG_ERROR                     2
#define DEBUG_WARNING                   3
#define DEBUG_INFO                      4
#define DEBUG_MESSAGESIZE               256

struct debug_interface
{

    struct resource resource;
    void (*write)(unsigned int level, unsigned int count, char *string, char *file, unsigned int line);

};

void debug_fmt(unsigned int level, char *file, unsigned int line, char *fmt, ...);
void debug_fmtv(unsigned int level, char *file, unsigned int line, char *fmt, void **args);
void debug_assert(unsigned int level, unsigned int test, char *file, unsigned int line);
void debug_registerinterface(struct debug_interface *interface);
void debug_unregisterinterface(struct debug_interface *interface);
void debug_initinterface(struct debug_interface *interface, void (*write)(unsigned int level, unsigned int count, char *string, char *file, unsigned int line));
