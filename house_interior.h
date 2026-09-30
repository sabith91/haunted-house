#ifndef HOUSE_INTERIOR_H
#define HOUSE_INTERIOR_H

#include "common.h"
#include "primitives.h"

// ------------------------------------------------------------------ interior furniture & decor
void portrait(float x, float y, float z, float rotY, float w, float h, float tilt);
void bed(float x, float y, float z, float rot, float wid, float len,
         float fr, float fg, float fb, float mr, float mg, float mb);
void ironBed(float x, float y, float z, float rot);
void dresser(float x, float y, float z);
void radiator(float x, float y, float z);
void candelabraBase(float x, float y, float z);

// ------------------------------------------------------------------ static house interior list
void buildHouseInt();

#endif // HOUSE_INTERIOR_H
