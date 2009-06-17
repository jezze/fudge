#include <system.h>
#include <screen.h>
#include <keyboard.h>
#include <console.h>

void console_init()
{

	screen_clear();

	puts("Welcome to Fudge! Type help for more information.\n\n");
	puts("fudge:/$ ");

	while(1)
	{

		if (keyboard_buffer_size())
		{

			uint8_t c = keyboard_buffer_read();

			putc(c);

			if (c == '\n')
			{

				puts("fudge:/$ ");

			}


		}

	}

	for(;;);

}
