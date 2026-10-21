unsigned int port_init(char *path);
void port_destroy(void);
void glue_exit(int status) __attribute__((noreturn));
void glue_present(SDL_Surface *surface);
