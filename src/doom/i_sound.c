#include "config.h"
#include "doomtype.h"
#include "doomstat.h"
#include "sounds.h"
#include "w_wad.h"
#include "i_sound.h"

/* no sound yet: effects and music are accepted and dropped */
int snd_pcspeaker;
int snd_card = 1;
int mus_card = 1;
int snd_samplerate = 11025;
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

void I_InitSound(void)
{

}

void I_ShutdownSound(void)
{

}

void I_SetChannels(void)
{

}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{

    char namebuf[9];

    sprintf(namebuf, "ds%s", sfx->name);

    return W_SafeGetNumForName(namebuf);

}

int I_StartSound(int id, int channel, int vol, int sep, int pitch, int priority)
{

    return -1;

}

void I_StopSound(int handle)
{

}

dboolean I_SoundIsPlaying(int handle)
{

    return false;

}

dboolean I_AnySoundStillPlaying(void)
{

    return false;

}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{

}

void I_SetMusicVolume(int volume)
{

}

void I_PauseSong(int handle)
{

}

void I_ResumeSong(int handle)
{

}

int I_RegisterSong(const void *data, size_t len)
{

    return 0;

}

int I_RegisterMusic(const char *filename, musicinfo_t *music)
{

    return 1;

}

void I_PlaySong(int handle, int looping)
{

}

void I_StopSong(int handle)
{

}

void I_UnRegisterSong(int handle)
{

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

