#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int open_path(unsigned int index, unsigned int count, char *buffer)
{

    return memory_match(buffer, "/", 1) ? call_open(index, CALL_DR, count - 1, buffer + 1) : call_open(index, CALL_DW, count, buffer);

}

static unsigned int open_pipe(unsigned int index, unsigned int number)
{

    char path[FUDGE_BSIZE];
    unsigned int offset = 0;

    offset += memory_write(path, FUDGE_BSIZE, "temp/", 5, offset);
    offset += memory_write_number(path, FUDGE_BSIZE, number, 10, offset);

    return call_open(index, CALL_DR, offset, path);

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int pindex;
    unsigned int cindex;

    call_open(CALL_D1, CALL_DI, 0, 0);
    call_open(CALL_D2, CALL_DO, 0, 0);
    call_open(CALL_D3, CALL_DC, 0, 0);
    call_open(CALL_D4, CALL_DW, 0, 0);

    for (pindex = 0; pindex < expression->count; pindex++)
    {

        struct pipe *pipe = &expression->pipes[pindex];

        for (cindex = 0; cindex < pipe->count; cindex++)
        {

            struct command *command = &pipe->commands[cindex];

            if (command->in0.path.count)
                open_path(CALL_DI, command->in0.path.count, state->buffer + command->in0.path.index);

            if (command->in0.data.count)
                call_write(CALL_DI, 0, command->in0.data.count, state->buffer + command->in0.data.index);

            if (command->in1.path.count)
                open_path(CALL_DC, command->in1.path.count, state->buffer + command->in1.path.index);

            if (command->in1.data.count)
                call_write(CALL_DC, 0, command->in1.data.count, state->buffer + command->in1.data.index);

            if (command->out0.path.count)
                open_path(CALL_DO, command->out0.path.count, state->buffer + command->out0.path.index);

            if (!command->out0.path.count && pipe->count > 1 && cindex != pipe->count - 1)
                open_pipe(CALL_DO, cindex);

            call_open(CALL_DW, CALL_DR, 4, "bin/");
            open_path(CALL_D0, command->binary.count, state->buffer + command->binary.index);
            call_open(CALL_DW, CALL_D4, 0, 0);
            call_spawn(CALL_D0);
            call_close(CALL_D0);
            call_open(CALL_DI, CALL_DO, 0, 0);
            call_open(CALL_DO, CALL_D2, 0, 0);

        }

        call_open(CALL_DI, CALL_D1, 0, 0);
        call_open(CALL_DO, CALL_D2, 0, 0);
        call_open(CALL_DC, CALL_D3, 0, 0);
        call_open(CALL_DW, CALL_D4, 0, 0);

    }

    call_close(CALL_D1);
    call_close(CALL_D2);
    call_close(CALL_D3);
    call_close(CALL_D4);

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_DI, 0, FUDGE_BSIZE, buffer), buffer);

    if (parse(&state, &expression))
        execute(&state, &expression);

}

