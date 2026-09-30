#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <GL/glu.h>

// ------------------------------------------------------------------ constants
static const float PI = 3.14159265f;
static const float DEG = 57.2957795f;
static const float WH = 9.0f;      // wall height of the mansion (hall is double height)
static const float F0 = 0.30f;     // ground-floor level (plinth)
static const float F1 = 4.50f;     // upstairs floor level (14 steps x 0.3 above F0)

// ------------------------------------------------------------------ structures
struct Box {
    float x0, y0, z0, x1, y1, z1;
};

struct Door {
    float hx, hy, hz, baseRot, w, h, sgn, maxA;   // hinge, closed direction, size, swing
    float open, target;
    bool latch;
    Box gap;
};

struct Spot {
    float x, z, s;
    unsigned seed;
};

// display-list ids
enum {
    LG_GROUND,
    LG_PATH,
    LG_FENCE,
    LG_TREES,
    LG_PINES,
    LG_GRAVE,
    LG_EXT,
    LG_INT,
    LG_RAIL,
    LG_PROPS,
    LG_LEAF,
    LG_COUNT
};

// ------------------------------------------------------------------ globals (extern)
extern GLUquadric* Q;
extern int winW, winH;

// player / camera
extern float px, pz, feet, yaw, pitchA, walkPhase;
extern bool kW, kA, kS, kD, shiftDown;
extern bool tL, tR, tU, tD;
extern bool flashOn, hudOn, mouseLook, warpIgnore;

// time & atmosphere
extern float simTime, fogDens, indoorAmt;
extern float flashV, flashT, nextFlash;
extern int lastMs, fpsFrames;
extern float fpsVal, fpsAcc;
extern const float fogBase[3];

// flicker values
extern float fChand, fCandle, fLamp;

// display lists & randomness
extern GLuint base;
extern unsigned rs;

// collision solids
extern std::vector<Box> solids;

// doors & gate
extern Door doors[4];
extern float gateOpen;
extern bool gateToggle;

// trees
extern std::vector<Spot> deadTrees, pines;

// camera forward vector
extern float fwd[3];

// ------------------------------------------------------------------ inline random
inline float rnd() {
    rs = rs * 1664525u + 1013904223u;
    return ((rs >> 8) & 0xFFFF) / 65535.0f;
}

inline float rndr(float a, float b) {
    return a + (b - a) * rnd();
}

// ------------------------------------------------------------------ collision helper
void solid(float x0, float y0, float z0, float x1, float y1, float z1);

#endif // COMMON_H
