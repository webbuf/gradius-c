#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// object "virtual" functions
typedef struct object_t Object;
typedef void (*ObjDrawFunc)(Object*);
typedef void (*ObjUpdateFunc)(Object*, uint32_t);
typedef bool (*ObjCollideFunc)(Object*, Object*);
typedef void (*ObjCollideCallback)(Object*, uint8_t);

typedef enum tag_t{
    NOCOLLIDE,
    PLAYER,
    PROJECTILE,
    ENEMY,
    POWERUP,
    TERRAIN,
    BOSS,
    ENEMY_PROJECTILE
}Tags;

typedef struct object_vtable_t {
    ObjDrawFunc        draw;
    ObjUpdateFunc      update;
    Tags               tag;
    ObjCollideFunc     collide;
    ObjCollideCallback callback;
} ObjVtable;

typedef struct object_t {
    ObjVtable*      vtable;
    Coord2D         position;
    Coord2D         velocity;
    Bounds2D        hitbox;
} Object;

typedef void (*ObjRegistrationFunc)(Object*);

// class-wide registration methods
void objEnableRegistration(ObjRegistrationFunc registerFunc, ObjRegistrationFunc deregisterFunc);
void objDisableRegistration();

// object API
void objInit(Object* obj, ObjVtable* vtable, Coord2D pos, Coord2D vel, Bounds2D hit);
void objDeinit(Object* obj);
void objDraw(Object* obj);
void objUpdate(Object* obj, uint32_t milliseconds);
void objCollide(Object* obj, Object* other);

// default update implementation that just moves at the current velocity
void objDefaultUpdate(Object* obj, uint32_t milliseconds);
bool dummyCollide(Object* o1, Object* o2);
void dummyCallback(Object* obj, uint8_t tag);

#ifdef __cplusplus
}
#endif