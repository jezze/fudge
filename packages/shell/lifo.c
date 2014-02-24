#include <fudge.h>
#include "lifo.h"

unsigned int lifo_stack_push(struct lifo_stack *stack, unsigned int count, void *data)
{

    if (stack->head + count < stack->size)
    {

        memory_copy(stack->buffer + stack->head, data, count);

        stack->head += count;

        return count;

    }

    return 0;

}

unsigned int lifo_stack_pop(struct lifo_stack *stack, unsigned int count)
{

    if (stack->head >= count)
    {

        stack->head -= count;

        return count;

    }

    return 0;

}

void lifo_stack_clear(struct lifo_stack *stack)
{

    stack->head = 0;

}

void lifo_stack_init(struct lifo_stack *stack, unsigned int size, void *buffer)
{

    memory_clear(stack, sizeof (struct lifo_stack));

    stack->size = size;
    stack->buffer = buffer;

}

