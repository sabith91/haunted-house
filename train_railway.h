#ifndef TRAIN_RAILWAY_H
#define TRAIN_RAILWAY_H

#include "common.h"
#include "primitives.h"

// ------------------------------------------------------------------ track path geometry
float trackX(float z);
float trackYaw(float z);

// ------------------------------------------------------------------ static railway list
void buildRail();

// ------------------------------------------------------------------ dynamic train animation
void wheel(float x, float y, float z, float ang);
float trainHeadZ();
void drawTrain();

#endif // TRAIN_RAILWAY_H
