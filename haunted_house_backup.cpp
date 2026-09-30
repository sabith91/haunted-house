// ============================================================================
//  Interactive 3D Haunted House Simulation      Simple OpenGL Project | Roll 2107091
//  Student : MD.SABITH  (CSE, 3rd year)
//  Tech    : C++, OpenGL 1.x (fixed function), GLUT / FreeGLUT, GLU quadrics
//  Mood    : Granny (house horror) + Alan Wake 2 (foggy forest, railway, flashlight)
//
//  Everything is built from primitives (cube, prism, cylinder, sphere, cone,
//  torus and quads) - no model files, no texture files.
//
//  Scene (see report, Section 4)      X to the right, Z towards the viewer
//     fenced compound 80 x 80  (fence at +-40), main gate at Z = +40
//     mansion  X -12..12, Z -30..-14   (hall, staircase, gallery, 3 bedrooms)
//     graveyard X -38..-22, Z 12..34
//     railway  X = 55 + 8 sin(Z/25), outside the east fence
//
//  Controls
//     W A S D  walk (Shift = run)      Mouse / arrow keys  look around
//     F  flashlight      E  open/close nearest door or gate   L  lightning
//     1 gate   2 entrance hall   3 bedroom   4 railway        H  help overlay
//     M  mouse look on/off                                     Esc quit
// ============================================================================
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

// ------------------------------------------------------------------ globals
static GLUquadric* Q;
static int winW = 1280, winH = 720;

// player / camera
static float px = 0, pz = 47, feet = 0, yaw = 0, pitchA = 0.02f, walkPhase = 0;
static bool kW = false, kA = false, kS = false, kD = false, shiftDown = false;
static bool tL = false, tR = false, tU = false, tD = false;      // arrow keys
static bool flashOn = true, hudOn = true, mouseLook = true, warpIgnore = false;

// time & atmosphere
static float simTime = 0, fogDens = 0.018f, indoorAmt = 0;
static float flashV = 0, flashT = -1, nextFlash = 9;             // lightning
static int lastMs = 0, fpsFrames = 0; static float fpsVal = 0, fpsAcc = 0;
static const float fogBase[3] = { 0.08f, 0.12f, 0.13f };

// flicker values (updated every frame)
static float fChand = 1, fCandle = 1, fLamp = 1;

// display-list ids
enum { LG_GROUND, LG_PATH, LG_FENCE, LG_TREES, LG_PINES, LG_GRAVE, LG_EXT, LG_INT,
       LG_RAIL, LG_PROPS, LG_LEAF, LG_COUNT };
static GLuint base;

// ------------------------------------------------------------------ random
static unsigned rs = 12345;
static float rnd() { rs = rs * 1664525u + 1013904223u; return ((rs >> 8) & 0xFFFF) / 65535.0f; }
static float rndr(float a, float b) { return a + (b - a) * rnd(); }

// ------------------------------------------------------------------ collision
struct Box { float x0, y0, z0, x1, y1, z1; };
static std::vector<Box> solids;
static void solid(float x0, float y0, float z0, float x1, float y1, float z1) {
    Box b = { x0, y0, z0, x1, y1, z1 }; solids.push_back(b);
}

// ------------------------------------------------------------------ doors
struct Door {
    float hx, hy, hz, baseRot, w, h, sgn, maxA;   // hinge, closed direction, size, swing
    float open, target; bool latch; Box gap;
};
static Door doors[4];                              // 0 front door, 1..3 bedroom doors
static float gateOpen = 0; static bool gateToggle = false;

// trees (positions kept for drawing + collision)
struct Spot { float x, z, s; unsigned seed; };
static std::vector<Spot> deadTrees, pines;

// ============================================================================
//  PRIMITIVE HELPERS
// ============================================================================
static inline void col(float r, float g, float b) { glColor3f(r, g, b); }

static void box(float cx, float cy, float cz, float w, float h, float d) {
    glPushMatrix(); glTranslatef(cx, cy, cz); glScalef(w, h, d); glutSolidCube(1.0); glPopMatrix();
}
static void cyl(float r0, float r1, float h, int sl = 12) {          // vertical, from y=0 up
    glPushMatrix(); glRotatef(-90, 1, 0, 0); gluCylinder(Q, r0, r1, h, sl, 1); glPopMatrix();
}
static void cylCap(float r0, float r1, float h, int sl = 12) {
    cyl(r0, r1, h, sl);
    glPushMatrix(); glTranslatef(0, h, 0); glRotatef(-90, 1, 0, 0); gluDisk(Q, 0, r1, sl, 1); glPopMatrix();
}
static void cone(float r, float h, int sl = 12) {                     // vertical, base at y=0
    glPushMatrix(); glRotatef(-90, 1, 0, 0); glutSolidCone(r, h, sl, 1); glPopMatrix();
}
static void sph(float x, float y, float z, float r, int sl = 12) {
    glPushMatrix(); glTranslatef(x, y, z); glutSolidSphere(r, sl, sl); glPopMatrix();
}
static void ell(float x, float y, float z, float rx, float ry, float rz, int sl = 12) {
    glPushMatrix(); glTranslatef(x, y, z); glScalef(rx, ry, rz); glutSolidSphere(1.0, sl, sl); glPopMatrix();
}
// triangular prism: ridge along X (length w), cross-section d wide, h high, base on y0
static void prism(float cx, float y0, float cz, float w, float h, float d) {
    float hw = w / 2, hd = d / 2, l = sqrtf(h * h + hd * hd), ny = hd / l, nz = h / l;
    glPushMatrix(); glTranslatef(cx, y0, cz);
    glBegin(GL_QUADS);
    glNormal3f(0, ny, nz);  glVertex3f(-hw, 0, hd);  glVertex3f(hw, 0, hd);  glVertex3f(hw, h, 0);  glVertex3f(-hw, h, 0);
    glNormal3f(0, ny, -nz); glVertex3f(hw, 0, -hd);  glVertex3f(-hw, 0, -hd); glVertex3f(-hw, h, 0); glVertex3f(hw, h, 0);
    glNormal3f(0, -1, 0);   glVertex3f(-hw, 0, hd);  glVertex3f(-hw, 0, -hd); glVertex3f(hw, 0, -hd); glVertex3f(hw, 0, hd);
    glEnd();
    glBegin(GL_TRIANGLES);
    glNormal3f(1, 0, 0);  glVertex3f(hw, 0, hd);   glVertex3f(hw, 0, -hd);  glVertex3f(hw, h, 0);
    glNormal3f(-1, 0, 0); glVertex3f(-hw, 0, -hd); glVertex3f(-hw, 0, hd);  glVertex3f(-hw, h, 0);
    glEnd();
    glPopMatrix();
}
// prism whose ridge runs along Z (front-facing gable)
static void prismZ(float cx, float y0, float cz, float len, float h, float wid) {
    glPushMatrix(); glTranslatef(cx, 0, cz); glRotatef(90, 0, 1, 0); prism(0, y0, 0, len, h, wid); glPopMatrix();
}
static void quadUp(float x0, float z0, float x1, float z1, float y) {
    glBegin(GL_QUADS); glNormal3f(0, 1, 0);
    glVertex3f(x0, y, z1); glVertex3f(x1, y, z1); glVertex3f(x1, y, z0); glVertex3f(x0, y, z0); glEnd();
}

// One face of a sub-divided box (per-cell brightness noise gives a stone / plank
// look and lets the fixed-function light work per cell instead of per big face).
static void faceGrid(float ax, float ay, float az, float ux, float uy, float uz, int nu,
                     float vx, float vy, float vz, int nv, float nx, float ny, float nz,
                     float r, float g, float b, float jit) {
    glNormal3f(nx, ny, nz);
    glBegin(GL_QUADS);
    for (int i = 0; i < nu; i++) for (int j = 0; j < nv; j++) {
        float k = 1.0f + (rnd() - 0.5f) * 2 * jit; glColor3f(r * k, g * k, b * k);
        float u0 = (float)i / nu, u1 = (float)(i + 1) / nu, v0 = (float)j / nv, v1 = (float)(j + 1) / nv;
        glVertex3f(ax + ux * u0 + vx * v0, ay + uy * u0 + vy * v0, az + uz * u0 + vz * v0);
        glVertex3f(ax + ux * u1 + vx * v0, ay + uy * u1 + vy * v0, az + uz * u1 + vz * v0);
        glVertex3f(ax + ux * u1 + vx * v1, ay + uy * u1 + vy * v1, az + uz * u1 + vz * v1);
        glVertex3f(ax + ux * u0 + vx * v1, ay + uy * u0 + vy * v1, az + uz * u0 + vz * v1);
    }
    glEnd();
}
static int cells(float len, float cell) { int n = (int)ceilf(len / cell); return n < 1 ? 1 : n; }
static void bigBox(float x0, float y0, float z0, float x1, float y1, float z1,
                   float r, float g, float b, float jit = 0.10f, float cell = 1.0f) {
    float dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
    int nx = cells(dx, cell), ny = cells(dy, cell), nz = cells(dz, cell);
    faceGrid(x0, y1, z0, dx, 0, 0, nx, 0, 0, dz, nz, 0, 1, 0, r, g, b, jit);
    faceGrid(x0, y0, z0, dx, 0, 0, nx, 0, 0, dz, nz, 0, -1, 0, r, g, b, jit);
    faceGrid(x0, y0, z1, dx, 0, 0, nx, 0, dy, 0, ny, 0, 0, 1, r, g, b, jit);
    faceGrid(x0, y0, z0, dx, 0, 0, nx, 0, dy, 0, ny, 0, 0, -1, r, g, b, jit);
    faceGrid(x1, y0, z0, 0, 0, dz, nz, 0, dy, 0, ny, 1, 0, 0, r, g, b, jit);
    faceGrid(x0, y0, z0, 0, 0, dz, nz, 0, dy, 0, ny, -1, 0, 0, r, g, b, jit);
}
// solid wall: draws AND registers collision
static void wallBox(float x0, float y0, float z0, float x1, float y1, float z1,
                    float r, float g, float b, float jit = 0.10f, float cell = 1.0f) {
    solid(x0, y0, z0, x1, y1, z1); bigBox(x0, y0, z0, x1, y1, z1, r, g, b, jit, cell);
}
// interior lining / wallpaper layer (no collision)
static void lining(float x0, float y0, float z0, float x1, float y1, float z1, float r, float g, float b) {
    bigBox(x0, y0, z0, x1, y1, z1, r, g, b, 0.14f, 0.8f);
}

// ------------------------------------------------------------------ materials
static void mat(float r, float g, float b, float spec, float shin) {
    glColor3f(r, g, b);
    GLfloat s[4] = { spec, spec, spec, 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shin);
}
static void matStone() { mat(0.32f, 0.32f, 0.34f, 0.03f, 4); }
static void matWood()  { mat(0.25f, 0.16f, 0.09f, 0.05f, 8); }
static void matRust()  { mat(0.45f, 0.22f, 0.10f, 0.10f, 12); }
static void matIron()  { mat(0.07f, 0.07f, 0.08f, 0.35f, 30); }

// ------------------------------------------------------------------ helpers
static float trackX(float z) { return 55.0f + 8.0f * sinf(z / 25.0f); }
static float trackYaw(float z) { return atan2f(8.0f / 25.0f * cosf(z / 25.0f), 1.0f) * DEG; }

static void flame(float x, float y, float z, float s, float ph) {   // emissive candle flame
    float f = 0.75f + 0.25f * sinf(simTime * 13 + ph) + 0.15f * sinf(simTime * 29 + ph * 2);
    glDisable(GL_LIGHTING);
    glPushMatrix(); glTranslatef(x, y, z); glColor3f(1.0f, 0.62f + 0.2f * f, 0.15f);
    glScalef(1, f, 1); cone(0.028f * s, 0.11f * s, 6); glPopMatrix();
    glEnable(GL_LIGHTING);
}

// ============================================================================
//  WINDOWS (emissive quads, drawn without lighting)
// ============================================================================
// type 0 dark+cracked, 1 lit amber, 2 pale moonlight, 3 pale + iron bars
static void windowFace(float w, float h, int type, bool arch) {
    glDisable(GL_LIGHTING);
    float r = 0.02f, g = 0.03f, b = 0.04f;
    if (type == 1) { r = 1.0f; g = 0.78f; b = 0.30f; }
    if (type >= 2) { r = 0.30f; g = 0.44f; b = 0.50f; }
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(-w / 2, -h / 2, 0); glVertex3f(w / 2, -h / 2, 0); glVertex3f(w / 2, h / 2, 0); glVertex3f(-w / 2, h / 2, 0);
    glEnd();
    if (arch) {
        glBegin(GL_TRIANGLE_FAN); glVertex3f(0, h / 2, 0);
        for (int i = 0; i <= 12; i++) { float a = PI * i / 12; glVertex3f(w / 2 * cosf(a), h / 2 + w / 2 * sinf(a), 0); }
        glEnd();
    }
    float top = arch ? h / 2 + w / 2 : h / 2;
    glColor3f(0.05f, 0.04f, 0.03f);                        // mullions
    glBegin(GL_QUADS);
    glVertex3f(-0.03f, -h / 2, 0.005f); glVertex3f(0.03f, -h / 2, 0.005f); glVertex3f(0.03f, top, 0.005f); glVertex3f(-0.03f, top, 0.005f);
    glVertex3f(-w / 2, -0.03f, 0.005f); glVertex3f(w / 2, -0.03f, 0.005f); glVertex3f(w / 2, 0.03f, 0.005f); glVertex3f(-w / 2, 0.03f, 0.005f);
    if (type == 3) for (float x = -w / 2 + 0.15f; x < w / 2; x += 0.2f) {
        glVertex3f(x - 0.015f, -h / 2, 0.01f); glVertex3f(x + 0.015f, -h / 2, 0.01f);
        glVertex3f(x + 0.015f, h / 2, 0.01f);  glVertex3f(x - 0.015f, h / 2, 0.01f);
    }
    glEnd();
    if (type == 0) {                                        // cracked glass
        glColor3f(0.25f, 0.28f, 0.30f); glBegin(GL_LINES);
        glVertex3f(-w * 0.3f, h * 0.3f, 0.006f); glVertex3f(0.0f, 0.0f, 0.006f);
        glVertex3f(0.0f, 0.0f, 0.006f); glVertex3f(w * 0.25f, -h * 0.35f, 0.006f);
        glVertex3f(0.0f, 0.0f, 0.006f); glVertex3f(w * 0.35f, h * 0.2f, 0.006f);
        glEnd();
    }
    glEnable(GL_LIGHTING);
}
// wall centred at (x,y,z); rot = direction the OUTSIDE faces (0:+Z, 90:+X, 180:-Z, -90:-X)
static void window(float x, float y, float z, float rot, float w, float h, int outer, int inner, bool arch = false) {
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(rot, 0, 1, 0);
    glPushMatrix(); glTranslatef(0, 0, 0.22f); windowFace(w, h, outer, arch); glPopMatrix();
    if (inner >= 0) { glPushMatrix(); glTranslatef(0, 0, -0.30f); glRotatef(180, 0, 1, 0); windowFace(w, h, inner, arch); glPopMatrix(); }
    glPopMatrix();
}
static int litHash(float a, float b, float c) { return (((int)(a * 3.f + b * 7.f + c * 5.f + 100)) % 3 + 3) % 3; }

// ============================================================================
//  DEAD TREES, PINES
// ============================================================================
static void branch(float len, float r, int depth) {
    col(0.16f, 0.12f, 0.09f);
    cyl(r, r * 0.5f, len, 6);
    if (depth <= 0) return;
    int n = 2 + (int)(rnd() * 2);
    for (int i = 0; i < n; i++) {
        glPushMatrix();
        glTranslatef(0, len * (0.5f + 0.45f * rnd()), 0);
        glRotatef(rnd() * 360, 0, 1, 0); glRotatef(25 + rnd() * 35, 0, 0, 1);
        branch(len * (0.55f + 0.2f * rnd()), r * 0.55f, depth - 1);
        glPopMatrix();
    }
}
static void deadTree(unsigned seed, float s) {
    rs = seed; glPushMatrix(); glScalef(s, s, s);
    col(0.15f, 0.11f, 0.08f); cyl(0.38f, 0.11f, 4.6f, 8);
    int nb = 4 + (int)(rnd() * 3);
    for (int i = 0; i < nb; i++) {
        glPushMatrix(); glTranslatef(0, 1.8f + rnd() * 2.6f, 0);
        glRotatef(rnd() * 360, 0, 1, 0); glRotatef(30 + rnd() * 30, 0, 0, 1);
        branch(1.5f + rnd(), 0.12f, 1); glPopMatrix();
    }
    glPopMatrix();
}
static void pineTree(float s) {
    glPushMatrix(); glScalef(s, s, s);
    col(0.15f, 0.10f, 0.07f); cyl(0.28f, 0.18f, 3.2f, 6);
    col(0.03f, 0.10f, 0.06f);
    glPushMatrix(); glTranslatef(0, 2.0f, 0); cone(2.3f, 3.6f, 8); glPopMatrix();
    col(0.035f, 0.115f, 0.065f);
    glPushMatrix(); glTranslatef(0, 4.0f, 0); cone(1.8f, 3.2f, 8); glPopMatrix();
    col(0.04f, 0.12f, 0.07f);
    glPushMatrix(); glTranslatef(0, 5.9f, 0); cone(1.2f, 3.0f, 8); glPopMatrix();
    glPopMatrix();
}
static bool inHouseZone(float x, float z, float m) { return fabsf(x) < 12 + m && z < -12 + m && z > -31 - m; }
static void genTrees() {
    rs = 4242;
    while ((int)deadTrees.size() < 40) {
        Spot t; t.x = rndr(-38, 38); t.z = rndr(-38, 38); t.s = rndr(0.85f, 1.3f); t.seed = (unsigned)(rnd() * 100000) + 7;
        if (fabsf(t.x) < 3.5f && t.z > -16) continue;                 // keep the path free
        if (inHouseZone(t.x, t.z, 4)) continue;
        if (fabsf(t.x - 13) < 4 && fabsf(t.z + 6) < 4) continue;      // wagon
        if (fabsf(t.x) < 10 && t.z > 32) continue;                    // gate area
        deadTrees.push_back(t);
    }
    rs = 9191;
    // rows on both sides of the railway
    for (float z = -150; z < 150; z += 5.5f) for (int side = -1; side <= 1; side += 2) for (int row = 0; row < 2; row++) {
        Spot t; float off = (row == 0 ? rndr(6.5f, 9.5f) : rndr(12, 19)) * side;
        t.x = trackX(z) + off; t.z = z + rndr(-2, 2); t.s = rndr(0.9f, 1.6f); t.seed = 0; pines.push_back(t);
    }
    // forest ring around the fence
    int n = 0;
    while (n < 230) {
        Spot t; t.x = rndr(-125, 125); t.z = rndr(-125, 125); t.s = rndr(0.9f, 1.6f); t.seed = 0;
        if (fabsf(t.x) < 47 && fabsf(t.z) < 47) continue;
        if (fabsf(t.x - trackX(t.z)) < 22) continue;
        if (fabsf(t.x) < 8 && t.z > 40) continue;                     // keep the approach to the gate clear
        pines.push_back(t); n++;
    }
}

// ============================================================================
//  STATIC DISPLAY LISTS
// ============================================================================
static void buildGround() {
    glNewList(base + LG_GROUND, GL_COMPILE); rs = 101;
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    for (float x = -144; x < 144; x += 8) for (float z = -144; z < 144; z += 8) {
        if (x >= -48 && x + 8 <= 48 && z >= -48 && z + 8 <= 48) continue;
        float k = 0.85f + 0.3f * rnd(); glColor3f(0.04f * k, 0.09f * k, 0.05f * k);
        glVertex3f(x, 0, z + 8); glVertex3f(x + 8, 0, z + 8); glVertex3f(x + 8, 0, z); glVertex3f(x, 0, z);
    }
    for (float x = -48; x < 48; x += 2) for (float z = -48; z < 48; z += 2) {
        float k = 0.80f + 0.4f * rnd(); glColor3f(0.06f * k, 0.14f * k, 0.07f * k);
        glVertex3f(x, 0, z + 2); glVertex3f(x + 2, 0, z + 2); glVertex3f(x + 2, 0, z); glVertex3f(x, 0, z);
    }
    glEnd();
    col(0.07f, 0.17f, 0.08f);                                        // grass tufts (thin cones)
    for (int i = 0; i < 260; i++) {
        float x = rndr(-44, 44), z = rndr(-44, 44); if (fabsf(x) < 2.2f && z > -14) continue; if (fabsf(x) < 13 && z < -13 && z > -31) continue;
        glPushMatrix(); glTranslatef(x, 0, z);
        for (int j = 0; j < 3; j++) { glPushMatrix(); glRotatef(rndr(0, 360), 0, 1, 0); glTranslatef(0.06f, 0, 0);
            glRotatef(rndr(-14, 14), 0, 0, 1); cone(0.035f, rndr(0.4f, 0.8f), 4); glPopMatrix(); }
        glPopMatrix();
    }
    glEndList();
}
static void buildPath() {
    glNewList(base + LG_PATH, GL_COMPILE); rs = 202;
    glNormal3f(0, 1, 0); glBegin(GL_QUADS);
    for (float x = -1.5f; x < 1.5f; x += 0.75f) for (float z = -11.5f; z < 60; z += 0.75f) {
        float k = 0.65f + 0.7f * rnd(), j = rndr(-0.02f, 0.02f);
        glColor3f(0.30f * k, 0.30f * k, 0.32f * k);
        glVertex3f(x + 0.03f, 0.02f, z + 0.72f + j); glVertex3f(x + 0.72f, 0.02f, z + 0.72f);
        glVertex3f(x + 0.72f, 0.02f, z + 0.03f);     glVertex3f(x + 0.03f, 0.02f, z + 0.03f - j);
    }
    glEnd(); glEndList();
}

static void pillar(float x, float z) {
    matStone();
    box(x, 1.6f, z, 0.9f, 3.2f, 0.9f); box(x, 3.3f, z, 1.15f, 0.2f, 1.15f);
    box(x, 0.25f, z, 1.1f, 0.5f, 1.1f);
    col(0.28f, 0.28f, 0.3f); sph(x, 3.75f, z, 0.3f);
}
static void gargoyle(float x, float y, float z, float rotY) {
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(rotY, 0, 1, 0); col(0.18f, 0.18f, 0.20f);
    box(0, 0.3f, 0, 0.5f, 0.6f, 0.4f); sph(0, 0.78f, 0.12f, 0.2f, 10);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix(); glTranslatef(s * 0.09f, 0.94f, 0.12f); glRotatef(-s * 20, 0, 0, 1); cone(0.04f, 0.16f, 6); glPopMatrix();   // horns
        glPushMatrix(); glTranslatef(s * 0.25f, 0.4f, -0.12f); glRotatef(-s * 55, 0, 0, 1); glScalef(1, 1, 0.18f); cone(0.32f, 0.75f, 6); glPopMatrix(); // wings
    }
    glPopMatrix();
}
static void woodSection(float x0, float z0, float x1, float z1) {
    float L = sqrtf((x1 - x0) * (x1 - x0) + (z1 - z0) * (z1 - z0)); int n = (int)(L / 4 + 0.5f);
    float dx = (x1 - x0) / n, dz = (z1 - z0) / n; bool alongX = fabsf(dx) > 0.01f;
    matWood();
    for (int i = 0; i <= n; i++) {                                    // posts with cone caps
        glPushMatrix(); glTranslatef(x0 + dx * i, 0, z0 + dz * i);
        if (rnd() < 0.25f) glRotatef(rndr(-6, 6), alongX ? 0 : 1, 0, alongX ? 1 : 0);
        col(0.24f, 0.16f, 0.09f); cyl(0.15f, 0.13f, 2.0f, 8);
        glTranslatef(0, 2.0f, 0); cone(0.17f, 0.25f, 8); glPopMatrix();
    }
    for (int i = 0; i < n; i++) for (int r = 0; r < 2; r++) {
        glPushMatrix(); glTranslatef(x0 + dx * (i + 0.5f), r == 0 ? 1.6f : 0.7f, z0 + dz * (i + 0.5f));
        if (rnd() < 0.3f) glRotatef(rndr(-5, 5), alongX ? 0 : 1, 0, alongX ? 1 : 0);
        col(0.22f, 0.15f, 0.09f);
        if (alongX) glScalef(fabsf(dx), 0.1f, 0.06f); else glScalef(0.06f, 0.1f, fabsf(dz));
        glutSolidCube(1.0); glPopMatrix();
    }
}
static void ironFront(float xa, float xb) {
    matIron();
    for (float x = xa; x <= xb + 0.001f; x += 0.25f) {
        box(x, 1.0f, 40, 0.06f, 2.0f, 0.06f);
        glPushMatrix(); glTranslatef(x, 2.0f, 40); cone(0.05f, 0.18f, 4); glPopMatrix();
    }
    box((xa + xb) / 2, 1.8f, 40, xb - xa, 0.08f, 0.08f); box((xa + xb) / 2, 0.5f, 40, xb - xa, 0.08f, 0.08f);
}
static void buildFence() {
    glNewList(base + LG_FENCE, GL_COMPILE); rs = 303;
    woodSection(-40, -40, 40, -40);         // back
    woodSection(40, -40, 40, 40);           // east
    woodSection(-40, -40, -40, 12);         // west (wooden part)
    woodSection(-40, 36, -40, 40);          // west corner
    ironFront(-40, -4.95f); ironFront(4.95f, 40);
    // brick wall along the graveyard
    bigBox(-40.15f, 0, 12, -39.85f, 2.2f, 36, 0.30f, 0.13f, 0.10f, 0.16f, 0.5f);
    matStone(); box(-40, 2.3f, 24, 0.5f, 0.15f, 24.4f);
    // stone pillars: corners, sides of the gate, extra along the front
    float pp[][2] = { {-40,-40},{40,-40},{-40,40},{40,40},{-4.5f,40},{4.5f,40},{-22,40},{22,40},{0,-40},{40,0},{-40,0},{-40,12},{-40,36} };
    for (unsigned i = 0; i < sizeof(pp) / sizeof(pp[0]); i++) pillar(pp[i][0], pp[i][1]);
    gargoyle(-4.5f, 3.4f, 40, 0); gargoyle(4.5f, 3.4f, 40, 0);
    glEndList();
}

static void tombstone(float x, float z, int type, float ry) {
    glPushMatrix(); glTranslatef(x, 0, z); glRotatef(ry, 0, 1, 0);
    glRotatef(rndr(-7, 7), 0, 0, 1); glRotatef(rndr(-5, 5), 1, 0, 0);
    float k = rndr(0.8f, 1.15f); col(0.32f * k, 0.32f * k, 0.34f * k);
    if (type == 0) { box(0, 0.45f, 0, 0.7f, 0.9f, 0.16f); ell(0, 0.9f, 0, 0.35f, 0.2f, 0.08f, 10); }
    else if (type == 1) { box(0, 0.4f, 0, 0.6f, 0.8f, 0.14f); box(0, 0.83f, 0, 0.66f, 0.07f, 0.18f); }
    else { box(0, 0.7f, 0, 0.14f, 1.4f, 0.14f); box(0, 1.0f, 0, 0.7f, 0.14f, 0.14f); }  // cross
    box(0, 0.06f, 0, 0.9f, 0.12f, 0.5f);
    glPopMatrix();
}
static void buildGrave() {
    glNewList(base + LG_GRAVE, GL_COMPILE); rs = 404;
    for (int r = 0; r < 6; r++) for (int c = 0; c < 4; c++) {
        float x = -36 + c * 3.8f + rndr(-0.6f, 0.6f), z = 14.5f + r * 3.6f + rndr(-0.5f, 0.5f);
        tombstone(x, z, (int)(rnd() * 3), rndr(-15, 15));
        solid(x - 0.4f, 0, z - 0.3f, x + 0.4f, 1.0f, z + 0.3f);
    }
    glEndList();
}
static void buildProps() {
    glNewList(base + LG_PROPS, GL_COMPILE); rs = 505;
    // ---- old wagon (R1 / R5) at (13,-6)
    glPushMatrix(); glTranslatef(13, 0, -6); glRotatef(25, 0, 1, 0); matWood();
    box(0, 0.95f, 0, 3.2f, 0.12f, 1.5f);
    for (int s = -1; s <= 1; s += 2) { glPushMatrix(); glTranslatef(0, 1.25f, s * 0.75f); glRotatef(-s * 12, 1, 0, 0); box(0, 0, 0, 3.2f, 0.55f, 0.06f); glPopMatrix(); }
    box(-1.6f, 1.2f, 0, 0.06f, 0.5f, 1.5f); box(2.4f, 0.8f, 0, 1.8f, 0.08f, 0.1f);        // end board + tongue
    for (int wx = -1; wx <= 1; wx += 2) for (int wz = -1; wz <= 1; wz += 2) {
        if (wx == 1 && wz == 1) continue;                                                    // one wheel is missing
        glPushMatrix(); glTranslatef(wx * 1.0f, 0.55f, wz * 0.9f);
        if (wx == -1 && wz == 1) glRotatef(14, 1, 0, 0);                                     // cracked / leaning wheel
        col(0.20f, 0.13f, 0.08f); glutSolidTorus(0.06, 0.5, 8, 18);
        for (int k = 0; k < 4; k++) { glPushMatrix(); glRotatef(k * 45, 0, 0, 1); box(0, 0, 0, 1.0f, 0.06f, 0.06f); glPopMatrix(); }
        sph(0, 0, 0, 0.09f, 8); glPopMatrix();
    }
    glPopMatrix();
    solid(11.4f, 0, -7.4f, 14.6f, 1.6f, -4.6f);
    // ---- pumpkins
    float pk[][3] = { {-4,0,-9},{5.5f,0,-9.5f},{-9,0,-8},{-33,0,15} };
    for (int i = 0; i < 4; i++) {
        col(0.85f, 0.38f, 0.05f); ell(pk[i][0], 0.27f, pk[i][2], 0.36f, 0.27f, 0.36f);
        col(0.2f, 0.28f, 0.08f); glPushMatrix(); glTranslatef(pk[i][0], 0.5f, pk[i][2]); cyl(0.04f, 0.03f, 0.14f, 6); glPopMatrix();
    }
    // ---- lamp posts beside the gate (bulbs are dynamic)
    for (int s = -1; s <= 1; s += 2) {
        matRust(); glPushMatrix(); glTranslatef(s * 7.0f, 0, 38); col(0.25f, 0.12f, 0.06f);
        cyl(0.2f, 0.12f, 0.5f, 8); cyl(0.07f, 0.06f, 3.2f, 8);
        glTranslatef(0, 3.25f, 0); cone(0.42f, 0.35f, 8);
        glPopMatrix();
    }
    // ---- blob "shadows" under the wagon and trees (no real shadows in OpenGL 1.x)
    glDisable(GL_LIGHTING); glColor3f(0.02f, 0.05f, 0.03f);
    glBegin(GL_QUADS);
    for (unsigned i = 0; i < deadTrees.size(); i++) {
        float x = deadTrees[i].x, z = deadTrees[i].z;
        glVertex3f(x - 1, 0.015f, z + 1); glVertex3f(x + 1, 0.015f, z + 1); glVertex3f(x + 1, 0.015f, z - 1); glVertex3f(x - 1, 0.015f, z - 1);
    }
    glEnd(); glEnable(GL_LIGHTING);
    glEndList();
}
static void buildTrees() {
    glNewList(base + LG_TREES, GL_COMPILE);
    for (unsigned i = 0; i < deadTrees.size(); i++) {
        glPushMatrix(); glTranslatef(deadTrees[i].x, 0, deadTrees[i].z); deadTree(deadTrees[i].seed, deadTrees[i].s); glPopMatrix();
        solid(deadTrees[i].x - 0.35f, 0, deadTrees[i].z - 0.35f, deadTrees[i].x + 0.35f, 5, deadTrees[i].z + 0.35f);
    }
    glEndList();
    glNewList(base + LG_PINES, GL_COMPILE);
    for (unsigned i = 0; i < pines.size(); i++) {
        glPushMatrix(); glTranslatef(pines[i].x, 0, pines[i].z); pineTree(pines[i].s); glPopMatrix();
    }
    glEndList();
}

// ---------------------------------------------------------------- railway
static void buildRail() {
    glNewList(base + LG_RAIL, GL_COMPILE); rs = 606;
    for (float z = -170; z < 170; z += 2) {                               // gravel bed + two rails per 2-unit segment
        float zm = z + 1, xm = trackX(zm);
        glPushMatrix(); glTranslatef(xm, 0, zm); glRotatef(trackYaw(zm), 0, 1, 0);
        col(0.13f, 0.13f, 0.14f); box(0, 0.05f, 0, 3.4f, 0.1f, 2.06f);
        matRust(); col(0.24f, 0.17f, 0.13f);
        box(-0.75f, 0.24f, 0, 0.1f, 0.16f, 2.05f); box(0.75f, 0.24f, 0, 0.1f, 0.16f, 2.05f);
        glPopMatrix();
    }
    for (float z = -170; z < 170; z += 0.7f) {                            // sleepers
        glPushMatrix(); glTranslatef(trackX(z), 0.15f, z); glRotatef(trackYaw(z), 0, 1, 0);
        float k = rndr(0.8f, 1.1f); col(0.20f * k, 0.13f * k, 0.08f * k); box(0, 0, 0, 2.4f, 0.1f, 0.26f); glPopMatrix();
    }
    glEndList();
}

// ---------------------------------------------------------------- gate leaf (local +X, hinge at origin)
static void buildLeaf() {
    glNewList(base + LG_LEAF, GL_COMPILE); matIron();
    box(2.0f, 0.35f, 0, 4.0f, 0.12f, 0.08f); box(2.0f, 2.45f, 0, 4.0f, 0.12f, 0.08f);
    box(0.05f, 1.4f, 0, 0.1f, 2.6f, 0.1f); box(3.95f, 1.4f, 0, 0.1f, 2.6f, 0.1f);
    for (float x = 0.3f; x < 3.9f; x += 0.3f) {
        glPushMatrix(); glTranslatef(x, 0.35f, 0); cyl(0.03f, 0.03f, 2.2f, 6);
        glTranslatef(0, 2.2f, 0); cone(0.05f, 0.2f, 5); glPopMatrix();
    }
    matRust(); glPushMatrix(); glTranslatef(2.0f, 1.4f, 0); glRotatef(90, 1, 0, 0); glutSolidTorus(0.03, 0.4, 6, 16); glPopMatrix();   // ring ornament
    glEndList();
}

// ============================================================================
//  MANSION EXTERIOR
// ============================================================================
static void towerAndTurret() {
    // round stone tower, front-left corner (R1)
    float tx = -13.5f, tz = -11.8f;
    solid(tx - 2.7f, 0, tz - 2.7f, tx + 2.7f, 14, tz + 2.7f);
    glPushMatrix(); glTranslatef(tx, 0, tz);
    matStone(); col(0.30f, 0.30f, 0.32f); cyl(2.9f, 2.8f, 14.0f, 20);
    col(0.22f, 0.22f, 0.24f); glPushMatrix(); glTranslatef(0, 13.6f, 0); glRotatef(90, 1, 0, 0); glutSolidTorus(0.2, 2.85, 8, 20); glPopMatrix();
    col(0.08f, 0.09f, 0.11f); glPushMatrix(); glTranslatef(0, 14, 0); cone(3.4f, 6.0f, 20); glPopMatrix();      // slate cone roof
    col(0.06f, 0.06f, 0.07f); glPushMatrix(); glTranslatef(0, 19.8f, 0); cone(0.16f, 3.0f, 6); glPopMatrix();   // spire
    for (int h = 0; h < 3; h++) for (int a = 0; a < 6; a++) {                                                    // arched slits / lit slits
        glPushMatrix(); glRotatef(a * 60.0f + 20.0f * h, 0, 1, 0); glTranslatef(0, 4.5f + h * 3.4f, 2.87f);
        glDisable(GL_LIGHTING);
        if ((a + h) % 4 == 0) glColor3f(1.0f, 0.78f, 0.30f); else glColor3f(0.02f, 0.03f, 0.04f);
        glBegin(GL_QUADS); glVertex3f(-0.22f, -0.6f, 0); glVertex3f(0.22f, -0.6f, 0); glVertex3f(0.22f, 0.5f, 0); glVertex3f(-0.22f, 0.5f, 0); glEnd();
        glBegin(GL_TRIANGLE_FAN); glVertex3f(0, 0.5f, 0); for (int i = 0; i <= 8; i++) { float t = PI * i / 8; glVertex3f(0.22f * cosf(t), 0.5f + 0.22f * sinf(t), 0); } glEnd();
        glEnable(GL_LIGHTING); glPopMatrix();
    }
    glPopMatrix();
    // tilted timber turret, back-right (R5)
    matWood();
    col(0.16f, 0.10f, 0.06); box(9, 11, -27, 3.0f, 4.0f, 3.0f);
    col(0.20f, 0.13f, 0.08f); box(9, 14.75f, -27, 2.6f, 3.5f, 2.6f);
    col(0.11f, 0.07f, 0.04f); box(9, 9.1f, -27, 3.3f, 0.2f, 3.3f); box(9, 13.0f, -27, 3.0f, 0.15f, 3.0f); box(9, 16.5f, -27, 2.8f, 0.15f, 2.8f);
    glPushMatrix(); glTranslatef(9, 16.5f, -27); glRotatef(7, 0, 0, 1);                   // leaning top block
    col(0.17f, 0.11f, 0.07f); box(0, 1.5f, 0, 2.3f, 3.0f, 2.3f);
    col(0.07f, 0.08f, 0.10f); prism(0, 3.0f, 0, 3.0f, 1.8f, 2.9f); prismZ(0, 3.0f, 0, 3.0f, 1.8f, 2.9f);
    glPopMatrix();
    window(9, 14.7f, -27, 0, 0.6f, 0.9f, 1, -1); window(9, 11.5f, -27, 0, 0.6f, 0.9f, 0, -1);
    window(7.48f, 14.7f, -27, -90, 0.6f, 0.9f, 1, -1);
    solid(7.4f, 9, -28.6f, 10.6f, 20, -25.4f);
}

static void dormer(float x, float z, float rot) {
    glPushMatrix(); glTranslatef(x, 0, z); glRotatef(rot, 0, 1, 0);
    float y = 10.9f; matStone(); col(0.24f, 0.24f, 0.26f); box(0, y, 0, 1.4f, 1.4f, 1.2f);
    col(0.08f, 0.09f, 0.11f); prismZ(0, y + 0.7f, 0, 1.5f, 0.9f, 1.7f);
    glPushMatrix(); glTranslatef(0, y, 0.6f); glDisable(GL_LIGHTING);
    if (litHash(x, z, 1) == 0) glColor3f(1.0f, 0.78f, 0.30f); else glColor3f(0.02f, 0.03f, 0.04f);
    glBegin(GL_QUADS); glVertex3f(-0.35f, -0.45f, 0.02f); glVertex3f(0.35f, -0.45f, 0.02f); glVertex3f(0.35f, 0.4f, 0.02f); glVertex3f(-0.35f, 0.4f, 0.02f); glEnd();
    glEnable(GL_LIGHTING); glPopMatrix();
    glPopMatrix();
}

static void buildHouseExt() {
    glNewList(base + LG_EXT, GL_COMPILE); rs = 707;
    const float sr = 0.32f, sg = 0.32f, sb = 0.34f;
    // ---- outer walls (with door gap in the front wall)
    wallBox(-11.8f, 0, -14.2f, -1.2f, WH, -13.8f, sr, sg, sb);
    wallBox(1.2f, 0, -14.2f, 11.8f, WH, -13.8f, sr, sg, sb);
    wallBox(-1.2f, 3.4f, -14.2f, 1.2f, WH, -13.8f, sr, sg, sb);
    wallBox(-11.8f, 0, -30.2f, 11.8f, WH, -29.8f, sr, sg, sb);
    wallBox(-12.2f, 0, -30.2f, -11.8f, WH, -13.8f, sr, sg, sb);
    wallBox(11.8f, 0, -30.2f, 12.2f, WH, -13.8f, sr, sg, sb);
    // darker lower band
    bigBox(-11.8f, 0, -13.8f, -1.2f, 1.4f, -13.7f, 0.2f, 0.2f, 0.22f, 0.15f, 0.7f);
    bigBox(1.2f, 0, -13.8f, 11.8f, 1.4f, -13.7f, 0.2f, 0.2f, 0.22f, 0.15f, 0.7f);
    bigBox(-11.8f, 0, -30.3f, 11.8f, 1.4f, -30.2f, 0.2f, 0.2f, 0.22f, 0.15f, 0.7f);
    bigBox(-12.3f, 0, -30.2f, -12.2f, 1.4f, -13.8f, 0.2f, 0.2f, 0.22f, 0.15f, 0.7f);
    bigBox(12.2f, 0, -30.2f, 12.3f, 1.4f, -13.8f, 0.2f, 0.2f, 0.22f, 0.15f, 0.7f);
    // ---- roof (dark slate), cross gables, dormers, chimneys
    matStone(); col(0.09f, 0.10f, 0.12f);
    prism(0, 8.9f, -22, 25.6f, 3.1f, 17.8f);
    prismZ(-7.5f, 9.0f, -16.4f, 5.2f, 2.7f, 5.0f); prismZ(7.5f, 9.0f, -16.4f, 5.2f, 2.7f, 5.0f);
    dormer(-3, -18, 0); dormer(3, -18, 0); dormer(-3, -26, 180); dormer(3, -26, 180);
    window(-7.5f, 10.0f, -13.8f, 0, 0.8f, 1.0f, 1, -1, true); window(7.5f, 10.0f, -13.8f, 0, 0.8f, 1.0f, 0, -1, true);
    float ch[][3] = { {-8,-23,15.5f},{9.5f,-20,14.0f},{2.5f,-27.5f,16.2f} };
    for (int i = 0; i < 3; i++) {
        col(0.26f, 0.24f, 0.24f); box(ch[i][0], (9.6f + ch[i][2]) / 2, ch[i][1], 1.1f, ch[i][2] - 9.6f, 1.1f);
        col(0.18f, 0.17f, 0.18f); box(ch[i][0], ch[i][2] + 0.12f, ch[i][1], 1.45f, 0.25f, 1.45f);
    }
    // ---- porch, steps, front door frame (R1 / R3)
    matStone();
    for (int s = -1; s <= 1; s += 2) { col(0.30f, 0.30f, 0.32f); glPushMatrix(); glTranslatef(s * 2.3f, 0, -11.5f); cylCap(0.24f, 0.22f, 3.6f, 12); glPopMatrix(); }
    col(0.26f, 0.26f, 0.28f); box(0, 3.85f, -11.4f, 5.3f, 0.5f, 0.6f);
    col(0.02f, 0.02f, 0.03f); glPushMatrix(); glTranslatef(0, 3.4f, -11.05f); glScalef(1, 0.7f, 0.3f); glutSolidTorus(0.1, 1.0, 6, 14); glPopMatrix();   // arch lintel
    col(0.09f, 0.10f, 0.12f); prismZ(0, 4.1f, -12.4f, 3.8f, 1.5f, 5.8f);
    col(0.26f, 0.26f, 0.28f); box(-2.3f, 3.6f, -13.0f, 0.4f, 0.3f, 2.4f); box(2.3f, 3.6f, -13.0f, 0.4f, 0.3f, 2.4f);
    for (int k = 1; k <= 5; k++) { col(0.28f - 0.01f * k, 0.28f - 0.01f * k, 0.30f - 0.01f * k); box(0, 0.03f * k, -11.75f - 0.5f * (k - 1), 4.4f, 0.06f * k, 0.5f); }
    col(0.02f, 0.02f, 0.03f); solid(-2.6f, 0, -12.6f, -2.0f, 4, -12.0f); solid(2.0f, 0, -12.6f, 2.6f, 4, -12.0f);
    // ---- tower + turret
    towerAndTurret();
    // ---- windows: about one third lit, the others dark and cracked
    float wx[] = { -8, 8 }; float wy[] = { 2.4f, 6.6f };
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
        int o = litHash(wx[i], wy[j], -14) == 0 ? 1 : 0; window(wx[i], wy[j], -14, 0, 1.0f, 1.6f, o, wy[j] > 5 ? 2 : -1);
    }
    window(-3.6f, 2.6f, -14, 0, 0.9f, 1.4f, 1, 2, true); window(3.6f, 2.6f, -14, 0, 0.9f, 1.4f, 0, 2, true);
    window(-3.6f, 6.8f, -14, 0, 0.9f, 1.4f, 0, 2, true); window(3.6f, 6.8f, -14, 0, 0.9f, 1.4f, 1, 2, true);
    float sz[] = { -18, -22, -26 };
    for (int j = 0; j < 3; j++) for (int k = 0; k < 2; k++) {
        float y = k == 0 ? 2.4f : 6.6f;
        window(-12, y, sz[j], -90, 1.0f, 1.6f, litHash(-12, y, sz[j]) == 0 ? 1 : 0, k ? 3 : -1);
    }
    float ez[] = { -18, -22, -27.5f };
    for (int j = 0; j < 3; j++) for (int k = 0; k < 2; k++) {
        float y = k == 0 ? 2.4f : 6.6f;
        window(12, y, ez[j], 90, 1.0f, 1.6f, litHash(12, y, ez[j]) == 0 ? 1 : 0, k ? 2 : -1);
    }
    window(-8, 2.4f, -30, 180, 1.0f, 1.6f, 0, -1); window(8, 2.4f, -30, 180, 1.0f, 1.6f, 1, -1);
    window(-8, 6.6f, -30, 180, 1.0f, 1.6f, 1, 2); window(8, 6.6f, -30, 180, 1.0f, 1.6f, 0, 2);
    window(-3.6f, 6.8f, -30, 180, 0.9f, 1.4f, 0, 2, true); window(0, 6.8f, -30, 180, 0.9f, 1.4f, 1, 2, true); window(3.6f, 6.8f, -30, 180, 0.9f, 1.4f, 0, 2, true);
    glEndList();
}

// ============================================================================
//  MANSION INTERIOR (static part)
// ============================================================================
static void portrait(float x, float y, float z, float rotY, float w, float h, float tilt) {
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(rotY, 0, 1, 0); glRotatef(tilt, 0, 0, 1);
    col(0.30f, 0.20f, 0.08f); box(0, 0, 0, w, h, 0.07f);
    col(0.05f, 0.06f, 0.05f); glNormal3f(0, 0, 1);
    glBegin(GL_QUADS); glVertex3f(-w / 2 + 0.08f, -h / 2 + 0.08f, 0.04f); glVertex3f(w / 2 - 0.08f, -h / 2 + 0.08f, 0.04f);
    glVertex3f(w / 2 - 0.08f, h / 2 - 0.08f, 0.04f); glVertex3f(-w / 2 + 0.08f, h / 2 - 0.08f, 0.04f); glEnd();
    col(0.34f, 0.31f, 0.26f);                                  // pale face
    glBegin(GL_TRIANGLE_FAN); glVertex3f(0, h * 0.12f, 0.045f);
    for (int i = 0; i <= 14; i++) { float a = 2 * PI * i / 14; glVertex3f(w * 0.17f * cosf(a), h * 0.12f + h * 0.2f * sinf(a), 0.045f); } glEnd();
    col(0.10f, 0.09f, 0.08f);                                  // shoulders
    glBegin(GL_QUADS); glVertex3f(-w * 0.32f, -h / 2 + 0.08f, 0.045f); glVertex3f(w * 0.32f, -h / 2 + 0.08f, 0.045f);
    glVertex3f(w * 0.2f, -h * 0.12f, 0.045f); glVertex3f(-w * 0.2f, -h * 0.12f, 0.045f); glEnd();
    glPopMatrix();
}
static void bed(float x, float y, float z, float rot, float wid, float len, float fr, float fg, float fb, float mr, float mg, float mb) {
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(rot, 0, 1, 0);
    col(fr, fg, fb); box(0, 0.25f, 0, wid, 0.2f, len); box(0, 0.75f, -len / 2 + 0.05f, wid, 0.9f, 0.1f); box(0, 0.5f, len / 2 - 0.05f, wid, 0.5f, 0.08f);
    for (int a = -1; a <= 1; a += 2) for (int b = -1; b <= 1; b += 2) box(a * (wid / 2 - 0.05f), 0.1f, b * (len / 2 - 0.05f), 0.1f, 0.2f, 0.1f);
    col(mr, mg, mb); box(0, 0.45f, 0.02f, wid - 0.1f, 0.2f, len - 0.15f);
    col(mr * 1.2f, mg * 1.2f, mb * 1.2f); ell(0, 0.62f, -len / 2 + 0.35f, wid * 0.32f, 0.09f, 0.24f);
    glPopMatrix();
}
static void ironBed(float x, float y, float z, float rot) {
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(rot, 0, 1, 0); matIron();
    for (int a = -1; a <= 1; a += 2) {
        glPushMatrix(); glTranslatef(a * 0.5f, 0, -1.02f); cyl(0.03f, 0.03f, 1.1f, 6); glPopMatrix();
        glPushMatrix(); glTranslatef(a * 0.5f, 0, 1.02f); cyl(0.03f, 0.03f, 0.7f, 6); glPopMatrix();
        for (float t = -0.4f; t <= 0.41f; t += 0.2f) { glPushMatrix(); glTranslatef(t, 0.25f, -1.02f); cyl(0.015f, 0.015f, 0.8f, 5); glPopMatrix(); }
    }
    box(0, 1.0f, -1.02f, 1.0f, 0.04f, 0.04f); box(0, 0.65f, 1.02f, 1.0f, 0.04f, 0.04f);
    box(0, 0.35f, 0, 0.96f, 0.03f, 2.0f);
    col(0.36f, 0.34f, 0.30f); box(0, 0.45f, 0, 0.9f, 0.14f, 1.95f);
    col(0.42f, 0.40f, 0.36f); ell(0, 0.58f, -0.75f, 0.28f, 0.07f, 0.2f);
    glPopMatrix();
}
static void dresser(float x, float y, float z) {                    // faces +X
    glPushMatrix(); glTranslatef(x, y, z); matWood(); col(0.22f, 0.14f, 0.08f);
    box(0, 0.5f, 0, 0.9f, 1.0f, 1.6f); box(0, 1.03f, 0, 1.0f, 0.06f, 1.7f);
    col(0.10f, 0.07f, 0.04f);
    for (int i = 0; i < 3; i++) { box(0.46f, 0.2f + i * 0.3f, 0, 0.03f, 0.02f, 1.5f); col(0.6f, 0.5f, 0.25f); sph(0.49f, 0.32f + i * 0.3f - 0.12f, -0.3f, 0.035f, 6); sph(0.49f, 0.32f + i * 0.3f - 0.12f, 0.3f, 0.035f, 6); col(0.10f, 0.07f, 0.04f); }
    glPopMatrix();
}
static void radiator(float x, float y, float z) {
    glPushMatrix(); glTranslatef(x, y, z); matIron(); col(0.16f, 0.15f, 0.14f);
    for (int i = 0; i < 9; i++) box(0, 0.4f, -0.5f + i * 0.125f, 0.12f, 0.6f, 0.06f);
    box(0, 0.72f, 0, 0.14f, 0.05f, 1.15f); box(0, 0.1f, 0, 0.14f, 0.05f, 1.15f);
    glPopMatrix();
}
static void candelabraBase(float x, float y, float z) {
    glPushMatrix(); glTranslatef(x, y, z); col(0.45f, 0.36f, 0.15f);
    cyl(0.12f, 0.06f, 0.08f, 8); cyl(0.03f, 0.03f, 0.5f, 6); box(0, 0.5f, 0, 0.55f, 0.03f, 0.03f);
    col(0.85f, 0.82f, 0.7f); for (int i = -1; i <= 1; i++) { glPushMatrix(); glTranslatef(i * 0.25f, 0.5f, 0); cyl(0.025f, 0.025f, 0.16f, 6); glPopMatrix(); }
    glPopMatrix();
}

static void buildHouseInt() {
    glNewList(base + LG_INT, GL_COMPILE); rs = 808;
    // colours
    const float hr = 0.17f, hg = 0.21f, hb = 0.20f;     // hall walls: dark green-grey (R8)
    // ---- hall side walls (ground part, then upper part with door gaps)
    wallBox(-6.2f, 0, -29.8f, -5.8f, F1, -14.2f, hr, hg, hb, 0.12f);
    wallBox(5.8f, 0, -29.8f, 6.2f, F1, -14.2f, hr, hg, hb, 0.12f);
    wallBox(-6.2f, F1, -29.8f, -5.8f, WH, -24.6f, hr, hg, hb, 0.12f);
    wallBox(-6.2f, F1, -23.4f, -5.8f, WH, -14.2f, hr, hg, hb, 0.12f);
    wallBox(-6.2f, 7.5f, -24.6f, -5.8f, WH, -23.4f, hr, hg, hb, 0.12f);
    wallBox(5.8f, F1, -29.8f, 6.2f, WH, -28.1f, hr, hg, hb, 0.12f);
    wallBox(5.8f, F1, -26.9f, 6.2f, WH, -23.1f, hr, hg, hb, 0.12f);
    wallBox(5.8f, F1, -21.9f, 6.2f, WH, -14.2f, hr, hg, hb, 0.12f);
    wallBox(5.8f, 7.5f, -28.1f, 6.2f, WH, -26.9f, hr, hg, hb, 0.12f);
    wallBox(5.8f, 7.5f, -23.1f, 6.2f, WH, -21.9f, hr, hg, hb, 0.12f);
    wallBox(6.2f, F1, -25.2f, 11.8f, WH, -24.8f, 0.14f, 0.15f, 0.18f, 0.1f);      // partition B | C
    // ---- upstairs floor slabs, ceiling
    bigBox(-5.8f, 4.2f, -29.8f, 5.8f, F1, -21.3f, 0.20f, 0.13f, 0.08f, 0.22f, 0.5f);
    bigBox(-11.8f, 4.2f, -29.8f, -6.2f, F1, -14.2f, 0.20f, 0.13f, 0.08f, 0.22f, 0.5f);
    bigBox(6.2f, 4.2f, -29.8f, 11.8f, F1, -14.2f, 0.20f, 0.13f, 0.08f, 0.22f, 0.5f);
    bigBox(-12.2f, WH, -30.2f, 12.2f, WH + 0.1f, -13.8f, 0.12f, 0.09f, 0.07f, 0.12f, 1.0f);
    for (float z = -16.5f; z > -29; z -= 3) { col(0.10f, 0.07f, 0.05f); box(0, WH - 0.15f, z, 11.6f, 0.3f, 0.3f); }   // ceiling beams
    // ---- ground floor of the hall: alternating dark / lighter planks, some missing or raised
    col(0.02f, 0.02f, 0.02f); quadUp(-5.8f, -29.8f, 5.8f, -14.2f, F0 - 0.02f);
    for (int p = 0; p < 24; p++) for (int s = 0; s < 8; s++) {
        float x0 = -5.8f + p * 0.4833f, z0 = -29.8f + s * 1.95f; int r = (int)(rnd() * 40);
        if (r == 0) continue;                                                       // missing plank
        float k = (p % 2 ? 0.75f : 1.05f) * rndr(0.85f, 1.1f); float dy = (r == 1) ? 0.06f : 0.0f;
        col(0.22f * k, 0.14f * k, 0.08f * k);
        quadUp(x0 + 0.02f, z0, x0 + 0.46f, z0 + 1.95f, F0 + dy);
    }
    // ---- linings (dark green-grey wallpaper, peeling patches)
    lining(-5.8f, F0, -29.8f, 5.8f, WH, -29.76f, hr * 1.1f, hg * 1.1f, hb * 1.1f);
    for (int i = 0; i < 9; i++) {                                                   // peeling patches
        float px0 = rndr(-5.5f, 4.5f), py0 = rndr(0.8f, 7.5f); col(0.10f, 0.12f, 0.11f); glNormal3f(0, 0, 1);
        glBegin(GL_QUADS); glVertex3f(px0, py0, -29.75f); glVertex3f(px0 + rndr(0.5f, 1.2f), py0 + 0.1f, -29.75f);
        glVertex3f(px0 + 0.8f, py0 + rndr(0.6f, 1.4f), -29.75f); glVertex3f(px0 + 0.1f, py0 + 0.5f, -29.75f); glEnd();
    }
    // ---- staircase: 14 stacked cubes (rise 0.3, run 0.45, width 4) in the west half of the hall
    for (int i = 0; i < 14; i++) {
        float top = F0 + 0.3f * (i + 1), z1 = -15.0f - 0.45f * i, z0 = z1 - 0.45f; float k = (i % 2) ? 0.9f : 1.1f;
        bigBox(-5.8f, 0, z0, -2.0f, top, z1, 0.20f * k, 0.13f * k, 0.08f * k, 0.10f, 0.5f);
    }
    // ---- balustrade along the stairs (posts every step + sloped handrail), newel posts
    matWood(); col(0.16f, 0.10f, 0.06f);
    for (int i = 0; i < 14; i++) {
        float top = F0 + 0.3f * (i + 1), z = -15.0f - 0.45f * i - 0.22f;
        glPushMatrix(); glTranslatef(-2.05f, top, z); cyl(0.03f, 0.03f, 0.9f, 6); glPopMatrix();
    }
    { float rise = F1 - F0, run = 6.3f, len = sqrtf(rise * rise + run * run);
      glPushMatrix(); glTranslatef(-2.05f, F0 + rise / 2 + 1.0f, -15.0f - run / 2); glRotatef(atan2f(rise, run) * DEG, 1, 0, 0);
      box(0, 0, 0, 0.1f, 0.09f, len); glPopMatrix(); }
    float nz[] = { -15.0f, -21.3f }; float ny[] = { F0, F1 };
    for (int i = 0; i < 2; i++) { col(0.13f, 0.08f, 0.05f); box(-2.05f, ny[i] + 0.65f, nz[i], 0.2f, 1.3f, 0.2f); col(0.2f, 0.13f, 0.08f); sph(-2.05f, ny[i] + 1.4f, nz[i], 0.14f, 8); }
    solid(-2.15f, F0, -21.4f, -1.95f, F1 + 1.2f, -14.9f);
    // ---- gallery rail (front edge of the upstairs gallery)
    matWood(); col(0.16f, 0.10f, 0.06f);
    for (float x = -1.9f; x <= 6.0f; x += 0.3f) { glPushMatrix(); glTranslatef(x, F1, -21.3f); cyl(0.03f, 0.03f, 0.9f, 6); glPopMatrix(); }
    box(2.0f, F1 + 0.95f, -21.3f, 8.0f, 0.09f, 0.11f);
    col(0.13f, 0.08f, 0.05f); box(6.0f, F1 + 0.65f, -21.3f, 0.2f, 1.3f, 0.2f); sph(6.0f, F1 + 1.4f, -21.3f, 0.14f, 8);
    solid(-2.1f, F1, -21.45f, 6.1f, F1 + 1.2f, -21.15f);
    // ---- side tables under candelabras
    col(0.18f, 0.11f, 0.07f); box(5.0f, F0 + 0.4f, -15.2f, 1.2f, 0.8f, 0.5f); box(5.0f, F0 + 0.4f, -27.5f, 1.2f, 0.8f, 0.5f);
    solid(4.4f, F0, -15.5f, 5.6f, F0 + 0.8f, -14.9f); solid(4.4f, F0, -27.8f, 5.6f, F0 + 0.8f, -27.2f);
    // ---- portraits (six, one crooked)
    portrait(-5.75f, 2.9f, -17.6f, 90, 0.9f, 1.3f, 0);
    portrait(-5.75f, 4.0f, -20.3f, 90, 0.9f, 1.3f, 0);
    portrait(5.75f, 2.7f, -16.6f, -90, 0.9f, 1.3f, 0);
    portrait(5.75f, 2.7f, -19.6f, -90, 0.9f, 1.3f, 7);            // the crooked one
    portrait(-2.2f, 6.9f, -29.72f, 0, 1.1f, 1.5f, 0);
    portrait(2.2f, 6.9f, -29.72f, 0, 1.1f, 1.5f, 0);

    // ================================================================= BEDROOM A (child room, R9)
    lining(-11.8f, F1, -29.8f, -6.2f, WH, -29.76f, 0.20f, 0.16f, 0.12f);
    lining(-11.8f, F1, -14.24f, -6.2f, WH, -14.2f, 0.20f, 0.16f, 0.12f);
    for (int k = 0; k < 9; k++) { float s = rndr(0.8f, 1.15f); col(0.22f * s, 0.15f * s, 0.10f * s); box(-11.76f, F1 + 0.25f + k * 0.5f, -22, 0.04f, 0.46f, 15.6f); }  // plank wall
    bed(-9.5f, F1, -28.7f, 0, 1.6f, 2.2f, 0.18f, 0.12f, 0.07f, 0.40f, 0.36f, 0.30f);
    solid(-10.4f, F1, -29.8f, -8.6f, F1 + 1.0f, -27.5f);
    dresser(-11.35f, F1, -24.0f); solid(-11.8f, F1, -24.9f, -10.85f, F1 + 1.1f, -23.1f);
    radiator(-11.6f, F1, -18.0f);
    // rug + open book on the floor
    col(0.20f, 0.06f, 0.06f); quadUp(-10.2f, -21.8f, -7.6f, -19.6f, F1 + 0.012f);
    col(0.55f, 0.52f, 0.42f); quadUp(-9.2f, -16.7f, -8.7f, -16.4f, F1 + 0.02f); col(0.5f, 0.47f, 0.38f); quadUp(-8.7f, -16.7f, -8.2f, -16.4f, F1 + 0.02f);
    // hanging lamp (bulb is emissive, drawn dynamically)
    col(0.10f, 0.10f, 0.10f); glPushMatrix(); glTranslatef(-9, WH - 1.3f, -22); cyl(0.01f, 0.01f, 1.3f, 4); glPopMatrix();
    col(0.15f, 0.25f, 0.12f); glPushMatrix(); glTranslatef(-9, WH - 1.5f, -22); cone(0.38f, 0.28f, 12); glPopMatrix();

    // ================================================================= BEDROOM B (draped room, R10)
    lining(6.2f, F1, -29.8f, 11.8f, WH, -29.76f, 0.09f, 0.11f, 0.15f);
    lining(11.76f, F1, -29.8f, 11.8f, WH, -25.2f, 0.09f, 0.11f, 0.15f);
    lining(6.2f, F1, -25.24f, 11.8f, WH, -25.2f, 0.09f, 0.11f, 0.15f);
    bed(10.0f, F1, -28.7f, 0, 2.0f, 2.2f, 0.06f, 0.05f, 0.06f, 0.08f, 0.09f, 0.12f);
    solid(9.0f, F1, -29.8f, 11.0f, F1 + 1.0f, -27.5f);
    col(0.15f, 0.10f, 0.07f); box(8.2f, F1 + 0.3f, -29.3f, 0.8f, 0.6f, 0.7f); solid(7.8f, F1, -29.65f, 8.6f, F1 + 0.6f, -28.95f);
    col(0.85f, 0.82f, 0.7f); glPushMatrix(); glTranslatef(8.2f, F1 + 0.6f, -29.3f); cyl(0.04f, 0.04f, 0.18f, 8); glPopMatrix();
    for (int i = 0; i < 6; i++) { col(0.05f, 0.05f, 0.06f); float x = rndr(6.8f, 10.8f), z = rndr(-27, -25.6f);   // rags
        glNormal3f(0, 1, 0); glBegin(GL_QUADS); glVertex3f(x, F1 + 0.015f, z); glVertex3f(x + rndr(0.4f, 1.0f), F1 + 0.015f, z + 0.1f); glVertex3f(x + 0.6f, F1 + 0.015f, z + rndr(0.4f, 0.8f)); glVertex3f(x - 0.1f, F1 + 0.015f, z + 0.5f); glEnd(); }

    // ================================================================= BEDROOM C (abandoned, R11)
    lining(6.2f, F1, -14.24f, 11.8f, WH, -14.2f, 0.24f, 0.22f, 0.18f);
    lining(6.2f, F1, -24.8f, 11.8f, WH, -24.76f, 0.24f, 0.22f, 0.18f);
    for (float z = -24.6f; z < -14.3f; z += 0.5f) {                                       // striped peeling wallpaper on the east wall
        int st = ((int)((z + 30) / 0.5f)) % 2; col(st ? 0.34f : 0.26f, st ? 0.30f : 0.23f, st ? 0.22f : 0.17f);
        box(11.74f, F1 + 2.2f, z + 0.25f, 0.04f, 4.4f, 0.48f);
    }
    ironBed(10.7f, F1, -23.3f, -90); solid(9.6f, F1, -23.8f, 11.8f, F1 + 1.0f, -22.8f);
    ironBed(10.7f, F1, -19.5f, -90); solid(9.6f, F1, -20.0f, 11.8f, F1 + 1.0f, -19.0f);
    matWood(); col(0.13f, 0.09f, 0.06f); box(11.4f, F1 + 1.1f, -15.9f, 0.7f, 2.2f, 1.6f);
    col(0.08f, 0.05f, 0.03f); box(11.03f, F1 + 1.1f, -15.9f, 0.03f, 2.0f, 0.02f); col(0.6f, 0.5f, 0.25f); sph(11.02f, F1 + 1.1f, -15.7f, 0.04f, 6); sph(11.02f, F1 + 1.1f, -16.1f, 0.04f, 6);
    solid(11.05f, F1, -16.7f, 11.8f, F1 + 2.2f, -15.1f);
    float hl[][2] = { {8.5f,-17.5f},{9.2f,-15.8f},{7.2f,-19.6f} };                        // holes in the floor
    for (int i = 0; i < 3; i++) { col(0.01f, 0.01f, 0.01f); quadUp(hl[i][0] - 0.6f, hl[i][1] - 0.5f, hl[i][0] + 0.6f, hl[i][1] + 0.5f, F1 + 0.016f); }
    col(0.32f, 0.30f, 0.27f); quadUp(7.0f, -23.0f, 9.5f, -21.0f, F1 + 0.011f);            // dust on the floor
    glEndList();
}

// ============================================================================
//  DYNAMIC OBJECTS
// ============================================================================
static void drawDoorLeaf(const Door& d) {
    glPushMatrix(); glTranslatef(d.hx, d.hy, d.hz); glRotatef(d.baseRot + d.sgn * d.open * d.maxA, 0, 1, 0);
    matWood(); col(0.22f, 0.14f, 0.08f); box(d.w / 2, d.h / 2, 0, d.w, d.h, 0.1f);
    col(0.15f, 0.09f, 0.05f);
    for (int s = -1; s <= 1; s += 2) { box(d.w / 2, d.h * 0.74f, s * 0.06f, d.w * 0.7f, d.h * 0.28f, 0.03f); box(d.w / 2, d.h * 0.28f, s * 0.06f, d.w * 0.7f, d.h * 0.34f, 0.03f); }
    col(0.55f, 0.42f, 0.2f); sph(d.w - 0.2f, d.h * 0.45f, 0.1f, 0.06f, 8); sph(d.w - 0.2f, d.h * 0.45f, -0.1f, 0.06f, 8);
    glPopMatrix();
}
static void drawGate() {
    float a = gateOpen * 70.0f;
    glPushMatrix(); glTranslatef(-4, 0, 40); glRotatef(a, 0, 1, 0); glCallList(base + LG_LEAF); glPopMatrix();
    glPushMatrix(); glTranslatef(4, 0, 40); glRotatef(180 - a, 0, 1, 0); glCallList(base + LG_LEAF); glPopMatrix();
}
static void drawChandelier() {
    glPushMatrix(); glTranslatef(2.0f, WH, -17.6f); glRotatef(3.0f * sinf(1.1f * simTime), 0, 0, 1);
    matIron(); col(0.16f, 0.13f, 0.09f); glPushMatrix(); glTranslatef(0, -1.7f, 0); cyl(0.03f, 0.03f, 1.7f, 6); glPopMatrix();
    glTranslatef(0, -1.7f, 0); col(0.30f, 0.24f, 0.10f);
    glPushMatrix(); glRotatef(90, 1, 0, 0); glutSolidTorus(0.05, 0.9, 8, 22); glPopMatrix();
    sph(0, 0, 0, 0.16f, 8);
    for (int i = 0; i < 6; i++) {
        glPushMatrix(); glRotatef(i * 60.0f, 0, 1, 0);
        glPushMatrix(); glRotatef(90, 0, 1, 0); gluCylinder(Q, 0.03, 0.03, 0.9, 6, 1); glPopMatrix();
        glTranslatef(0.9f, 0, 0); col(0.85f, 0.82f, 0.7f); cyl(0.04f, 0.04f, 0.24f, 6);
        flame(0, 0.25f, 0, 1.5f, (float)i); glPopMatrix();
    }
    glPopMatrix();
}
static void drawCandelabra(float x, float y, float z, float ph) {
    candelabraBase(x, y, z);
    for (int i = -1; i <= 1; i++) flame(x + i * 0.25f, y + 0.66f, z, 1.0f, ph + i);
}
static void drape(float x, float y, float z, float w, float h, float rot, float ph, float r, float g, float b, float tear) {
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(rot, 0, 1, 0); col(r, g, b); glNormal3f(0, 0, 1);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 12; i++) {
        float u = (float)i / 12, xs = -w / 2 + u * w, fold = 0.06f * sinf(u * 20 + ph), sw = 0.10f * sinf(simTime * 0.8f + u * 5 + ph);
        float bot = -h * (1.0f - tear * 0.3f * sinf(u * 9 + ph) * sinf(u * 9 + ph));
        glVertex3f(xs, 0, fold); glVertex3f(xs + sw, bot, fold + sw * 0.5f);
    }
    glEnd(); glPopMatrix();
}
static void rockingChair() {
    glPushMatrix(); glTranslatef(-8.7f, F1, -19.0f); glRotatef(90, 0, 1, 0);        // faces the room centre
    glRotatef(5.0f * sinf(1.3f * simTime), 1, 0, 0); matWood(); col(0.22f, 0.14f, 0.08f);
    box(0, 0.5f, 0, 0.5f, 0.05f, 0.5f); box(0, 0.9f, -0.22f, 0.5f, 0.7f, 0.05f);
    for (int a = -1; a <= 1; a += 2) {
        box(a * 0.22f, 0.25f, 0.2f, 0.04f, 0.5f, 0.04f); box(a * 0.22f, 0.25f, -0.2f, 0.04f, 0.5f, 0.04f);
        box(a * 0.22f, 0.7f, 0, 0.04f, 0.04f, 0.4f);
        for (int s = -3; s <= 3; s++) box(a * 0.24f, 0.03f + 0.012f * s * s * 0.3f, s * 0.15f, 0.05f, 0.05f, 0.17f);   // curved runner
    }
    glPopMatrix();
}
static void drawLampBulbs() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f * fLamp, 0.85f * fLamp, 0.45f * fLamp); sph(-7, 3.5f, 38, 0.22f, 10);
    glColor3f(1.0f, 0.85f, 0.45f); sph(7, 3.5f, 38, 0.22f, 10);
    glColor3f(1.0f, 0.9f, 0.6f); sph(-9, WH - 1.65f, -22, 0.09f, 8);               // bedroom A lamp
    glEnable(GL_LIGHTING);
}
static void drawCandleB() {
    flame(8.2f, F1 + 0.78f, -29.3f, 1.4f, 5.0f);
}

// ---- train
static void wheel(float x, float y, float z, float ang) {
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(90, 0, 1, 0); glRotatef(ang, 0, 0, 1); glTranslatef(0, 0, -0.08f);
    col(0.10f, 0.08f, 0.07f); gluCylinder(Q, 0.45, 0.45, 0.16, 14, 1);
    gluDisk(Q, 0, 0.45, 14, 1); glTranslatef(0, 0, 0.16f); gluDisk(Q, 0, 0.45, 14, 1);
    col(0.30f, 0.15f, 0.08f); box(0, 0, 0.02f, 0.9f, 0.07f, 0.06f); box(0, 0, 0.02f, 0.07f, 0.9f, 0.06f);
    glPopMatrix();
}
static float trainHeadZ() { return -160.0f + 320.0f * (fmodf(simTime, 30.0f) / 30.0f); }
static void drawTrain() {
    float zh = trainHeadZ(), wa = simTime * 1273.0f;
    float zc[3] = { zh - 4.0f, zh - 13.0f, zh - 22.0f };
    for (int c = 0; c < 3; c++) {
        glPushMatrix(); glTranslatef(trackX(zc[c]), 0, zc[c]); glRotatef(trackYaw(zc[c]), 0, 1, 0);
        matRust(); col(0.42f, 0.20f, 0.09f);
        box(0, 1.0f, 0, 2.4f, 0.35f, c == 0 ? 8.0f : 8.4f);                       // chassis
        if (c == 0) {                                                               // locomotive
            col(0.44f, 0.21f, 0.09f); box(0, 2.1f, 0.5f, 2.6f, 2.4f, 5.0f);
            box(0, 1.6f, 3.6f, 2.2f, 1.6f, 1.6f);                                   // nose
            col(0.38f, 0.17f, 0.08f); box(0, 3.0f, -2.5f, 2.8f, 3.2f, 2.6f);        // cab
            col(0.16f, 0.09f, 0.06f); prism(0, 4.6f, -2.5f, 2.9f, 0.6f, 2.8f);
            col(0.85f, 0.55f, 0.15f); glNormal3f(1, 0, 0);                          // orange patches
            for (int s = -1; s <= 1; s += 2) { glBegin(GL_QUADS); float x = s * 1.32f;
                glVertex3f(x, 1.6f, 1.0f); glVertex3f(x, 1.6f, 2.4f); glVertex3f(x, 2.4f, 2.4f); glVertex3f(x, 2.4f, 1.0f);
                glVertex3f(x, 2.5f, -0.6f); glVertex3f(x, 2.5f, 0.3f); glVertex3f(x, 3.1f, 0.3f); glVertex3f(x, 3.1f, -0.6f); glEnd(); }
            glDisable(GL_LIGHTING); glColor3f(0.03f, 0.04f, 0.05f);                 // cab windows
            for (int s = -1; s <= 1; s += 2) { glBegin(GL_QUADS); float x = s * 1.42f;
                glVertex3f(x, 3.1f, -3.3f); glVertex3f(x, 3.1f, -1.7f); glVertex3f(x, 4.1f, -1.7f); glVertex3f(x, 4.1f, -3.3f); glEnd(); }
            glColor3f(1.0f, 0.95f, 0.7f); sph(0, 2.0f, 4.45f, 0.28f, 10);          // headlight
            glEnable(GL_LIGHTING);
            col(0.20f, 0.10f, 0.06f); box(0, 3.9f, 1.0f, 0.5f, 0.9f, 0.5f);         // chimney
            float wz[4] = { -2.7f, -0.9f, 1.3f, 3.0f };
            for (int i = 0; i < 4; i++) for (int s = -1; s <= 1; s += 2) wheel(s * 1.1f, 0.75f, wz[i], wa * (i == 3 ? 0.7f : 1));
            col(0.25f, 0.14f, 0.10f); for (int s = -1; s <= 1; s += 2) box(s * 1.28f, 0.75f, -0.1f, 0.05f, 0.09f, 4.0f);
        } else {                                                                    // carriages
            col(0.36f, 0.16f, 0.09f); box(0, 2.4f, 0, 2.8f, 2.8f, 8.2f);
            col(0.20f, 0.10f, 0.07f); box(0, 3.9f, 0, 2.6f, 0.2f, 8.2f);
            glDisable(GL_LIGHTING); glColor3f(0.03f, 0.04f, 0.05f);
            for (int s = -1; s <= 1; s += 2) for (int w = 0; w < 4; w++) { float x = s * 1.42f, z = -3.0f + w * 2.0f; glBegin(GL_QUADS);
                glVertex3f(x, 2.6f, z - 0.6f); glVertex3f(x, 2.6f, z + 0.6f); glVertex3f(x, 3.5f, z + 0.6f); glVertex3f(x, 3.5f, z - 0.6f); glEnd(); }
            glEnable(GL_LIGHTING);
            for (int i = 0; i < 4; i++) for (int s = -1; s <= 1; s += 2) wheel(s * 1.1f, 0.75f, (i < 2 ? -3.4f : 3.4f) + (i % 2) * 0.9f, wa);
        }
        glPopMatrix();
    }
}

// ---- creatures
static void winged(float scale, float flapAmp, float flapSpeed, float ph, float r, float g, float b) {
    glPushMatrix(); glScalef(scale, scale, scale); col(r, g, b);
    ell(0, 0, 0, 0.18f, 0.09f, 0.10f, 8); sph(0.17f, 0.03f, 0, 0.07f, 6);
    float a = flapAmp * sinf(flapSpeed * simTime + ph); glNormal3f(0, 1, 0);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix(); glRotatef(s * a, 1, 0, 0);
        glBegin(GL_TRIANGLES); glVertex3f(0.10f, 0, 0); glVertex3f(-0.15f, 0, 0); glVertex3f(-0.02f, 0, s * 0.7f); glEnd();
        glPopMatrix();
    }
    glPopMatrix();
}
static void drawBatsCrows() {
    for (int i = 0; i < 5; i++) {                                                   // bats round the round tower
        float th = simTime * (1.3f + 0.15f * i) + i * 1.3f, rad = 4.5f + 0.8f * i;
        glPushMatrix(); glTranslatef(-13.5f + rad * cosf(th), 14.5f + 1.2f * sinf(simTime * 2 + i), -11.8f + rad * sinf(th));
        glRotatef(-(th * DEG) - 90, 0, 1, 0); winged(1.0f, 35, 12, i, 0.04f, 0.04f, 0.05f); glPopMatrix();
    }
    for (int i = 0; i < 6; i++) {                                                   // crows circle over the trees (R5)
        float th = simTime * (0.35f + 0.03f * i) + i * 1.05f, rad = 15 + 3 * i;
        glPushMatrix(); glTranslatef(8 + rad * cosf(th), 12.5f + 1.5f * sinf(simTime * 0.7f + i), 12 + rad * sinf(th));
        glRotatef(-(th * DEG) - 90, 0, 1, 0); winged(2.4f, 22, 4.5f, i, 0.02f, 0.02f, 0.025f); glPopMatrix();
    }
}
static void ghostPos(float& x, float& y, float& z) {
    x = -30 + 6.0f * sinf(0.25f * simTime); z = 23 + 6.0f * cosf(0.2f * simTime); y = 1.5f + 0.3f * sinf(simTime);
}
static void drawGhost(float cx, float cz) {
    float x, y, z; ghostPos(x, y, z); float a = 0.35f + 0.15f * sinf(0.7f * simTime);
    glPushMatrix(); glTranslatef(x, y, z); glRotatef(atan2f(cx - x, cz - z) * DEG, 0, 1, 0);
    glDisable(GL_LIGHTING); glColor4f(0.75f, 0.85f, 0.95f, a);
    glPushMatrix(); glTranslatef(0, -1.0f, 0); cone(0.75f, 1.9f, 14); glPopMatrix();
    sph(0, 1.1f, 0, 0.36f, 14);
    for (int s = -1; s <= 1; s += 2) { glPushMatrix(); glTranslatef(s * 0.3f, 0.5f, 0.1f); glRotatef(s * 70, 0, 0, 1); glRotatef(-30, 1, 0, 0); cone(0.12f, 0.9f, 6); glPopMatrix(); }
    glColor4f(0.02f, 0.03f, 0.05f, 0.9f);
    sph(-0.13f, 1.17f, 0.31f, 0.06f, 8); sph(0.13f, 1.17f, 0.31f, 0.06f, 8); ell(0, 0.98f, 0.33f, 0.07f, 0.11f, 0.04f, 8);
    glEnable(GL_LIGHTING); glPopMatrix();
}
struct Beam { float wx, wy, wz, dir; };
static Beam beams[] = { {-12,6.6f,-18,1},{-12,6.6f,-22,1},{-12,6.6f,-26,1},{12,6.6f,-27.5f,-1},{12,6.6f,-18,-1},{12,6.6f,-22,-1} };
static void drawBeams() {
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    for (unsigned i = 0; i < sizeof(beams) / sizeof(beams[0]); i++) {
        Beam& b = beams[i]; float w = 0.55f, xi = b.wx + b.dir * 0.32f, xf = b.wx + b.dir * 4.2f, dr = 0.9f;
        float f = 0.8f + 0.2f * sinf(simTime * 0.5f + i);
        glColor4f(0.55f, 0.68f, 0.95f, 0.16f * f); glVertex3f(xi, b.wy + 0.8f, b.wz - w); glVertex3f(xi, b.wy + 0.8f, b.wz + w);
        glColor4f(0.55f, 0.68f, 0.95f, 0.05f * f); glVertex3f(xf, F1 + 0.03f, b.wz + w + dr); glVertex3f(xf, F1 + 0.03f, b.wz - w + dr);
    }
    glEnd(); glEnable(GL_LIGHTING);
}
static void drawFogSheets() {
    glDisable(GL_LIGHTING); glDisable(GL_FOG);
    for (int i = 0; i < 7; i++) {
        float x = 30 * sinf(0.05f * simTime + i * 2.1f), z = -10 + 35 * cosf(0.04f * simTime + i * 1.3f) + i * 4, y = 0.45f + 0.15f * i;
        glColor4f(0.30f, 0.42f, 0.42f, 0.055f); glBegin(GL_QUADS);
        glVertex3f(x - 16, y, z - 16); glVertex3f(x + 16, y, z - 16); glVertex3f(x + 16, y, z + 16); glVertex3f(x - 16, y, z + 16); glEnd();
    }
    glEnable(GL_FOG); glEnable(GL_LIGHTING);
}

// ============================================================================
//  SKY
// ============================================================================
static float stars[200][3];
static void drawSky(float cx, float cy, float cz) {
    glDisable(GL_LIGHTING); glDisable(GL_FOG); glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE);
    glPushMatrix(); glTranslatef(cx, cy, cz);
    float R = 190, f = flashV;
    float hor[3] = { fogBase[0] + f * 0.5f, fogBase[1] + f * 0.5f, fogBase[2] + f * 0.55f };
    float mid[3] = { 0.03f + f * 0.4f, 0.05f + f * 0.42f, 0.09f + f * 0.5f };
    float top[3] = { 0.01f + f * 0.3f, 0.015f + f * 0.32f, 0.04f + f * 0.4f };
    for (int s = 0; s < 4; s++) {
        glPushMatrix(); glRotatef(s * 90.0f, 0, 1, 0);
        glBegin(GL_QUADS);
        glColor3fv(hor); glVertex3f(-R, -40, -R); glVertex3f(R, -40, -R);
        glColor3fv(mid); glVertex3f(R, 60, -R); glVertex3f(-R, 60, -R);
        glColor3fv(mid); glVertex3f(-R, 60, -R); glVertex3f(R, 60, -R);
        glColor3fv(top); glVertex3f(R, 190, -R); glVertex3f(-R, 190, -R);
        glEnd(); glPopMatrix();
    }
    glColor3fv(top); glBegin(GL_QUADS); glVertex3f(-R, 190, -R); glVertex3f(R, 190, -R); glVertex3f(R, 190, R); glVertex3f(-R, 190, R); glEnd();
    // stars
    glPointSize(2.0f); glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++) { float t = 0.55f + 0.45f * sinf(simTime * 1.5f + i); glColor3f(0.75f * t, 0.8f * t, 0.9f * t); glVertex3fv(stars[i]); }
    glEnd();
    // moon (near white) with a soft halo
    float md[3] = { -0.30f, 0.36f, -0.88f }; float ml = sqrtf(md[0] * md[0] + md[1] * md[1] + md[2] * md[2]);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix(); glTranslatef(md[0] / ml * 150, md[1] / ml * 150, md[2] / ml * 150);
    glColor4f(0.5f, 0.6f, 0.8f, 0.05f); glutSolidSphere(22, 40, 40);
    glColor4f(0.6f, 0.7f, 0.9f, 0.08f); glutSolidSphere(15, 40, 40);
    glColor3f(0.96f, 0.96f, 0.90f); glutSolidSphere(9, 40, 40);
    glColor3f(0.78f, 0.78f, 0.74f); glPushMatrix(); glTranslatef(-2.5f, 2.0f, 8.0f); glutSolidSphere(1.6, 8, 8); glTranslatef(4.0f, -3.5f, 0.3f); glutSolidSphere(1.1, 8, 8); glPopMatrix();
    glPopMatrix();
    // slow drifting cloud layers
    for (int i = 0; i < 9; i++) {
        float ox = fmodf(simTime * (1.2f + 0.2f * i) + i * 90, 400.0f) - 200, oz = -160 + i * 40, y = 70 + 8 * (i % 3);
        glColor4f(0.10f + f * 0.5f, 0.13f + f * 0.5f, 0.16f + f * 0.5f, 0.30f);
        glBegin(GL_QUADS); glVertex3f(ox - 90, y, oz - 30); glVertex3f(ox + 90, y, oz - 30); glVertex3f(ox + 90, y, oz + 30); glVertex3f(ox - 90, y, oz + 30); glEnd();
    }
    glDisable(GL_BLEND);
    glPopMatrix();
    glDepthMask(GL_TRUE); glEnable(GL_DEPTH_TEST); glEnable(GL_FOG); glEnable(GL_LIGHTING);
}

// ============================================================================
//  LIGHTS  (8 fixed-function lights, only those near the player are enabled)
// ============================================================================
static void setPointLight(GLenum L, float x, float y, float z, float r, float g, float b, float c, float l, float q) {
    GLfloat pos[4] = { x, y, z, 1 }, dif[4] = { r, g, b, 1 }, amb[4] = { 0, 0, 0, 1 }, spc[4] = { r * 0.3f, g * 0.3f, b * 0.3f, 1 };
    glLightfv(L, GL_POSITION, pos); glLightfv(L, GL_DIFFUSE, dif); glLightfv(L, GL_AMBIENT, amb); glLightfv(L, GL_SPECULAR, spc);
    glLightf(L, GL_CONSTANT_ATTENUATION, c); glLightf(L, GL_LINEAR_ATTENUATION, l); glLightf(L, GL_QUADRATIC_ATTENUATION, q);
    glLightf(L, GL_SPOT_CUTOFF, 180.0f);
}
static bool closeTo(float x, float y, float z, float R, float ex, float ey, float ez) {
    float dx = x - ex, dy = y - ey, dz = z - ez; return dx * dx + dy * dy + dz * dz < R * R;
}
static void setupWorldLights(float ex, float ey, float ez) {
    // GL_LIGHT0 moon: directional (w = 0), cold and dim
    GLfloat moonPos[] = { -0.4f, 1.0f, -0.3f, 0.0f };
    float mk = (1.0f - 0.85f * indoorAmt) + flashV * 1.5f;
    GLfloat moonDiff[] = { 0.30f * mk, 0.38f * mk, 0.55f * mk, 1.0f }, zero[] = { 0, 0, 0, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, moonPos); glLightfv(GL_LIGHT0, GL_DIFFUSE, moonDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, zero); glLightfv(GL_LIGHT0, GL_SPECULAR, moonDiff); glEnable(GL_LIGHT0);

    float cf = fChand;   // GL_LIGHT2 chandelier
    setPointLight(GL_LIGHT2, 2.0f + 0.1f * sinf(1.1f * simTime), WH - 2.4f, -17.6f, 1.5f * cf, 1.0f * cf, 0.45f * cf, 0.5f, 0.04f, 0.018f);
    if (closeTo(2, 6, -17.6f, 20, ex, ey, ez)) glEnable(GL_LIGHT2); else glDisable(GL_LIGHT2);
    // GL_LIGHT3 bedroom candle (strong flicker)
    setPointLight(GL_LIGHT3, 8.2f, F1 + 1.0f, -29.3f, 1.4f * fCandle, 0.65f * fCandle, 0.18f * fCandle, 0.4f, 0.10f, 0.08f);
    if (closeTo(8.2f, 6, -29.3f, 16, ex, ey, ez)) glEnable(GL_LIGHT3); else glDisable(GL_LIGHT3);
    // GL_LIGHT4 lamp post near the gate
    setPointLight(GL_LIGHT4, -7, 3.4f, 38, 1.3f * fLamp, 1.0f * fLamp, 0.45f * fLamp, 0.6f, 0.04f, 0.012f);
    if (closeTo(-7, 3, 38, 45, ex, ey, ez)) glEnable(GL_LIGHT4); else glDisable(GL_LIGHT4);
    // GL_LIGHT5 train headlight: spotlight that moves with the train
    { float zh = trainHeadZ(), zc = zh - 4.0f, yw = trackYaw(zc) / DEG;
      GLfloat pos[4] = { trackX(zc) + sinf(yw) * 4.5f, 2.0f, zc + cosf(yw) * 4.5f, 1 }, dir[3] = { sinf(yw), -0.04f, cosf(yw) };
      GLfloat dif[4] = { 1.5f, 1.4f, 1.0f, 1 };
      glLightfv(GL_LIGHT5, GL_POSITION, pos); glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, dir); glLightfv(GL_LIGHT5, GL_DIFFUSE, dif); glLightfv(GL_LIGHT5, GL_AMBIENT, zero);
      glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 30.0f); glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 6.0f);
      glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, 1.0f); glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.02f); glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.0015f);
      if (closeTo(pos[0], 2, pos[2], 110, ex, ey, ez)) glEnable(GL_LIGHT5); else glDisable(GL_LIGHT5); }
    // GL_LIGHT6 warm window glow in front of the house
    setPointLight(GL_LIGHT6, 0, 4.5f, -10.5f, 0.55f, 0.38f, 0.14f, 0.6f, 0.05f, 0.03f);
    if (closeTo(0, 4, -10, 30, ex, ey, ez)) glEnable(GL_LIGHT6); else glDisable(GL_LIGHT6);
    // GL_LIGHT7 ghost glow (follows the ghost)
    { float gx, gy, gz; ghostPos(gx, gy, gz);
      setPointLight(GL_LIGHT7, gx, gy, gz, 0.35f, 0.5f, 0.7f, 0.6f, 0.06f, 0.04f);
      if (closeTo(gx, gy, gz, 26, ex, ey, ez)) glEnable(GL_LIGHT7); else glDisable(GL_LIGHT7); }
}

// ============================================================================
//  HUD
// ============================================================================
static void text(float x, float y, const char* s, void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y); for (; *s; s++) glutBitmapCharacter(font, *s);
}
static const char* zoneName() {
    if (fabsf(px) < 12 && pz < -14 && pz > -30) return feet > 3.0f ? "Upstairs" : "Mansion hall";
    if (px > 42) return "Railway / forest";
    if (fabsf(px) > 40 || fabsf(pz) > 40) return "Outside the fence";
    return "Haunted grounds";
}
static void drawHUD() {
    glDisable(GL_LIGHTING); glDisable(GL_FOG); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    char buf[160];
    glColor3f(0.85f, 0.75f, 0.45f);
    snprintf(buf, sizeof buf, "Interactive 3D Haunted House  |  %s  |  Flashlight: %s  |  %.0f fps", zoneName(), flashOn ? "ON" : "off", fpsVal);
    text(12, winH - 20, buf);
    if (hudOn) {
        glColor3f(0.6f, 0.65f, 0.7f);
        text(12, 46, "WASD move (Shift run)   Mouse/arrows look   F flashlight   E door/gate   L lightning");
        text(12, 28, "1 gate   2 hall   3 bedroom   4 railway   M mouse look   H hide help   Esc quit");
    }
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_FOG); glEnable(GL_LIGHTING);
}

// ============================================================================
//  DISPLAY
// ============================================================================
static float fwd[3];
static void display() {
    float fc[4] = { fogBase[0] + flashV * 0.5f, fogBase[1] + flashV * 0.5f, fogBase[2] + flashV * 0.55f, 1 };
    glClearColor(fc[0], fc[1], fc[2], 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    // flashlight: spot in EYE space (modelview is identity here), before gluLookAt
    GLfloat fPos[] = { 0.0f, 0.0f, 0.0f, 1.0f }, fDir[] = { 0.0f, 0.0f, -1.0f }, fDif[] = { 1.2f, 1.2f, 1.05f, 1 }, fAmb[] = { 0, 0, 0, 1 };
    glLightfv(GL_LIGHT1, GL_POSITION, fPos); glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, fDir);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 22.0f); glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 12.0f);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fDif); glLightfv(GL_LIGHT1, GL_SPECULAR, fDif); glLightfv(GL_LIGHT1, GL_AMBIENT, fAmb);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5f); glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.04f); glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.004f);
    if (flashOn) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);

    float bob = 0.035f * sinf(walkPhase), ex = px, ey = feet + 1.7f + bob, ez = pz;
    gluLookAt(ex, ey, ez, ex + fwd[0], ey + fwd[1], ez + fwd[2], 0, 1, 0);

    GLfloat amb[4] = { (0.16f - 0.04f * indoorAmt) + flashV * 0.5f, (0.19f - 0.06f * indoorAmt) + flashV * 0.5f, (0.27f - 0.10f * indoorAmt) + flashV * 0.55f, 1 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    glFogfv(GL_FOG_COLOR, fc); glFogf(GL_FOG_DENSITY, fogDens);

    setupWorldLights(ex, ey, ez);
    drawSky(ex, ey, ez);

    glEnable(GL_LIGHTING); glEnable(GL_FOG); glEnable(GL_DEPTH_TEST); glDepthMask(GL_TRUE);
    // ---- opaque scene, in the order of the report (Section 15)
    glCallList(base + LG_GROUND); glCallList(base + LG_FENCE); glCallList(base + LG_PATH);
    glCallList(base + LG_TREES);  glCallList(base + LG_GRAVE); glCallList(base + LG_PROPS);
    glCallList(base + LG_EXT);    glCallList(base + LG_INT);
    glCallList(base + LG_RAIL);   glCallList(base + LG_PINES);
    drawGate();
    for (int i = 0; i < 4; i++) drawDoorLeaf(doors[i]);
    drawChandelier();
    drawCandelabra(-2.05f, F0 + 1.5f, -15.0f, 0); drawCandelabra(5.0f, F0 + 0.8f, -15.2f, 2); drawCandelabra(5.0f, F0 + 0.8f, -27.5f, 4);
    drape(7.7f, WH - 0.1f, -29.6f, 1.0f, 3.4f, 0, 0.0f, 0.10f, 0.12f, 0.17f, 1);       // bedroom B drapes (both sides of the bed)
    drape(11.3f, WH - 0.1f, -29.6f, 1.0f, 3.2f, 0, 2.0f, 0.10f, 0.12f, 0.17f, 1);
    drape(-11.6f, WH - 0.1f, -14.6f, 1.0f, 3.0f, 0, 1.0f, 0.12f, 0.06f, 0.06f, 0);     // bedroom A curtain
    drape(11.5f, WH - 0.1f, -23.3f, 1.2f, 2.6f, -90, 0.5f, 0.28f, 0.24f, 0.18f, 1);    // bedroom C canopy over the first iron bed
    rockingChair(); drawCandleB(); drawLampBulbs();
    drawTrain();
    // ---- transparent things last
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE);
    drawBeams(); drawFogSheets(); drawGhost(ex, ez);
    glDepthMask(GL_TRUE); glDisable(GL_BLEND);
    drawBatsCrows();

    drawHUD();
    glutSwapBuffers();
}

// ============================================================================
//  UPDATE
// ============================================================================
static float stepsH(float z) {
    int k = (int)ceilf((-11.5f - z) / 0.5f); if (k < 0) k = 0; if (k > 5) k = 5; return 0.06f * k;
}
static float floorH(float x, float z, float ft) {
    if (fabsf(x) < 2.2f && z <= -11.5f && z > -13.8f) return stepsH(z);
    bool inH = fabsf(x) < 12.2f && z < -13.8f && z > -30.2f;
    if (!inH) return 0;
    if (x > -6.0f && x < -2.0f && z <= -15.0f && z > -21.3f) {
        int idx = (int)ceilf((-15.0f - z) / 0.45f); if (idx < 0) idx = 0; if (idx > 14) idx = 14; return F0 + 0.3f * idx;
    }
    bool upper = (fabsf(x) < 6.2f && z < -21.3f) || fabsf(x) > 5.8f;
    if (upper && ft > 2.6f) return F1;
    return F0;
}
static void collide(float& x, float& z, const std::vector<Box>& extra) {
    const float r = 0.35f;
    for (int it = 0; it < 3; it++) {
        for (unsigned n = 0; n < solids.size() + extra.size(); n++) {
            const Box& b = n < solids.size() ? solids[n] : extra[n - solids.size()];
            if (feet + 1.6f <= b.y0 || feet + 0.3f >= b.y1) continue;
            float cx = fmaxf(b.x0, fminf(x, b.x1)), cz = fmaxf(b.z0, fminf(z, b.z1)), dx = x - cx, dz = z - cz, d2 = dx * dx + dz * dz;
            if (d2 >= r * r) continue;
            if (d2 > 1e-8f) { float d = sqrtf(d2); x = cx + dx / d * r; z = cz + dz / d * r; }
            else {                                                       // centre is inside the box: push out the shortest way
                float l = x - b.x0, rr = b.x1 - x, f = z - b.z0, bk = b.z1 - z, m = fminf(fminf(l, rr), fminf(f, bk));
                if (m == l) x = b.x0 - r; else if (m == rr) x = b.x1 + r; else if (m == f) z = b.z0 - r; else z = b.z1 + r;
            }
        }
    }
}
static void setView(int n) {
    switch (n) {
    case 1: px = 0; pz = 46; yaw = 0; pitchA = 0.03f; feet = 0; break;                       // gate (R3 / R2)
    case 2: px = 0.5f; pz = -15.6f; yaw = 0; pitchA = 0.06f; feet = F0; break;                // hall (R6)
    case 3: px = 7.2f; pz = -26.0f; yaw = 30.0f / DEG; pitchA = -0.05f; feet = F1; break;    // bedroom (R9 - R11)
    case 4: px = trackX(50) - 2.4f; pz = 50; yaw = 0.14f; pitchA = 0.03f; feet = 0; break;            // railway (R12 / R13)
    }
}
static void initDoors() {
    Door d0 = { -1.2f, F0, -14.0f, 0, 2.4f, 3.0f, 1, 90, 0, 0, false, { -1.2f, F0, -14.15f, 1.2f, 3.4f, -13.85f } };
    Door d1 = { -6.0f, F1, -23.4f, 90, 1.2f, 3.0f, 1, 90, 0, 0, false, { -6.15f, F1, -24.6f, -5.85f, 7.5f, -23.4f } };
    Door d2 = { 6.0f, F1, -26.9f, 90, 1.2f, 3.0f, -1, 90, 0, 0, false, { 5.85f, F1, -28.1f, 6.15f, 7.5f, -26.9f } };
    Door d3 = { 6.0f, F1, -21.9f, 90, 1.2f, 3.0f, -1, 90, 0, 0, false, { 5.85f, F1, -23.1f, 6.15f, 7.5f, -21.9f } };
    doors[0] = d0; doors[1] = d1; doors[2] = d2; doors[3] = d3;
}
static void interact() {
    float best = 1e9f; int bi = -1;
    for (int i = 0; i < 4; i++) {
        Door& d = doors[i]; float cx = (d.gap.x0 + d.gap.x1) / 2, cz = (d.gap.z0 + d.gap.z1) / 2;
        float dist = sqrtf((px - cx) * (px - cx) + (pz - cz) * (pz - cz));
        if (fabsf(feet + 0.0f - (d.hy - (i == 0 ? F0 : F1))) < 1.5f && dist < 3.2f && dist < best) { best = dist; bi = i; }
    }
    float gd = sqrtf(px * px + (pz - 40) * (pz - 40));
    if (gd < 6.5f && gd < best) { gateToggle = !gateToggle; return; }
    if (bi >= 0) { doors[bi].target = doors[bi].target > 0.5f ? 0.0f : 1.0f; doors[bi].latch = true; }
}
static void update(float dt) {
    simTime += dt;
    fwd[0] = sinf(yaw) * cosf(pitchA); fwd[1] = sinf(pitchA); fwd[2] = -cosf(yaw) * cosf(pitchA);
    yaw += ((tR ? 1 : 0) - (tL ? 1 : 0)) * 1.8f * dt;
    pitchA += ((tU ? 1 : 0) - (tD ? 1 : 0)) * 1.2f * dt;
    if (pitchA > 1.3f) pitchA = 1.3f; if (pitchA < -1.3f) pitchA = -1.3f;

    // doors + gate
    for (int i = 0; i < 4; i++) {
        Door& d = doors[i];
        if (i == 1 && !d.latch && feet > 3.0f && fabsf(px + 5.0f) < 3.0f && fabsf(pz + 24) < 3.0f) { d.target = 1; d.latch = true; }   // first bedroom door creaks open
        float diff = d.target - d.open, st = 0.9f * dt; d.open += fabsf(diff) < st ? diff : (diff > 0 ? st : -st);
    }
    bool nearGate = px * px + (pz - 40) * (pz - 40) < 7.0f * 7.0f;
    float gt = (nearGate != gateToggle) ? 1.0f : 0.0f, gd = gt - gateOpen, gs = 0.7f * dt;
    gateOpen += fabsf(gd) < gs ? gd : (gd > 0 ? gs : -gs);

    // walking
    float sp = (shiftDown ? 6.0f : 3.0f) * dt, fx = sinf(yaw), fz = -cosf(yaw), rx = cosf(yaw), rz = sinf(yaw), mx = 0, mz = 0;
    if (kW) { mx += fx; mz += fz; } if (kS) { mx -= fx; mz -= fz; } if (kD) { mx += rx; mz += rz; } if (kA) { mx -= rx; mz -= rz; }
    float ml = sqrtf(mx * mx + mz * mz);
    if (ml > 0.001f) { mx /= ml; mz /= ml; walkPhase += (shiftDown ? 11.0f : 7.0f) * dt; }
    float nx = px + mx * sp, nz = pz + mz * sp;
    std::vector<Box> extra;
    for (int i = 0; i < 4; i++) if (doors[i].open < 0.5f) extra.push_back(doors[i].gap);
    if (gateOpen < 0.5f) { Box g = { -4, 0, 39.85f, 4, 2.6f, 40.15f }; extra.push_back(g); }
    Box fr[] = { {-40.2f,0,39.9f,-4.0f,2.6f,40.1f},{4.0f,0,39.9f,40.2f,2.6f,40.1f},{-40.2f,0,-40.1f,40.2f,2.6f,-39.9f},{39.9f,0,-40.2f,40.1f,2.6f,40.2f},{-40.1f,0,-40.2f,-39.9f,2.6f,40.2f} };
    for (int i = 0; i < 5; i++) extra.push_back(fr[i]);
    collide(nx, nz, extra);
    if (nx > 120) nx = 120; if (nx < -120) nx = -120; if (nz > 120) nz = 120; if (nz < -120) nz = -120;
    px = nx; pz = nz;
    float tf = floorH(px, pz, feet); feet += (tf - feet) * fminf(1.0f, dt * (tf > feet ? 14.0f : 8.0f));

    // atmosphere: fog density by zone (0.018 outdoors, 0.006 inside, 0.03 railway/forest)
    bool inside = fabsf(px) < 11.9f && pz < -14.1f && pz > -29.9f;
    float ti = inside ? 1.0f : 0.0f; indoorAmt += (ti - indoorAmt) * fminf(1.0f, dt * 3);
    float td = inside ? 0.006f : (px > 42 ? 0.03f : 0.018f); fogDens += (td - fogDens) * fminf(1.0f, dt * 2);
    glFogf(GL_FOG_DENSITY, fogDens);

    // flicker
    fChand = 0.85f + 0.10f * sinf(9 * simTime) + 0.06f * sinf(23 * simTime + 1) + 0.05f * (rnd() - 0.5f);
    fCandle = 0.65f + 0.20f * sinf(13 * simTime) + 0.12f * sinf(31 * simTime + 2) + 0.2f * (rnd() - 0.5f);
    fLamp = (fmodf(simTime, 7.0f) < 0.3f) ? 0.25f + 0.6f * fabsf(sinf(simTime * 45)) : 1.0f;

    // lightning
    nextFlash -= dt; if (nextFlash <= 0) { flashT = 0; nextFlash = rndr(12, 28); }
    if (flashT >= 0) { flashT += dt; flashV = (flashT < 0.06f || (flashT > 0.09f && flashT < 0.15f)) ? 1.0f : 0.0f; if (flashT > 0.2f) { flashT = -1; flashV = 0; } }
}

// ============================================================================
//  INPUT / GLUT
// ============================================================================
static void keyDown(unsigned char k, int, int) {
    shiftDown = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
    if (k >= 'A' && k <= 'Z') k = k - 'A' + 'a';
    switch (k) {
    case 'w': kW = true; break; case 'a': kA = true; break; case 's': kS = true; break; case 'd': kD = true; break;
    case 'f': flashOn = !flashOn; break; case 'e': interact(); break; case 'l': flashT = 0; break; case 'h': hudOn = !hudOn; break;
    case 'm': mouseLook = !mouseLook; glutSetCursor(mouseLook ? GLUT_CURSOR_NONE : GLUT_CURSOR_LEFT_ARROW); break;
    case '1': case '2': case '3': case '4': setView(k - '0'); break;
    case 27: exit(0);
    }
}
static void keyUp(unsigned char k, int, int) {
    shiftDown = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
    if (k >= 'A' && k <= 'Z') k = k - 'A' + 'a';
    switch (k) { case 'w': kW = false; break; case 'a': kA = false; break; case 's': kS = false; break; case 'd': kD = false; break; }
}
static void specialDown(int k, int, int) {
    switch (k) { case GLUT_KEY_LEFT: tL = true; break; case GLUT_KEY_RIGHT: tR = true; break; case GLUT_KEY_UP: tU = true; break; case GLUT_KEY_DOWN: tD = true; break; }
}
static void specialUp(int k, int, int) {
    switch (k) { case GLUT_KEY_LEFT: tL = false; break; case GLUT_KEY_RIGHT: tR = false; break; case GLUT_KEY_UP: tU = false; break; case GLUT_KEY_DOWN: tD = false; break; }
}
static void mouseMove(int x, int y) {
    if (!mouseLook) return;
    int cx = winW / 2, cy = winH / 2;
    if (warpIgnore) { warpIgnore = false; if (x == cx && y == cy) return; }
    if (x == cx && y == cy) return;
    yaw += (x - cx) * 0.0025f; pitchA -= (y - cy) * 0.0025f;
    if (pitchA > 1.3f) pitchA = 1.3f; if (pitchA < -1.3f) pitchA = -1.3f;
    warpIgnore = true; glutWarpPointer(cx, cy);
}
static void reshape(int w, int h) {
    if (h < 1) h = 1; winW = w; winH = h; glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(65.0, (double)w / h, 0.1, 400.0); glMatrixMode(GL_MODELVIEW);
}
static void idle() {
    int ms = glutGet(GLUT_ELAPSED_TIME); float dt = (ms - lastMs) / 1000.0f; lastMs = ms;
    if (dt > 0.1f) dt = 0.1f;
    update(dt);
    fpsFrames++; fpsAcc += dt; if (fpsAcc >= 0.5f) { fpsVal = fpsFrames / fpsAcc; fpsFrames = 0; fpsAcc = 0; }
    glutPostRedisplay();
}

static void init() {
    Q = gluNewQuadric(); gluQuadricNormals(Q, GLU_SMOOTH);
    glEnable(GL_DEPTH_TEST); glEnable(GL_NORMALIZE); glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL); glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_FOG); glFogi(GL_FOG_MODE, GL_EXP2); glFogfv(GL_FOG_COLOR, fogBase); glFogf(GL_FOG_DENSITY, 0.018f); glHint(GL_FOG_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    rs = 31337; for (int i = 0; i < 200; i++) {                       // star field on a dome
        float a = rndr(0, 2 * PI), e = rndr(0.12f, 1.4f);
        stars[i][0] = 150 * cosf(e) * cosf(a); stars[i][1] = 150 * sinf(e); stars[i][2] = 150 * cosf(e) * sinf(a);
    }
    base = glGenLists(LG_COUNT);
    genTrees(); initDoors();
    buildGround(); buildPath(); buildFence(); buildTrees(); buildGrave(); buildProps();
    buildHouseExt(); buildHouseInt(); buildRail(); buildLeaf();
    setView(1);
    fwd[0] = 0; fwd[1] = 0; fwd[2] = -1;
}

// optional: ./haunted_house --shot <viewpoint 1-4> <file.ppm>   (renders one frame, saves it, exits; used for testing)
static int shotView = 0; static const char* shotFile = 0;
static void shotIdle() {
    for (int i = 0; i < 90; i++) update(1.0f / 30.0f);
    if (shotView == 4) simTime = 16.0f;
    display();
    std::vector<unsigned char> px3(winW * winH * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1); glReadPixels(0, 0, winW, winH, GL_RGB, GL_UNSIGNED_BYTE, &px3[0]);
    FILE* f = fopen(shotFile, "wb"); fprintf(f, "P6\n%d %d\n255\n", winW, winH);
    for (int y = winH - 1; y >= 0; y--) fwrite(&px3[y * winW * 3], 1, winW * 3, f);
    fclose(f); exit(0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Interactive 3D Haunted House Simulation - Roll 2107091");
    init();
    glutDisplayFunc(display); glutReshapeFunc(reshape); glutIdleFunc(idle);
    glutKeyboardFunc(keyDown); glutKeyboardUpFunc(keyUp); glutSpecialFunc(specialDown); glutSpecialUpFunc(specialUp);
    glutPassiveMotionFunc(mouseMove); glutMotionFunc(mouseMove); glutIgnoreKeyRepeat(1);
    if (argc >= 4 && !strcmp(argv[1], "--shot")) {
        shotView = atoi(argv[2]); shotFile = argv[3]; setView(shotView);
        if (argc >= 6) { yaw = atof(argv[4]) / DEG; pitchA = atof(argv[5]) / DEG; }
        if (argc >= 8) { px = atof(argv[6]); pz = atof(argv[7]); }
        if (argc >= 9) feet = atof(argv[8]);
        glutIdleFunc(shotIdle);
    } else {
        glutSetCursor(GLUT_CURSOR_NONE); lastMs = glutGet(GLUT_ELAPSED_TIME);
        glutWarpPointer(winW / 2, winH / 2); warpIgnore = true;
    }
    glutMainLoop();
    return 0;
}
