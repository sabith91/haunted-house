#ifndef DYNAMIC_OBJECTS_H
#define DYNAMIC_OBJECTS_H

#include "common.h"
#include "primitives.h"
#include "house_interior.h"

// ------------------------------------------------------------------ animated estate & house features
void drawDoorLeaf(const Door& d);
void drawGate();
void drawChandelier();
void drawCandelabra(float x, float y, float z, float ph);
void drape(float x, float y, float z, float w, float h, float rot,
           float ph, float r, float g, float b, float tear);
void rockingChair();
void drawLampBulbs();
void drawCandleB();

// ------------------------------------------------------------------ animated creatures & atmosphere
void winged(float scale, float flapAmp, float flapSpeed, float ph, float r, float g, float b);
void drawBatsCrows();
void ghostPos(float& x, float& y, float& z);
void drawGhost(float cx, float cz);
void drawBeams();
void drawFogSheets();

#endif // DYNAMIC_OBJECTS_H
