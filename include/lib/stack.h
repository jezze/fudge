#ifndef STACK_H
#define STACK_H

typedef struct stack
{

    char *buffer;
    unsigned int size;
    unsigned int head;

} stack_t;

extern stack_t stack_create(char *buffer, unsigned int size);
extern void stack_push(stack_t *stack, char c);
extern char stack_pop(stack_t *stack);
extern void stack_clear(stack_t *stack);

#endif

