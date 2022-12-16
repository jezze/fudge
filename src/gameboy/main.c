#include "audio.h"
#include "video.h"
#include "cpu.h"

static unsigned char *rom;
static unsigned char *cart_ram;

static unsigned char gb_rom_read(struct gb_s *gb, const unsigned int addr)
{

    return rom[addr];

}

static unsigned char gb_cart_ram_read(struct gb_s *gb, const unsigned int addr)
{

    return cart_ram[addr];

}

static void gb_cart_ram_write(struct gb_s *gb, const unsigned int addr, const unsigned char val)
{

    cart_ram[addr] = val;

}

static unsigned int getsavesize(struct gb_s *gb)
{

    const unsigned int ram_sizes[] = { 0x00, 0x800, 0x2000, 0x8000, 0x20000 };
    unsigned char ram_size = gb->gb_rom_read(gb, 0x0149);

    return ram_sizes[ram_size];

}

static const char *getromname(struct gb_s* gb, char title_str[static 16])
{

    uint_fast16_t title_loc = 0x134;
    const uint_fast16_t title_end = 0x143;
    const char *title_start = title_str;

    for (; title_loc <= title_end; title_loc++)
    {

        const char title_char = gb->gb_rom_read(gb, title_loc);

        if (title_char >= ' ' && title_char <= '_')
        {

            *title_str = title_char;
            title_str++;

        }

        else
            break;

    }

    *title_str = '\0';

    return title_start;

}

static unsigned char *read_rom_to_ram(const char *filename)
{

    FILE *rom_file = fopen(filename, "rb");
    size_t rom_size;
    unsigned char *rom = NULL;

    if (rom_file == NULL)
        return NULL;

    fseek(rom_file, 0, SEEK_END);
    rom_size = ftell(rom_file);
    rewind(rom_file);
    rom = malloc(rom_size);

    if (fread(rom, sizeof(unsigned char), rom_size, rom_file) != rom_size)
    {

        free(rom);
        fclose(rom_file);

        return NULL;

    }

    fclose(rom_file);

    return rom;

}

static void read_cart_ram_file(const char *filename, const size_t len)
{

    FILE *f;

    if (!len)
        return;

    cart_ram = malloc(len);

    if (cart_ram == NULL)
    {

        exit(EXIT_FAILURE);

    }

    f = fopen(filename, "rb");

    if (f == NULL)
    {

        memset(cart_ram, 0xFF, len);

        return;

    }

    fread(cart_ram, sizeof(unsigned char), len, f);
    fclose(f);

}

static void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const unsigned short val)
{

    switch (gb_err)
    {

    case GB_INVALID_OPCODE:
        fprintf(stdout, "Invalid opcode %#04x at PC: %#06x, SP: %#06x\n", val, gb->cpu_reg.pc - 1, gb->cpu_reg.sp);

        break;

    case GB_INVALID_WRITE:
    case GB_INVALID_READ:
        return;

    default:
        printf("Unknown error");

        break;
    }

    fprintf(stderr, "Error. Press q to exit, or any other key to continue.");

    if (getchar() == 'q')
    {

        free(rom);
        free(cart_ram);
        exit(EXIT_FAILURE);

    }

    return;

}

int main(int argc, char **argv)
{

    struct gb_s gb;
    const double target_speed_ms = 1000.0 / VERTICAL_SYNC;
    double speed_compensation = 0.0;
    unsigned int running = 1;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
    unsigned int new_ticks, old_ticks;
    enum gb_init_error_e gb_ret;
    unsigned int fast_mode = 1;
    unsigned int fast_mode_timer = 1;
    char *romfile = NULL;
    int ret = EXIT_SUCCESS;
    time_t rawtime;
    struct tm *timeinfo;

    switch (argc)
    {

    case 2:
        romfile = argv[1];

        break;

    default:
        exit(EXIT_FAILURE);

    }

    if ((rom = read_rom_to_ram(romfile)) == NULL)
    {

        ret = EXIT_FAILURE;

        goto out;

    }

    gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error);

    switch (gb_ret)
    {

    case GB_INIT_NO_ERROR:
        break;

    case GB_INIT_CARTRIDGE_UNSUPPORTED:
        puts("Unsupported cartridge.");
        ret = EXIT_FAILURE;
        goto out;

    case GB_INIT_INVALID_CHECKSUM:
        puts("Invalid ROM: Checksum failure.");
        ret = EXIT_FAILURE;
        goto out;

    default:
        printf("Unknown error: %d\n", gb_ret);
        ret = EXIT_FAILURE;
        goto out;
    }

    read_cart_ram_file(romfile, getsavesize(&gb));
    time(&rawtime);

    timeinfo = localtime(&rawtime);

    gb_set_rtc(&gb, timeinfo);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {

        printf("Could not initialise SDL: %s\n", SDL_GetError());
        ret = EXIT_FAILURE;

        goto out;

    }

#if ENABLE_AUDIO
    SDL_AudioDeviceID dev;
    SDL_AudioSpec want, have;

    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_F32SYS,
    want.channels = 2;
    want.samples = AUDIO_SAMPLES;
    want.callback = audio_callback;
    want.userdata = NULL;

    printf("Audio driver: %s\n", SDL_GetAudioDeviceName(0, 0));

    if ((dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) == 0)
    {

        printf("SDL could not open audio device: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);

    }

    SDL_PauseAudioDevice(dev, 0);
#endif

#if ENABLE_AUDIO
    audio_init();
#endif

    {

        char title_str[28] = "Peanut-SDL: ";

        printf("ROM: %s\n", getromname(&gb, title_str + 12));
        printf("MBC: %d\n", gb.mbc);

        window = SDL_CreateWindow(title_str, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LCD_WIDTH * 2, LCD_HEIGHT * 2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);

        if (window == NULL)
        {

            printf("Could not create window: %s\n", SDL_GetError());
            ret = EXIT_FAILURE;

            goto out;

        }

    }

    SDL_SetWindowMinimumSize(window, LCD_WIDTH, LCD_HEIGHT);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    if (renderer == NULL)
    {

        printf("Could not create renderer: %s\n", SDL_GetError());
        ret = EXIT_FAILURE;

        goto out;

    }

    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0)
    {

        printf("Renderer could not draw color: %s\n", SDL_GetError());
        ret = EXIT_FAILURE;

        goto out;

    }

    if (SDL_RenderClear(renderer) < 0)
    {

        printf("Renderer could not clear: %s\n", SDL_GetError());
        ret = EXIT_FAILURE;

        goto out;

    }

    SDL_RenderPresent(renderer);
    SDL_RenderSetLogicalSize(renderer, LCD_WIDTH, LCD_HEIGHT);
    SDL_RenderSetIntegerScale(renderer, 1);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);

    if (texture == NULL)
    {

        printf("Texture could not be created: %s\n", SDL_GetError());
        ret = EXIT_FAILURE;

        goto out;

    }

#if ENABLE_VIDEO
    video_setpalette();
#endif

    while (running)
    {

        static unsigned int rtc_timer = 0;
        int delay;

        old_ticks = SDL_GetTicks();

        while (SDL_PollEvent(&event))
        {

            static int fullscreen = 0;

            switch (event.type)
            {

            case SDL_QUIT:
                running = 0;

                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {

                case SDLK_RETURN:
                    gb.direct.joypad_bits.start = 0;

                    break;

                case SDLK_BACKSPACE:
                    gb.direct.joypad_bits.select = 0;

                    break;

                case SDLK_z:
                    gb.direct.joypad_bits.a = 0;

                    break;

                case SDLK_x:
                    gb.direct.joypad_bits.b = 0;

                    break;

                case SDLK_UP:
                    gb.direct.joypad_bits.up = 0;

                    break;

                case SDLK_RIGHT:
                    gb.direct.joypad_bits.right = 0;

                    break;

                case SDLK_DOWN:
                    gb.direct.joypad_bits.down = 0;

                    break;

                case SDLK_LEFT:
                    gb.direct.joypad_bits.left = 0;

                    break;

                case SDLK_SPACE:
                    fast_mode = 2;

                    break;

                case SDLK_1:
                    fast_mode = 1;

                    break;

                case SDLK_2:
                    fast_mode = 2;

                    break;

                case SDLK_3:
                    fast_mode = 3;

                    break;

                case SDLK_4:
                    fast_mode = 4;

                    break;

                case SDLK_r:
                    gb_reset(&gb);

                    break;

#if ENABLE_VIDEO
                case SDLK_i:
                    gb.direct.interlace = ~gb.direct.interlace;

                    break;

                case SDLK_o:
                    gb.direct.frame_skip = ~gb.direct.frame_skip;

                    break;
#endif

                }

                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {

                case SDLK_RETURN:
                    gb.direct.joypad_bits.start = 1;

                    break;

                case SDLK_BACKSPACE:
                    gb.direct.joypad_bits.select = 1;

                    break;

                case SDLK_z:
                    gb.direct.joypad_bits.a = 1;

                    break;

                case SDLK_x:
                    gb.direct.joypad_bits.b = 1;

                    break;

                case SDLK_UP:
                    gb.direct.joypad_bits.up = 1;

                    break;

                case SDLK_RIGHT:
                    gb.direct.joypad_bits.right = 1;

                    break;

                case SDLK_DOWN:
                    gb.direct.joypad_bits.down = 1;

                    break;

                case SDLK_LEFT:
                    gb.direct.joypad_bits.left = 1;

                    break;

                case SDLK_SPACE:
                    fast_mode = 1;

                    break;

                case SDLK_f:
                    if (fullscreen)
                    {

                        SDL_SetWindowFullscreen(window, 0);

                        fullscreen = 0;

                        SDL_ShowCursor(SDL_ENABLE);

                    }

                    else
                    {

                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

                        fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;

                        SDL_ShowCursor(SDL_DISABLE);

                    }
                    
                    break;

                case SDLK_F11:
                    if (fullscreen)
                    {

                        SDL_SetWindowFullscreen(window, 0);

                        fullscreen = 0;

                        SDL_ShowCursor(SDL_ENABLE);

                    }

                    else
                    {

                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

                        fullscreen = SDL_WINDOW_FULLSCREEN;

                        SDL_ShowCursor(SDL_DISABLE);

                    }

                    break;

                }

                break;

            }

        }

        gb_run_frame(&gb);

        rtc_timer += target_speed_ms / fast_mode;

        if (rtc_timer >= 1000)
        {

            rtc_timer -= 1000;

            gb_tick_rtc(&gb);

        }

        if (fast_mode_timer > 1)
        {

            fast_mode_timer--;

            continue;

        }

        fast_mode_timer = fast_mode;

#if ENABLE_VIDEO
        SDL_UpdateTexture(texture, NULL, video_getfb(), LCD_WIDTH * sizeof(unsigned short));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
#endif

        new_ticks = SDL_GetTicks();

        speed_compensation += target_speed_ms - (new_ticks - old_ticks);
        delay = (int)speed_compensation;
        speed_compensation -= delay;

        if (delay > 0)
        {

            unsigned int delay_ticks = SDL_GetTicks();
            unsigned int after_delay_ticks;

            rtc_timer += delay;

            if (rtc_timer >= 1000)
            {

                rtc_timer -= 1000;
                gb_tick_rtc(&gb);

            }

            SDL_Delay(delay);

            after_delay_ticks = SDL_GetTicks();
            speed_compensation += (double)delay - (int)(after_delay_ticks - delay_ticks);

        }
        
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
    SDL_Quit();

out:
    free(rom);
    free(cart_ram);

    return ret;

}

