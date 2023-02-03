#define DEBUG_LOG(l, s)                 debug_log(l, s, __FILE__, __LINE__)
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

void debug_log(unsigned int level, char *string, char *file, unsigned int line);
void debug_assert(unsigned int level, unsigned int test, char *file, unsigned int line);
void debug_registerinterface(struct debug_interface *interface);
void debug_unregisterinterface(struct debug_interface *interface);
void debug_initinterface(struct debug_interface *interface, void (*write)(unsigned int level, char *string, char *file, unsigned int line));
