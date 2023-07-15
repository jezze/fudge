#define DEBUG_FMT0(l, s)                    debug_fmt0(l, s, __FILE__, __LINE__)
#define DEBUG_FMT1(l, s, a1)                debug_fmt1(l, s, a1, __FILE__, __LINE__)
#define DEBUG_FMT2(l, s, a1, a2)            debug_fmt2(l, s, a1, a2, __FILE__, __LINE__)
#define DEBUG_FMT3(l, s, a1, a2, a3)        debug_fmt3(l, s, a1, a2, a3, __FILE__, __LINE__)
#define DEBUG_FMT4(l, s, a1, a2, a3, a4)    debug_fmt4(l, s, a1, a2, a3, a4, __FILE__, __LINE__)
#define DEBUG_ASSERT(l, t)                  debug_assert(l, t, __FILE__, __LINE__)
#define DEBUG_CRITICAL                  1
#define DEBUG_ERROR                     2
#define DEBUG_WARNING                   3
#define DEBUG_INFO                      4

struct debug_interface
{

    struct resource resource;
    void (*write)(unsigned int level, unsigned int count, char *string, char *file, unsigned int line);

};

void debug_fmt0(unsigned int level, char *fmt, char *file, unsigned int line);
void debug_fmt1(unsigned int level, char *fmt, void *arg1, char *file, unsigned int line);
void debug_fmt2(unsigned int level, char *fmt, void *arg1, void *arg2, char *file, unsigned int line);
void debug_fmt3(unsigned int level, char *fmt, void *arg1, void *arg2, void *arg3, char *file, unsigned int line);
void debug_fmt4(unsigned int level, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, char *file, unsigned int line);
void debug_assert(unsigned int level, unsigned int test, char *file, unsigned int line);
void debug_registerinterface(struct debug_interface *interface);
void debug_unregisterinterface(struct debug_interface *interface);
void debug_initinterface(struct debug_interface *interface, void (*write)(unsigned int level, unsigned int count, char *string, char *file, unsigned int line));
