#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "common.h"

// ------------------------------------------------------------------ basic primitive helpers
inline void col(float r, float g, float b) {
    glColor3f(r, g, b);
}

void box(float cx, float cy, float cz, float w, float h, float d);
void cyl(float r0, float r1, float h, int sl = 12);
void cylCap(float r0, float r1, float h, int sl = 12);
void cone(float r, float h, int sl = 12);
void sph(float x, float y, float z, float r, int sl = 12);
void ell(float x, float y, float z, float rx, float ry, float rz, int sl = 12);
void prism(float cx, float y0, float cz, float w, float h, float d);
void prismZ(float cx, float y0, float cz, float len, float h, float wid);
void quadUp(float x0, float z0, float x1, float z1, float y);

// ------------------------------------------------------------------ subdivided boxes & walls
void faceGrid(float ax, float ay, float az, float ux, float uy, float uz, int nu,
              float vx, float vy, float vz, int nv, float nx, float ny, float nz,
              float r, float g, float b, float jit);
int cells(float len, float cell);
void bigBox(float x0, float y0, float z0, float x1, float y1, float z1,
            float r, float g, float b, float jit = 0.10f, float cell = 1.0f);
void wallBox(float x0, float y0, float z0, float x1, float y1, float z1,
             float r, float g, float b, float jit = 0.10f, float cell = 1.0f);
void lining(float x0, float y0, float z0, float x1, float y1, float z1,
            float r, float g, float b);

// ------------------------------------------------------------------ materials
void mat(float r, float g, float b, float spec, float shin);
void matStone();
void matWood();
void matRust();
void matIron();

// ------------------------------------------------------------------ flame & windows
void flame(float x, float y, float z, float s, float ph);
void windowFace(float w, float h, int type, bool arch);
void window(float x, float y, float z, float rot, float w, float h, int outer, int inner, bool arch = false);
int litHash(float a, float b, float c);

#endif // PRIMITIVES_H
