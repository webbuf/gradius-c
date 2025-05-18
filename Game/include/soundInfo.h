#pragma once
#include "baseTypes.h"
#include "sound.h"

#ifdef __cplusplus
extern "C" {
#endif

enum SoundIndices {
	SOUND_BASIC_SHOOT,
	SOUND_MISSILE_SHOOT,
	SOUND_BANK_POWERUP,
	SOUND_PLAYER_DIE,
	SOUND_ENEMY_SHOOT,
	SOUND_ENEMY_DIE,
	SOUND_BOSS_SHOOT,
	SOUND_BGM,
	SOUND_COUNT
};

char* getSoundFilename(int32_t index);
int32_t getSoundHandle(int32_t index);
void setSoundHandle(int32_t index, int32_t handle);

#ifdef __cplusplus
}
#endif