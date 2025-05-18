#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct background_t Background;

//typical new/delete constructors. not really going to explain these
Background* backgroundNew(uint8_t state);
void backgroundDelete(Background* b);

void backgroundPauseScroll(Background* b);
void backgroundResumeScroll(Background* b);

#ifdef __cplusplus
}
#endif