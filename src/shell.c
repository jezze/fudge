#include <system.h>
#include <screen.h>
#include <kbd.h>
#include <shell.h>

uint8_t shellBuffer[256];
uint16_t shellBufferSize = 0;

void shell_buffer_write(uint8_t c)
{

	if (shellBufferSize < 256)
	{

		shellBuffer[shellBufferSize] = c;
		shellBufferSize++;

	}

}

uint8_t *shell_buffer_read()
{

	shellBuffer[shellBufferSize] = '\0';

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

				uint8_t *command = shell_buffer_read();

				if (strcmp(command, "help"))
				{

					puts("Help is not available!\n");

				}

				else
				{

					puts("Command not recognized");

				}

				shell_buffer_clear();

			}

		}

	}

}
