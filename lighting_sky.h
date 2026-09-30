#ifndef LIGHTING_SKY_H
#define LIGHTING_SKY_H

#include "common.h"
#include "train_railway.h"
#include "dynamic_objects.h"

// ------------------------------------------------------------------ sky dome & starfield
extern float stars[200][3];
void initStars();
void drawSky(float cx, float cy, float cz);

// ------------------------------------------------------------------ fixed-function lighting
void setPointLight(GLenum L, float x, float y, float z,
                   float r, float g, float b, float c, float l, float q);
bool closeTo(float x, float y, float z, float R, float ex, float ey, float ez);
void setupWorldLights(float ex, float ey, float ez);

// ------------------------------------------------------------------ 2D heads-up display
void text(float x, float y, const char* s, void* font = GLUT_BITMAP_HELVETICA_12);
const char* zoneName();
void drawHUD();

#endif // LIGHTING_SKY_H
