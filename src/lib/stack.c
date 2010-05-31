#include <lib/types.h>
#include <lib/stack.h>

stack_t stack_create(char *buffer, uint32_t size)
{

    stack_t stack;
    stack.buffer = buffer;
    stack.size = size;
    stack.head = 0;

    return stack;

}

void stack_push(stack_t *stack, char c)
{

    if (stack->head < stack->size)
    {

        stack->buffer[stack->head] = c;
        stack->head++;

    }

}

char stack_pop(stack_t *stack)
{

    if (stack->head > 0)
    {

        char c = stack->buffer[stack->head - 1];
        stack->head--;

        return c;

    }

    return 0;

}

void stack_clear(stack_t *stack)
{

    stack->head = 0;

}

