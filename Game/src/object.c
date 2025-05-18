#include "baseTypes.h"
#include "object.h"
#include "hitbox.h"

static ObjRegistrationFunc _registerFunc = NULL;
static ObjRegistrationFunc _deregisterFunc = NULL;

/// @brief Enable callback to a registrar on ObjInit/Deinit
/// @param registerFunc 
/// @param deregisterFunc 
void objEnableRegistration(ObjRegistrationFunc registerFunc, ObjRegistrationFunc deregisterFunc)
{
    _registerFunc = registerFunc;
    _deregisterFunc = deregisterFunc;
}

/// @brief Disable registration during ObjInit/Deinit
void objDisableRegistration()
{
    _registerFunc = _deregisterFunc = NULL;
}

/// @brief Initialize an object. Intended to be called from subclass constructors
/// @param obj 
/// @param vtable 
/// @param pos 
/// @param vel 
void objInit(Object* obj, ObjVtable* vtable, Coord2D pos, Coord2D vel, Bounds2D hit)
{
    obj->vtable = vtable;
    obj->position = pos;
    obj->velocity = vel;
    obj->hitbox = hit;

    if (_registerFunc != NULL)
    {
        _registerFunc(obj);
    }
}

/// @brief Deinitialize an object
/// @param obj 
void objDeinit(Object* obj)
{
    if (_deregisterFunc != NULL)
    {
        _deregisterFunc(obj);
    }
}

/// @brief Draw this object, using it's vtable
/// @param obj 
void objDraw(Object* obj)
{
    if (obj->vtable != NULL && obj->vtable->draw != NULL) 
    {
        obj->vtable->draw(obj);
    }
}

/// @brief Update this object, using it's vtable
/// @param obj 
/// @param milliseconds 
void objUpdate(Object* obj, uint32_t milliseconds)
{
    if (obj->vtable != NULL && obj->vtable->update != NULL) 
    {
        obj->vtable->update(obj, milliseconds);
        return;
    }

    objDefaultUpdate(obj, milliseconds);
}

//ensures our shit is good and then calls the other object's callback function is the collision is good
void objCollide(Object* obj, Object* other) {
    if (obj->vtable != NULL && obj->vtable->collide != NULL && other->vtable != NULL && other->vtable->tag != NOCOLLIDE) {
        uint8_t tag = obj->vtable->tag; //store this in case the collision deletes the object
        if (obj->vtable->collide(obj, other)) {
            other->vtable->callback(other, tag);
        }
        return;
    }
}
    //no default collide, because if there's nothing there we probably don't want to do anything


void objDefaultUpdate(Object* obj, uint32_t milliseconds)
{
    if (milliseconds > 16) {
        milliseconds = 16;
    }

    updateHitbox(&obj->hitbox, obj->velocity.x, obj->velocity.y, milliseconds);
    obj->position = boundsGetCenter(&obj->hitbox);
    //modified to always move the hitbox so our bounds are always there
}

bool dummyCollide(Object* o1, Object* o2) {
    return false;
}

void dummyCallback(Object* obj, uint8_t tag) {
    return;
}
//spare returns for things that don't need collision