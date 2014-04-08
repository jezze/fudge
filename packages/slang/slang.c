#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int walk_path(unsigned int index, unsigned int indexw, unsigned int count, char *buffer)
{

    if (memory_match(buffer, "/", 1))
        return call_walk(index, CALL_DR, count - 1, buffer + 1);

    return call_walk(index, indexw, count, buffer);

}

static void execute_command(struct command *command, char *buffer)
{

    unsigned int i;

    if (!walk_path(CALL_L1, CALL_L0, command->binary.count, buffer + command->binary.index))
        return;

    call_open(CALL_L1);

    for (i = 0; i < command->ins; i++)
    {

        unsigned int index = CALL_I1 + i * 2;

        if (command->in[i].index.count)
            index = CALL_I1 + ascii_read_value(buffer + command->in[i].index.index, command->in[i].index.count, 10) * 2;

        if (command->in[i].path.count)
        {

            walk_path(index, CALL_DW, command->in[i].path.count, buffer + command->in[i].path.index);
            call_open(index);

        }

    }

    for (i = 0; i < command->outs; i++)
    {

        unsigned int index = CALL_O1 + i * 2;

        if (command->out[i].index.count)
            index = CALL_O1 + ascii_read_value(buffer + command->out[i].index.index, command->out[i].index.count, 10) * 2;

        if (command->out[i].path.count)
        {

            walk_path(index, CALL_DW, command->out[i].path.count, buffer + command->out[i].path.index);
            call_open(index);

        }

    }

    call_spawn(CALL_L1);

    for (i = 0; i < command->ins; i++)
    {

        unsigned int index = CALL_I1 + i * 2;

        if (command->in[i].index.count)
            index = CALL_I1 + ascii_read_value(buffer + command->in[i].index.index, command->in[i].index.count, 10) * 2;

        if (command->in[i].path.count)
            call_close(index);

    }

    for (i = 0; i < command->outs; i++)
    {

        unsigned int index = CALL_O1 + i * 2;

        if (command->out[i].index.count)
            index = CALL_O1 + ascii_read_value(buffer + command->out[i].index.index, command->out[i].index.count, 10) * 2;

        if (command->out[i].path.count)
            call_close(index);

    }

    call_close(CALL_L1);

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int pindex;

    for (pindex = 0; pindex < expression->pipes; pindex++)
    {

        struct pipe *pipe = &expression->pipe[pindex];
        unsigned int cindex;

        for (cindex = 0; cindex < pipe->commands; cindex++)
        {

            if (pipe->commands > 1)
            {

                if (cindex == 0)
                {

                    call_walk(CALL_I1, CALL_I0, 0, 0);
                    call_walk(CALL_O1, CALL_DR, 15, "system/pipe/0/0");

                }

                else
                {

                    call_walk(CALL_I1, CALL_DR, 15, "system/pipe/0/1");
                    call_walk(CALL_O1, CALL_O0, 0, 0);

                }

            }

            else
            {

                call_walk(CALL_I1, CALL_I0, 0, 0);
                call_walk(CALL_O1, CALL_O0, 0, 0);

            }

            call_open(CALL_I1);
            call_open(CALL_O1);
            execute_command(&pipe->command[cindex], state->buffer);
            call_close(CALL_O1);
            call_close(CALL_I1);

        }

    }

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    if (!call_walk(CALL_L0, CALL_DR, 4, "bin/"))
        return;

    call_open(CALL_L0);

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_I0, 0, FUDGE_BSIZE, buffer), buffer);

    if (parse(&state, &expression))
        execute(&state, &expression);

    call_close(CALL_L0);

}

