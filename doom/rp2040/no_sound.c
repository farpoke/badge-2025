
#include "../i_sound.h"
#include "../s_sound.h"

int snd_SfxVolume = 0;
int snd_MusicVolume = 0;
int numChannels = 0;

void I_InitSound() {}
void I_SubmitSound(void) {}
void I_ShutdownSound(void) {}

void I_InitMusic(void) {}
void I_ShutdownMusic(void) {}

void S_Init(int sfxVolume, int musicVolume) {}
void S_Start(void) {}
void S_StartSound(void *origin, int sound_id) {}
void S_StartSoundAtVolume(void *origin, int sound_id, int volume) {}
void S_StopSound(void *origin) {}
void S_StartMusic(int music_id) {}
void S_ChangeMusic(int music_id, int looping) {}
void S_StopMusic(void) {}
void S_PauseSound(void) {}
void S_ResumeSound(void) {}
void S_UpdateSounds(void *listener) {}
void S_SetMusicVolume(int volume) {}
void S_SetSfxVolume(int volume) {}
