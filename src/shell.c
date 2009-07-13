#include <system.h>
#include <screen.h>
#include <kbd.h>
#include <shell.h>

char shellBuffer[SHELL_BUFFER_SIZE];
uint16_t shellBufferSize = 0;

void shell_buffer_clear()
{

	puts("fudge:/$ ");
	shellBufferSize = 0;

}

void shell_buffer_push(int8_t c)
{

	shellBuffer[shellBufferSize] = c;

	if (shellBufferSize < SHELL_BUFFER_SIZE)
		shellBufferSize++;


}

void shell_buffer_pop()
{

	if (shellBufferSize)
		shellBufferSize--;

}

char *shell_buffer_read()
{

	shell_buffer_push('\0');

	return shellBuffer;

}

uint16_t shell_buffer_size()
{

	return shellBufferSize;

}

void shell_init()
{

	screen_clear();

	puts("Fudge\n");
	puts("Copyright (c) 2009 Jens Nyberg\n\n");
	puts("Type `help` for a list of commands.\n\n");

	shell_buffer_clear();

	while(1)
	{

		if (kbd_buffer_size())
		{

			char c = kbd_buffer_read();

			if (c == '\b')
			{

				if (shell_buffer_size())
				{

					putc('\b');
					putc(' ');
					putc('\b');

					shell_buffer_pop();

				}

			}

			else if (c == '\n')
			{

				putc('\n');

				char *command = shell_buffer_read();

				if (strcmp(command, "") == 0)
				{

				}

				else if (strcmp(command, "help") == 0)
				{

					puts("Command list\n\n");
					puts("help - Show this dialog\n");
					puts("clear - Clear screen\n");

				}

				else if (strcmp(command, "clear") == 0)
				{

					screen_clear();

				}

				else
				{

					puts("Command not found\n");

				}

				shell_buffer_clear();

			}

			else
			{

				putc(c);
				shell_buffer_push(c);

			}

		}

	}

}
