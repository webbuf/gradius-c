#include <Windows.h>
#include <stdlib.h>
#include <gl/GLU.h>
#include <assert.h>
#include "baseTypes.h"
#include "levelmgr.h"
#include "objmgr.h"
#include "SOIL.h"
#include "field.h"
#include "sound.h"
#include "sprite.h"
#include "gameStateManager.h"

typedef struct level_t
{
    const LevelDef* def;
    Field* field;
    GameManager* gm;
} Level;

static int32_t _soundId = SOUND_NOSOUND;

static void _levelMgrPlaySound(Object* obj);

/// @brief Initialize the level manager
void levelMgrInit()
{
    initTextures();
}

/// @brief Shutdown the level manager
void levelMgrShutdown()
{
    soundUnload(_soundId);
}

/// @brief Loads the level and all required objects/assets
/// @param levelDef 
/// @return 
Level* levelMgrLoad(const LevelDef* levelDef)
{
    Level* level = malloc(sizeof(Level));
    if (level != NULL)
    {
        level->def = levelDef;
        level->field = fieldNew(levelDef->fieldBounds, levelDef->fieldColor);
        level->gm = gameManagerNew(levelDef->fieldBounds);
    }
    return level;
}

/// @brief Unloads the level and frees up any assets associated
/// @param level 
void levelMgrUnload(Level* level)
{
    if (level != NULL)
    {
        fieldDelete(level->field);

        gameManagerDelete(level->gm);
        free(level);
    }
}

static void _levelMgrPlaySound(Object* obj)
{
    soundPlay(_soundId);
}


