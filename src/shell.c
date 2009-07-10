#include <system.h>
#include <screen.h>
#include <kbd.h>
#include <shell.h>

int8_t shellBuffer[SHELL_BUFFER_SIZE];
uint16_t shellBufferSize = 0;

void shell_buffer_write(int8_t c)
{

	if (shellBufferSize < SHELL_BUFFER_SIZE)
	{

		shellBuffer[shellBufferSize] = c;
		shellBufferSize++;

	}

}

int8_t *shell_buffer_read()
{

	shellBuffer[shellBufferSize - 1] = '\0';

	return shellBuffer;

}

void shell_buffer_clear()
{

	puts("fudge:/$ ");
	shellBufferSize = 0;

}

void shell_init()
{

	screen_clear();

	puts("Welcome to Fudge! Type help for more information.\n\n");

	shell_buffer_clear();

	while(1)
	{

		if (kbd_buffer_size())
		{

			uint8_t c = kbd_buffer_read();

			shell_buffer_write(c);
			putc(c);

			if (c == '\n')
			{

				int8_t *command = shell_buffer_read();

				if (strcmp(command, "help") == 0)
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

		}

	}

}
