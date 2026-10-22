#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "doomtype.h"
#include "doomstat.h"
#include "sounds.h"
#include "s_sound.h"
#include "w_wad.h"
#include "memio.h"
#include "mus2mid.h"
#include "lprintf.h"
#include "i_sound.h"
#include "musicplayer.h"
#include "oplplayer.h"
#include "glue.h"

#define CHANNELS                        32
#define STEPS                           256
#define CHUNKFRAMES                     1024
#define AHEADMS                         200
#define LATEMS                          300
#define RATES                           3

struct channel
{

    int id;
    unsigned int step;
    unsigned int stepremainder;
    unsigned int samplerate;
    const unsigned char *data;
    const unsigned char *enddata;
    int leftvol;
    int rightvol;

};

int snd_pcspeaker;
int snd_card = 1;
int mus_card = 1;
int snd_samplerate = 22050;
const char *snd_soundfont;
const char *snd_mididev;
const char *snd_midiplayer;
const char *midiplayers[midi_player_last + 1] = {"sdl", "fluidsynth", "opl2", "portmidi", NULL};
int mus_fluidsynth_chorus;
int mus_fluidsynth_reverb;
int mus_fluidsynth_gain;
int mus_opl_gain;
int record_sound;
int record_sound_start;
int record_sound_paused;
int post_record_sound;
const char *record_soundcommand;
char *recorddata;
int recordlen;
int recordisplaying;
int record_remove_tempfiles;
static struct channel channels[CHANNELS];
static int steptable[STEPS];
static unsigned int audio;
static unsigned int rate;
static unsigned int started;
static unsigned int sentframes;
static short chunk[CHUNKFRAMES * 2];
static music_player_t *music = &opl_synth_player.music;
static unsigned int musicready;
static const void *song;
static void *songdata;

static void stopchannel(int i)
{

    channels[i].data = NULL;

}

static void updateparams(int handle, int volume, int separation, int pitch)
{

    struct channel *channel = &channels[handle];

    if (pitched_sounds)
        channel->step = steptable[pitch] + (((channel->samplerate << 16) / rate) - 65536);
    else
        channel->step = (channel->samplerate << 16) / rate;

    separation += 1;
    channel->leftvol = volume - ((volume * separation * separation) >> 16);
    separation -= 257;
    channel->rightvol = volume - ((volume * separation * separation) >> 16);

}

static short clamp(int value)
{

    return (value > SHRT_MAX) ? SHRT_MAX : (value < SHRT_MIN) ? SHRT_MIN : value;

}

/* the music is rendered first and the effects mixed on top, as upstream's sdl callback does */
static void mix(short *out, unsigned int frames)
{

    unsigned int i;

    memset(out, 0, frames * 4);

    if (song)
        music->render(music, out, frames);

    for (i = 0; i < frames; i++)
    {

        int left = out[i * 2];
        int right = out[i * 2 + 1];
        int c;

        for (c = 0; c < numChannels && c < CHANNELS; c++)
        {

            struct channel *channel = &channels[c];
            int s;

            if (!channel->data)
                continue;

            s = ((unsigned int)channel->data[0] * (0x10000 - channel->stepremainder)) + ((unsigned int)channel->data[1] * channel->stepremainder) - 0x800000;
            left += channel->leftvol * s / 49152;
            right += channel->rightvol * s / 49152;
            channel->stepremainder += channel->step;
            channel->data += channel->stepremainder >> 16;
            channel->stepremainder &= 0xFFFF;

            if (channel->data >= channel->enddata)
                stopchannel(c);

        }

        out[i * 2] = clamp(left);
        out[i * 2 + 1] = clamp(right);

    }

}

/* the clock decides how much is due, kept 200 ms ahead, under what the drivers queue (16 or 32 messages of 1024 frames); after a stall of more than 300 ms the gap is skipped rather than sent at once */
void I_UpdateSound(void)
{

    unsigned int now;
    unsigned int due;

    if (!audio)
        return;

    now = SDL_GetTicks();

    if (!started)
    {

        started = now;
        sentframes = 0;

    }

    due = (unsigned long long)(now - started + AHEADMS) * rate / 1000;

    if (due > sentframes + rate * (AHEADMS + LATEMS) / 1000)
        sentframes = due - rate * AHEADMS / 1000;

    while (sentframes < due)
    {

        unsigned int frames = (due - sentframes < CHUNKFRAMES) ? due - sentframes : CHUNKFRAMES;

        mix(chunk, frames);
        channel_send(0, audio, EVENT_DATA, frames * 4, chunk);

        sentframes += frames;

    }

}

static unsigned int setrate(unsigned int value)
{

    struct event_audioconf audioconf;

    audioconf.rate = value;
    audioconf.channels = 2;
    audioconf.bits = 16;

    return channel_send(0, audio, EVENT_AUDIOCONF, sizeof (struct event_audioconf), &audioconf) ? value : 0;

}

static void shutdownsound(void)
{

    if (song)
        music->stop(music);

    if (musicready)
        music->shutdown(music);

    song = NULL;
    musicready = 0;
    audio = 0;

}

/* the configured rate, else 44100 or 48000 Hz, which the drivers take */
void I_InitSound(void)
{

    unsigned int rates[RATES];
    int i;

    rates[0] = snd_samplerate;
    rates[1] = 44100;
    rates[2] = 48000;
    audio = channel_lookup(option_getstring("audio-service"));

    for (i = 0; audio && i < RATES && !rate; i++)
        rate = setrate(rates[i]);

    if (!rate)
    {

        audio = 0;

        lprintf(LO_WARN, "I_InitSound: no audio service, no sound\n");

        return;

    }

    snd_samplerate = rate;

    for (i = 0; i < STEPS; i++)
        steptable[i] = (int)(pow(1.2, ((double)(i - 128) / (64.0 * rate / 11025))) * 65536.0);

    musicready = music->init(music, rate);

    atexit(shutdownsound);
    lprintf(LO_INFO, "I_InitSound: %u Hz%s\n", rate, (musicready) ? ", opl music" : "");

}

void I_ShutdownSound(void)
{

    shutdownsound();

}

void I_SetChannels(void)
{

    memset(channels, 0, sizeof (channels));

}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{

    char namebuf[9];

    sprintf(namebuf, "ds%s", sfx->name);

    return W_SafeGetNumForName(namebuf);

}

int I_StartSound(int id, int channel, int vol, int sep, int pitch, int priority)
{

    const unsigned char *data;
    int lump;
    size_t length;

    if (!audio || channel < 0 || channel >= CHANNELS)
        return -1;

    lump = S_sfx[id].lumpnum;
    length = W_LumpLength(lump);

    if (length <= 8)
        return -1;

    data = W_LockLumpNum(lump);
    channels[channel].id = id;
    channels[channel].samplerate = (data[3] << 8) + data[2];
    channels[channel].data = data + 8;
    channels[channel].enddata = data + length - 1;
    channels[channel].stepremainder = 0;

    updateparams(channel, vol, sep, pitch);

    return channel;

}

void I_StopSound(int handle)
{

    if (handle >= 0 && handle < CHANNELS)
        stopchannel(handle);

}

dboolean I_SoundIsPlaying(int handle)
{

    return handle >= 0 && handle < CHANNELS && channels[handle].data != NULL;

}

dboolean I_AnySoundStillPlaying(void)
{

    int i;

    for (i = 0; i < CHANNELS; i++)
    {

        if (channels[i].data)
            return true;

    }

    return false;

}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{

    if (handle >= 0 && handle < CHANNELS)
        updateparams(handle, vol, sep, pitch);

}

void I_SetMusicVolume(int volume)
{

    if (song)
        music->setvolume(music, volume);

}

void I_PauseSong(int handle)
{

    if (song)
        music->pause(music);

}

void I_ResumeSong(int handle)
{

    if (song)
        music->resume(music);

}

void I_UnRegisterSong(int handle)
{

    if (song)
        music->unregistersong(music, song);

    song = NULL;

    free(songdata);

    songdata = NULL;

}

/* a mus lump becomes midi for the opl player, as upstream's mus2mid path does */
int I_RegisterSong(const void *data, size_t len)
{

    MEMFILE *instream;
    MEMFILE *outstream;
    void *outbuf;
    size_t outlen;

    I_UnRegisterSong(0);

    if (!musicready)
        return 0;

    if (len > 4 && memcmp(data, "MUS", 3))
    {

        song = music->registersong(music, data, len);

        return 0;

    }

    instream = mem_fopen_read(data, len);
    outstream = mem_fopen_write();

    if (!mus2mid(instream, outstream))
    {

        mem_get_buf(outstream, &outbuf, &outlen);

        songdata = malloc(outlen);

        if (songdata)
        {

            memcpy(songdata, outbuf, outlen);

            song = music->registersong(music, songdata, outlen);

        }

    }

    mem_fclose(instream);
    mem_fclose(outstream);

    return 0;

}

int I_RegisterMusic(const char *filename, musicinfo_t *musicinfo)
{

    return 1;

}

void I_PlaySong(int handle, int looping)
{

    if (song)
    {

        music->play(music, song, looping);
        music->setvolume(music, snd_MusicVolume);

    }

}

void I_StopSong(int handle)
{

    if (song)
        music->stop(music);

}

void M_ChangeMIDIPlayer(void)
{

}

void I_InitRecording(void)
{

}

void I_ShutdownRecording(void)
{

}

void I_SetRecordingVolume(int volume)
{

}

void I_StartRecording(void)
{

}

unsigned char *I_GrabRecording(int len)
{

    return NULL;

}

void I_PauseRecordingAudio(void)
{

}

void I_ResumeRecordingAudio(void)
{

}

void I_SeekRecording(int pos)
{

}

void I_PlayRecording(void)
{

}

void I_PauseRecording(void)
{

}

void I_ResumeRecording(void)
{

}

