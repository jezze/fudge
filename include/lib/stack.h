#ifndef LIB_STACK_H
#define LIB_STACK_H

struct stack
{

    char *buffer;
    unsigned int size;
    unsigned int head;

};

extern void stack_init(struct stack *stack, char *buffer, unsigned int size);
extern void stack_push(struct stack *stack, char c);
extern char stack_pop(struct stack *stack);
extern void stack_clear(struct stack *stack);

#endif

