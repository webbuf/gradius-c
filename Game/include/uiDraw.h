#pragma once
#include "baseTypes.h"
#include "enemyManager.h"
#include "vicViper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_t UI;

UI* uiNew(int8_t lives, EnemyManager* enemy, VicViper* player);

void uiDelete(UI* ui);

#ifdef __cplusplus
}
#endif