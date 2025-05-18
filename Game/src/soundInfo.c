#include "soundInfo.h"

#define FILENAME_MAX_LENGTH 50

static char soundFilenames[SOUND_COUNT][FILENAME_MAX_LENGTH] = {
	"asset/SFX/pew.wav",
	"asset/SFX/missile.wav",
	"asset/SFX/powerup.wav",
	"asset/SFX/pichuun.wav",
	"aseet/SFX/enemyShoot.wav",
	"asset/SFX/enemyDie.wav",
	"asset/SFX/bossShoot.wav",
	"asset/SFX/bgm.wav"
};
//array of filenames

static int32_t soundHandles[SOUND_COUNT] = {
	SOUND_NOSOUND,
	SOUND_NOSOUND,
	SOUND_NOSOUND,
	SOUND_NOSOUND,
	SOUND_NOSOUND,
	SOUND_NOSOUND,
	SOUND_NOSOUND,
	SOUND_NOSOUND
};
//array of handles

//getters and setters for above arrays since they're private
char* getSoundFilename(int32_t index) {
	return soundFilenames[index];
}

int32_t getSoundHandle(int32_t index) {
	return soundHandles[index];
}

void setSoundHandle(int32_t index, int32_t handle) {
	soundHandles[index] = handle;
}