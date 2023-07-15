#define DEBUG_FMT0(l, s)                debug_fmt0(l, s, __FILE__, __LINE__)
#define DEBUG_FMT1(l, s, a1)            debug_fmt1(l, s, a1, __FILE__, __LINE__)
#define DEBUG_FMT2(l, s, a1, a2)        debug_fmt2(l, s, a1, a2, __FILE__, __LINE__)
#define DEBUG_ASSERT(l, t)              debug_assert(l, t, __FILE__, __LINE__)
#define DEBUG_CRITICAL                  1
#define DEBUG_ERROR                     2
#define DEBUG_WARNING                   3
#define DEBUG_INFO                      4

struct debug_interface
{

    struct resource resource;
    void (*write)(unsigned int level, char *string, char *file, unsigned int line);

};

void debug_fmt0(unsigned int level, char *string, char *file, unsigned int line);
void debug_fmt1(unsigned int level, char *string, void *arg1, char *file, unsigned int line);
void debug_fmt2(unsigned int level, char *string, void *arg1, void *arg2, char *file, unsigned int line);
void debug_assert(unsigned int level, unsigned int test, char *file, unsigned int line);
void debug_registerinterface(struct debug_interface *interface);
void debug_unregisterinterface(struct debug_interface *interface);
void debug_initinterface(struct debug_interface *interface, void (*write)(unsigned int level, char *string, char *file, unsigned int line));
