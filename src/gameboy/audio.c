#include <fudge.h>
#include "audio.h"

#define AUDIO_NSAMPLES (AUDIO_SAMPLES * 2)
#define PHASEONE 0x10000
#define PHASERATE 45812984
#define COUNTERONE 0x400000
#define COUNTERINC(n, d) (((unsigned int)(n) << 22) / (AUDIO_RATE * (d)))
#define LEVELONE 0x10000
#define AUDIO_MEM_SIZE (0xFF3F - 0xFF10 + 1)
#define AUDIO_ADDR_COMPENSATION 0xFF10
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

static unsigned char audio_mem[AUDIO_MEM_SIZE];

struct chan_len_ctr
{

    unsigned load : 6;
    unsigned enabled : 1;
    int counter;
    int inc;

};

struct chan_vol_env
{

    unsigned step : 3;
    unsigned up : 1;
    int counter;
    int inc;

};

struct chan_freq_sweep
{

    unsigned int freq;
    unsigned rate : 3;
    unsigned up : 1;
    unsigned shift : 3;
    int counter;
    int inc;

};

static struct chan
{

    unsigned enabled : 1;
    unsigned powered : 1;
    unsigned on_left : 1;
    unsigned on_right : 1;
    unsigned muted : 1;
    unsigned volume : 4;
    unsigned volume_init : 4;
    unsigned short freq;
    int freq_counter;
    int freq_inc;
    int val;
    struct chan_len_ctr len;
    struct chan_vol_env env;
    struct chan_freq_sweep sweep;
    unsigned char duty;
    unsigned char duty_counter;
    unsigned short lfsr_reg;
    unsigned char lfsr_wide;
    int lfsr_div;
    unsigned char sample;
    int capacitor;

} chans[4];

static int mix[AUDIO_NSAMPLES];
static int vol_l, vol_r;

static int hipass(struct chan *c, int sample)
{

    int out = sample - c->capacitor;

    c->capacitor = sample - (out - out / 250);

    return out;

}

static int squareinc(unsigned int freq)
{

    return PHASERATE / ((2048 - freq) << 5);

}

static int weigh(int length, int inc, int value)
{

    return (length * ((1 << 30) / inc) >> 14) * value;

}

static void chan_enable(const unsigned char i, const unsigned char enable)
{

    unsigned char val;

    chans[i].enabled = enable;
    val = (audio_mem[0xFF26 - AUDIO_ADDR_COMPENSATION] & 0x80) | (chans[3].enabled << 3) | (chans[2].enabled << 2) | (chans[1].enabled << 1) | (chans[0].enabled << 0);
    audio_mem[0xFF26 - AUDIO_ADDR_COMPENSATION] = val;

}

static void update_env(struct chan *c)
{

    c->env.counter += c->env.inc;

    while (c->env.counter > COUNTERONE)
    {

        if (c->env.step)
        {

            c->volume += c->env.up ? 1 : -1;

            if (c->volume == 0 || c->volume == 15)
                c->env.inc = 0;

            c->volume = MAX(0, MIN(15, c->volume));

        }

        c->env.counter -= COUNTERONE;

    }

}

static void update_len(struct chan *c)
{

    if (c->len.enabled)
    {

        c->len.counter += c->len.inc;

        if (c->len.counter > COUNTERONE)
        {

            chan_enable(c - chans, 0);

            c->len.counter = 0;

        }

    }

}

static int update_freq(struct chan *c, int pos)
{

    c->freq_counter += c->freq_inc - pos;

    if (c->freq_counter > PHASEONE)
    {

        pos = c->freq_inc - (c->freq_counter - PHASEONE);
        c->freq_counter = 0;

        return pos;

    }

    return -1;

}

static void update_sweep(struct chan *c)
{

    c->sweep.counter += c->sweep.inc;

    while (c->sweep.counter > COUNTERONE)
    {

        if (c->sweep.shift)
        {

            unsigned short inc = (c->sweep.freq >> c->sweep.shift);

            if (!c->sweep.up)
                inc *= -1;

            c->freq += inc;

            if (c->freq > 2047)
            {

                c->enabled = 0;

            }

            else
            {

                c->freq_inc = squareinc(c->freq);

            }

        }

        else if (c->sweep.rate)
        {

            c->enabled = 0;

        }

        c->sweep.counter -= COUNTERONE;

    }

}

static void update_square(const unsigned char ch2)
{

    struct chan *c = chans + ch2;
    unsigned int i;

    if (!c->powered)
        return;

    c->freq_inc = squareinc(c->freq);

    for (i = 0; i < AUDIO_NSAMPLES; i += 2)
    {

        update_len(c);

        if (c->enabled)
        {

            int pos = 0;
            int prev_pos = 0;
            int sample = 0;

            update_env(c);

            if (!ch2)
                update_sweep(c);

            while ((pos = update_freq(c, pos)) >= 0)
            {

                c->duty_counter = (c->duty_counter + 1) & 7;

                sample += weigh(pos - prev_pos, c->freq_inc, c->val);
                c->val = (c->duty & (1 << c->duty_counter)) ? 1 : -1;
                prev_pos = pos;

            }

            sample += weigh(c->freq_inc - prev_pos, c->freq_inc, c->val);
            sample = hipass(c, sample * (int)c->volume / 15);

            if (!c->muted)
            {

                mix[i + 0] += sample * (int)c->on_left * vol_l / 28;
                mix[i + 1] += sample * (int)c->on_right * vol_r / 28;

            }

        }

    }

}

static unsigned char wave_sample(const unsigned int pos, const unsigned int volume)
{

    unsigned char sample = audio_mem[(0xFF30 + pos / 2) - AUDIO_ADDR_COMPENSATION];

    if (pos & 1)
        sample &= 0xF;
    else
        sample >>= 4;

    return volume ? (sample >> (volume - 1)) : 0;

}

static void update_wave(void)
{

    struct chan *c = chans + 2;
    unsigned int i;

    if (!c->powered)
        return;

    c->freq_inc = PHASERATE * 2 / ((2048 - c->freq) << 5);

    for (i = 0; i < AUDIO_NSAMPLES; i += 2)
    {

        update_len(c);

        if (c->enabled)
        {

            int pos = 0;
            int prev_pos = 0;
            int sample = 0;

            c->sample = wave_sample(c->val, c->volume);

            while ((pos = update_freq(c, pos)) >= 0)
            {

                c->val = (c->val + 1) & 31;
                sample += weigh(pos - prev_pos, c->freq_inc, c->sample);
                c->sample = wave_sample(c->val, c->volume);
                prev_pos = pos;

            }

            sample += weigh(c->freq_inc - prev_pos, c->freq_inc, c->sample);

            if (c->volume > 0)
            {

                static const int list[] = { LEVELONE * 15 / 2, LEVELONE * 15 / 4, LEVELONE * 3 / 2 };
                int diff = list[c->volume - 1];

                sample = hipass(c, (sample - diff) * 2 / 15);

                if (!c->muted)
                {

                    mix[i + 0] += sample * (int)c->on_left * vol_l / 28;
                    mix[i + 1] += sample * (int)c->on_right * vol_r / 28;

                }

            }

        }

    }

}

static void update_noise(void)
{

    static const unsigned char rates[] = { 8, 16, 32, 48, 64, 80, 96, 112 };
    struct chan *c = chans + 3;
    unsigned int i;

    if (!c->powered)
        return;

    c->freq_inc = PHASERATE / 8 / (rates[c->lfsr_div] << c->freq);

    if (c->freq >= 14)
        c->enabled = 0;

    for (i = 0; i < AUDIO_NSAMPLES; i += 2)
    {

        update_len(c);

        if (c->enabled)
        {

            int pos = 0;
            int prev_pos = 0;
            int sample = 0;

            update_env(c);

            while ((pos = update_freq(c, pos)) >= 0)
            {

                c->lfsr_reg = (c->lfsr_reg << 1) | (c->val == 1);

                if (c->lfsr_wide) {
                    c->val = !(((c->lfsr_reg >> 14) & 1) ^ ((c->lfsr_reg >> 13) & 1)) ? 1 : -1;
                } else {
                    c->val = !(((c->lfsr_reg >> 6) & 1) ^ ((c->lfsr_reg >> 5) & 1)) ? 1 : -1;
                }

                sample += weigh(pos - prev_pos, c->freq_inc, c->val);
                prev_pos = pos;

            }

            sample += weigh(c->freq_inc - prev_pos, c->freq_inc, c->val);
            sample = hipass(c, sample * (int)c->volume / 15);

            if (!c->muted)
            {

                mix[i + 0] += sample * (int)c->on_left * vol_l / 28;
                mix[i + 1] += sample * (int)c->on_right * vol_r / 28;

            }

        }

    }

}

void audio_render(short *out)
{

    unsigned int i;

    buffer_clear(mix, sizeof (mix));
    update_square(0);
    update_square(1);
    update_wave();
    update_noise();

    for (i = 0; i < AUDIO_NSAMPLES; i++)
    {

        int sample = mix[i];

        if (sample > LEVELONE)
            sample = LEVELONE;

        if (sample < -LEVELONE)
            sample = -LEVELONE;

        out[i] = sample * 32767 / LEVELONE;

    }

}

static void chan_trigger(unsigned char i)
{

    struct chan *c = chans + i;
    int len_max = 64;

    chan_enable(i, 1);
    c->volume = c->volume_init;

    {

        unsigned char val = audio_mem[(0xFF12 + (i * 5)) - AUDIO_ADDR_COMPENSATION];

        c->env.step = val & 0x07;
        c->env.up = val & 0x08 ? 1 : 0;
        c->env.inc = c->env.step ? COUNTERINC(64, c->env.step) : COUNTERINC(8, 1);
        c->env.counter = 0;

    }

    if (i == 0)
    {

        unsigned char val = audio_mem[0xFF10 - AUDIO_ADDR_COMPENSATION];

        c->sweep.freq = c->freq;
        c->sweep.rate = (val >> 4) & 0x07;
        c->sweep.up = !(val & 0x08);
        c->sweep.shift = (val & 0x07);
        c->sweep.inc = c->sweep.rate ? COUNTERINC(128, c->sweep.rate) : 0;
        c->sweep.counter = COUNTERONE + 1;

    }

    if (i == 2)
    {

        len_max = 256;
        c->val = 0;

    }

    else if (i == 3)
    {

        c->lfsr_reg = 0xFFFF;
        c->val = -1;

    }

    c->len.inc = COUNTERINC(256, len_max - c->len.load);
    c->len.counter = 0;

}

unsigned char audio_read(const unsigned short addr)
{

    static unsigned char ortab[] = {
        0x80, 0x3f, 0x00, 0xff,
        0xbf, 0xff, 0x3f, 0x00,
        0xff, 0xbf, 0x7f, 0xff,
        0x9f, 0xff, 0xbf, 0xff,
        0xff, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x70
    };

    if (addr > 0xFF26)
        return audio_mem[addr - AUDIO_ADDR_COMPENSATION];

    return audio_mem[addr - AUDIO_ADDR_COMPENSATION] | ortab[addr - 0xFF10];

}

void audio_write(const unsigned short addr, const unsigned char val)
{

    unsigned char i = (addr - 0xFF10) / 5;

    audio_mem[addr - AUDIO_ADDR_COMPENSATION] = val;

    switch (addr)
    {

    case 0xFF12:
    case 0xFF17:
    case 0xFF21:
        chans[i].volume_init = val >> 4;
        chans[i].powered = (val >> 3) != 0;

        if (chans[i].powered && chans[i].enabled)
        {

            if ((chans[i].env.step == 0 && chans[i].env.inc != 0))
            {

                if (val & 0x08)
                    chans[i].volume++;
                else
                    chans[i].volume += 2;

            }

            else
            {

                chans[i].volume = 16 - chans[i].volume;

            }

            chans[i].volume &= 0x0F;
            chans[i].env.step = val & 0x07;

        }

        break;

    case 0xFF1C:
        chans[i].volume = chans[i].volume_init = (val >> 5) & 0x03;

        break;

    case 0xFF11:
    case 0xFF16:
    case 0xFF20:
        {

            const unsigned char duty_lookup[] = { 0x10, 0x30, 0x3C, 0xCF };

            chans[i].len.load = val & 0x3f;
            chans[i].duty = duty_lookup[val >> 6];

        }

        break;

    case 0xFF1B:
        chans[i].len.load = val;

        break;

    case 0xFF13:
    case 0xFF18:
    case 0xFF1D:
        chans[i].freq &= 0xFF00;
        chans[i].freq |= val;

        break;

    case 0xFF1A:
        chans[i].powered = (val & 0x80) != 0;
        chan_enable(i, val & 0x80);

        break;

    case 0xFF14:
    case 0xFF19:
    case 0xFF1E:
        chans[i].freq &= 0x00FF;
        chans[i].freq |= ((val & 0x07) << 8);
    case 0xFF23:
        chans[i].len.enabled = val & 0x40 ? 1 : 0;

        if (val & 0x80)
            chan_trigger(i);

        break;

    case 0xFF22:
        chans[3].freq = val >> 4;
        chans[3].lfsr_wide = !(val & 0x08);
        chans[3].lfsr_div = val & 0x07;

        break;

    case 0xFF24:
        vol_l = (val >> 4) & 0x07;
        vol_r = val & 0x07;

        break;

    case 0xFF25:
        {

            unsigned char j;

            for (j = 0; j < 4; ++j)
            {

                chans[j].on_left  = (val >> (4 + j)) & 1;
                chans[j].on_right = (val >> j) & 1;

            }

        }

        break;

    }

}

void audio_init(void)
{

    const unsigned char regs_init[] = {
        0x80, 0xBF, 0xF3, 0xFF, 0x3F,
        0xFF, 0x3F, 0x00, 0xFF, 0x3F,
        0x7F, 0xFF, 0x9F, 0xFF, 0x3F,
        0xFF, 0xFF, 0x00, 0x00, 0x3F,
        0x77, 0xF3, 0xF1
    };

    const unsigned char wave_init[] = {
        0xac, 0xdd, 0xda, 0x48,
        0x36, 0x02, 0xcf, 0x16,
        0x2c, 0x04, 0xe5, 0x2c,
        0xac, 0xdd, 0xda, 0x48
    };
    unsigned char i;

    buffer_clear(chans, sizeof (chans));

    chans[0].val = chans[1].val = -1;

    for (i = 0; i < sizeof (regs_init); ++i)
        audio_write(0xFF10 + i, regs_init[i]);

    for (i = 0; i < sizeof (wave_init); ++i)
        audio_write(0xFF30 + i, wave_init[i]);

}

