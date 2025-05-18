#include <windows.h>											// Header File For Windows
#include <stdlib.h>												// Header File For Malloc/Free
#include <stdarg.h>												// Header File For Variable Argument Routines
#include <math.h>												// Header File For Math Operations
#include <gl\gl.h>												// Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include "glut.h"
#include "baseTypes.h"
#include "object.h"
#include "field.h"
#include "shape.h"

typedef struct field_t
{
	Object obj;

	Coord2D size;
	uint32_t color;
} Field;

// the object vtable for all fields
static void _fieldUpdate(Object* obj, uint32_t milliseconds);
static void _fieldDraw(Object* obj);
static ObjVtable _fieldVtable = {
	_fieldDraw,
	_fieldUpdate,
	NOCOLLIDE,
	dummyCollide,
	dummyCallback
};

/// @brief Instantiate and initialize a field object
/// @param bounds 
/// @param color 
/// @return 
Field* fieldNew(Bounds2D bounds, uint32_t color)
{
	Field* field = malloc(sizeof(Field));
	if(field != NULL)
	{
		Coord2D center = boundsGetCenter(&bounds);
		Coord2D vel = { 0.0f, 0.0f };
		objInit(&field->obj, &_fieldVtable, center, vel, bounds);

		field->size = boundsGetDimensions(&bounds);
		field->color = color;
	}
	return field;
}

/// @brief Free up resources
/// @param field 
void fieldDelete(Field* field)
{
	objDeinit(&field->obj);

	free(field);
}

/// @brief Set the color to draw the field border
/// @param field 
/// @param color 
void fieldSetColor(Field* field, long color)
{
	field->color = color;
}

/// @brief Get the current border color
/// @param field 
/// @return 
long fieldGetColor(const Field* field)
{
	return field->color;
}

/// @brief Retrieve the size as a Coord2D where x is width and y is height
/// @param field 
/// @return 
Coord2D fieldGetSize(const Field* field)
{
	return field->size;
}

/// @brief Currently a no-op
/// @param obj 
/// @param milliseconds 
static void _fieldUpdate(Object* obj, uint32_t milliseconds)
{
	objDefaultUpdate(obj, milliseconds);
}

/// @brief Draw the field border
/// @param obj 
static void _fieldDraw(Object* obj)
{
	return;
	//i use a field but i don't want the bounds
};
