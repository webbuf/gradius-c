#include "hitbox.h"
#include "shape.h"

//moves hitbox corners by given amount
void updateHitbox(Bounds2D* hitbox, float x, float y, uint32_t milliseconds) {
	hitbox->topLeft.x += x * ((float)milliseconds / 1000);
	hitbox->topLeft.y += y * ((float)milliseconds / 1000);
	hitbox->botRight.x += x * ((float)milliseconds / 1000);
	hitbox->botRight.y += y * ((float)milliseconds / 1000);
}

//AABB collision
bool colliding(Bounds2D* h1, Bounds2D* h2) {

	float h1width = h1->botRight.x - h1->topLeft.x;
	float h1height = h1->botRight.y - h1->topLeft.y;

	float h2width = h2->botRight.x - h2->topLeft.x;
	float h2height = h2->botRight.y - h2->topLeft.y;

	float h1x = boundsGetCenter(h1).x;
	float h1y = boundsGetCenter(h1).y;

	float h2x = boundsGetCenter(h2).x;
	float h2y = boundsGetCenter(h2).y;

	if (h1x < h2x + h2width &&
		h1x + h1width > h2x &&
		h1y < h2y + h2height &&
		h1y + h1height > h2y) {
		return true;
	}

	return false;
}

//debug function to draw hitbox bounds
void drawHitbox(Bounds2D* h) {
	shapeDrawLine(h->topLeft.x, h->topLeft.y, h->botRight.x, h->botRight.y, 255, 0, 0);
	shapeDrawLine(h->topLeft.x, h->botRight.y, h->botRight.x, h->topLeft.y, 255, 0, 0);
}