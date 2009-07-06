#include <system.h>
#include <screen.h>
#include <kbd.h>
#include <shell.h>

void shell_init()
{

	screen_clear();

	puts("Welcome to Fudge! Type help for more information.\n\n");
	puts("fudge:/$ ");

	while(1)
	{

		if (kbd_buffer_size())
		{

			uint8_t c = kbd_buffer_read();

			putc(c);

			if (c == '\n')
			{

				puts("fudge:/$ ");

			}


		}

	}

}
