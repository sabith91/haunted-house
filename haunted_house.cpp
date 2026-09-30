// ============================================================================
//  Interactive 3D Haunted House Simulation      Simple OpenGL Project | Roll 2107091
//  Student : MD.SABITH  (CSE, 3rd year)
//  Tech    : C++, OpenGL 1.x (fixed function), GLUT / FreeGLUT, GLU quadrics
//  Mood    : Granny (house horror) + Alan Wake 2 (foggy forest, railway, flashlight)
//
//  Modular Architecture:
//    - common.h          : Shared constants, math helpers, structs, extern globals
//    - primitives.h/.cpp : Shapes, subdivided walls (bigBox, wallBox), materials, windows
//    - train_railway.h/.cpp : Track geometry, rail display list, animated train
//    - house_exterior.h/.cpp: Mansion exterior walls, roof, gables, dormers, tower/turret
//    - house_interior.h/.cpp: Hall staircase, balustrades, wall linings, Bedrooms A/B/C
//    - grounds.h/.cpp    : Estate grounds, cobblestone path, fence, trees, graveyard, props
//    - dynamic_objects.h/.cpp: Animated doors, gate, chandelier, ghost, creatures, drapes
//    - lighting_sky.h/.cpp   : Sky dome, star field, 8 fixed-function lights, 2D HUD
//    - haunted_house.cpp : Main entry point, player physics, collision, input & game loop
// ============================================================================

#include "common.h"
#include "primitives.h"
#include "train_railway.h"
#include "house_exterior.h"
#include "house_interior.h"
#include "grounds.h"
#include "dynamic_objects.h"
#include "lighting_sky.h"

// ------------------------------------------------------------------ global state instances
GLUquadric* Q = NULL;
int winW = 1280, winH = 720;

// player / camera
float px = 0, pz = 47, feet = 0, yaw = 0, pitchA = 0.02f, walkPhase = 0;
bool kW = false, kA = false, kS = false, kD = false, shiftDown = false;
bool tL = false, tR = false, tU = false, tD = false; // arrow keys
bool flashOn = true, hudOn = true, mouseLook = true, warpIgnore = false;

// time & atmosphere
float simTime = 0, fogDens = 0.018f, indoorAmt = 0;
float flashV = 0, flashT = -1, nextFlash = 9; // lightning
int lastMs = 0, fpsFrames = 0;
float fpsVal = 0, fpsAcc = 0;
const float fogBase[3] = { 0.08f, 0.12f, 0.13f };

// flicker values
float fChand = 1, fCandle = 1, fLamp = 1;

// display lists & randomness
GLuint base = 0;
unsigned rs = 12345;

// collision solids
std::vector<Box> solids;

// doors & gate
Door doors[4];
float gateOpen = 0;
bool gateToggle = false;

// trees
std::vector<Spot> deadTrees, pines;

// camera forward vector
float fwd[3] = { 0, 0, -1 };

// ------------------------------------------------------------------ collision registrar
void solid(float x0, float y0, float z0, float x1, float y1, float z1) {
    Box b = { x0, y0, z0, x1, y1, z1 };
    solids.push_back(b);
}

// ============================================================================
//  DISPLAY
// ============================================================================
static void display() {
    float fc[4] = { fogBase[0] + flashV * 0.5f, fogBase[1] + flashV * 0.5f, fogBase[2] + flashV * 0.55f, 1 };
    glClearColor(fc[0], fc[1], fc[2], 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // flashlight: spot in EYE space (modelview is identity here), before gluLookAt
    GLfloat fPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat fDir[] = { 0.0f, 0.0f, -1.0f };
    GLfloat fDif[] = { 1.2f, 1.2f, 1.05f, 1 };
    GLfloat fAmb[] = { 0, 0, 0, 1 };
    glLightfv(GL_LIGHT1, GL_POSITION, fPos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, fDir);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 22.0f);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 12.0f);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fDif);
    glLightfv(GL_LIGHT1, GL_SPECULAR, fDif);
    glLightfv(GL_LIGHT1, GL_AMBIENT, fAmb);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.04f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.004f);
    if (flashOn) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);

    float bob = 0.035f * sinf(walkPhase);
    float ex = px, ey = feet + 1.7f + bob, ez = pz;
    gluLookAt(ex, ey, ez, ex + fwd[0], ey + fwd[1], ez + fwd[2], 0, 1, 0);

    GLfloat amb[4] = {
        (0.16f - 0.04f * indoorAmt) + flashV * 0.5f,
        (0.19f - 0.06f * indoorAmt) + flashV * 0.5f,
        (0.27f - 0.10f * indoorAmt) + flashV * 0.55f,
        1
    };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    glFogfv(GL_FOG_COLOR, fc);
    glFogf(GL_FOG_DENSITY, fogDens);

    setupWorldLights(ex, ey, ez);
    drawSky(ex, ey, ez);

    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // ---- opaque scene, in the order of the report (Section 15)
    glCallList(base + LG_GROUND);
    glCallList(base + LG_FENCE);
    glCallList(base + LG_PATH);
    glCallList(base + LG_TREES);
    glCallList(base + LG_GRAVE);
    glCallList(base + LG_PROPS);
    glCallList(base + LG_EXT);
    glCallList(base + LG_INT);
    glCallList(base + LG_RAIL);
    glCallList(base + LG_PINES);
    drawGate();
    for (int i = 0; i < 4; i++) drawDoorLeaf(doors[i]);
    drawChandelier();
    drawCandelabra(-2.05f, F0 + 1.5f, -15.0f, 0);
    drawCandelabra(5.0f, F0 + 0.8f, -15.2f, 2);
    drawCandelabra(5.0f, F0 + 0.8f, -27.5f, 4);
    drape(7.7f, WH - 0.1f, -29.6f, 1.0f, 3.4f, 0, 0.0f, 0.10f, 0.12f, 0.17f, 1);    // bedroom B drapes
    drape(11.3f, WH - 0.1f, -29.6f, 1.0f, 3.2f, 0, 2.0f, 0.10f, 0.12f, 0.17f, 1);
    drape(-11.6f, WH - 0.1f, -14.6f, 1.0f, 3.0f, 0, 1.0f, 0.12f, 0.06f, 0.06f, 0);  // bedroom A curtain
    drape(11.5f, WH - 0.1f, -23.3f, 1.2f, 2.6f, -90, 0.5f, 0.28f, 0.24f, 0.18f, 1); // bedroom C canopy
    rockingChair();
    drawCandleB();
    drawLampBulbs();
    drawTrain();

    // ---- transparent things last
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    drawBeams();
    drawFogSheets();
    drawGhost(ex, ez);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    drawBatsCrows();

    drawHUD();
    glutSwapBuffers();
}

// ============================================================================
//  UPDATE & PHYSICS
// ============================================================================
static float stepsH(float z) {
    int k = (int)ceilf((-11.5f - z) / 0.5f);
    if (k < 0) k = 0;
    if (k > 5) k = 5;
    return 0.06f * k;
}

static float floorH(float x, float z, float ft) {
    if (fabsf(x) < 2.2f && z <= -11.5f && z > -13.8f) return stepsH(z);
    bool inH = fabsf(x) < 12.2f && z < -13.8f && z > -30.2f;
    if (!inH) return 0;
    if (x > -6.0f && x < -2.0f && z <= -15.0f && z > -21.3f) {
        int idx = (int)ceilf((-15.0f - z) / 0.45f);
        if (idx < 0) idx = 0;
        if (idx > 14) idx = 14;
        return F0 + 0.3f * idx;
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
            float cx = fmaxf(b.x0, fminf(x, b.x1));
            float cz = fmaxf(b.z0, fminf(z, b.z1));
            float dx = x - cx, dz = z - cz;
            float d2 = dx * dx + dz * dz;
            if (d2 >= r * r) continue;
            if (d2 > 1e-8f) {
                float d = sqrtf(d2);
                x = cx + dx / d * r;
                z = cz + dz / d * r;
            } else { // centre is inside the box: push out the shortest way
                float l = x - b.x0, rr = b.x1 - x, f = z - b.z0, bk = b.z1 - z;
                float m = fminf(fminf(l, rr), fminf(f, bk));
                if (m == l) x = b.x0 - r;
                else if (m == rr) x = b.x1 + r;
                else if (m == f) z = b.z0 - r;
                else z = b.z1 + r;
            }
        }
    }
}

static void setView(int n) {
    switch (n) {
    case 1: px = 0; pz = 46; yaw = 0; pitchA = 0.03f; feet = 0; break;                    // gate (R3 / R2)
    case 2: px = 0.5f; pz = -15.6f; yaw = 0; pitchA = 0.06f; feet = F0; break;             // hall (R6)
    case 3: px = 7.2f; pz = -26.0f; yaw = 30.0f / DEG; pitchA = -0.05f; feet = F1; break; // bedroom (R9 - R11)
    case 4: px = trackX(50) - 2.4f; pz = 50; yaw = 0.14f; pitchA = 0.03f; feet = 0; break; // railway (R12 / R13)
    }
}

static void initDoors() {
    Door d0 = { -1.2f, F0, -14.0f, 0,  2.4f, 3.0f,  1, 90, 0, 0, false, { -1.2f,  F0, -14.15f,  1.2f, 3.4f, -13.85f } };
    Door d1 = { -6.0f, F1, -23.4f, 90, 1.2f, 3.0f,  1, 90, 0, 0, false, { -6.15f, F1, -24.6f,  -5.85f, 7.5f, -23.4f } };
    Door d2 = {  6.0f, F1, -26.9f, 90, 1.2f, 3.0f, -1, 90, 0, 0, false, {  5.85f, F1, -28.1f,   6.15f, 7.5f, -26.9f } };
    Door d3 = {  6.0f, F1, -21.9f, 90, 1.2f, 3.0f, -1, 90, 0, 0, false, {  5.85f, F1, -23.1f,   6.15f, 7.5f, -21.9f } };
    doors[0] = d0; doors[1] = d1; doors[2] = d2; doors[3] = d3;
}

static void interact() {
    float best = 1e9f;
    int bi = -1;
    for (int i = 0; i < 4; i++) {
        Door& d = doors[i];
        float cx = (d.gap.x0 + d.gap.x1) / 2, cz = (d.gap.z0 + d.gap.z1) / 2;
        float dist = sqrtf((px - cx) * (px - cx) + (pz - cz) * (pz - cz));
        if (fabsf(feet + 0.0f - (d.hy - (i == 0 ? F0 : F1))) < 1.5f && dist < 3.2f && dist < best) {
            best = dist;
            bi = i;
        }
    }
    float gd = sqrtf(px * px + (pz - 40) * (pz - 40));
    if (gd < 6.5f && gd < best) {
        gateToggle = !gateToggle;
        return;
    }
    if (bi >= 0) {
        doors[bi].target = doors[bi].target > 0.5f ? 0.0f : 1.0f;
        doors[bi].latch = true;
    }
}

static void update(float dt) {
    simTime += dt;
    fwd[0] = sinf(yaw) * cosf(pitchA);
    fwd[1] = sinf(pitchA);
    fwd[2] = -cosf(yaw) * cosf(pitchA);
    yaw += ((tR ? 1 : 0) - (tL ? 1 : 0)) * 1.8f * dt;
    pitchA += ((tU ? 1 : 0) - (tD ? 1 : 0)) * 1.2f * dt;
    if (pitchA > 1.3f) pitchA = 1.3f;
    if (pitchA < -1.3f) pitchA = -1.3f;

    // doors + gate
    for (int i = 0; i < 4; i++) {
        Door& d = doors[i];
        if (i == 1 && !d.latch && feet > 3.0f && fabsf(px + 5.0f) < 3.0f && fabsf(pz + 24) < 3.0f) {
            d.target = 1;
            d.latch = true; // first bedroom door creaks open
        }
        float diff = d.target - d.open, st = 0.9f * dt;
        d.open += fabsf(diff) < st ? diff : (diff > 0 ? st : -st);
    }
    bool nearGate = px * px + (pz - 40) * (pz - 40) < 7.0f * 7.0f;
    float gt = (nearGate != gateToggle) ? 1.0f : 0.0f;
    float gd = gt - gateOpen, gs = 0.7f * dt;
    gateOpen += fabsf(gd) < gs ? gd : (gd > 0 ? gs : -gs);

    // walking
    float sp = (shiftDown ? 6.0f : 3.0f) * dt;
    float fx = sinf(yaw), fz = -cosf(yaw), rx = cosf(yaw), rz = sinf(yaw);
    float mx = 0, mz = 0;
    if (kW) { mx += fx; mz += fz; }
    if (kS) { mx -= fx; mz -= fz; }
    if (kD) { mx += rx; mz += rz; }
    if (kA) { mx -= rx; mz -= rz; }
    float ml = sqrtf(mx * mx + mz * mz);
    if (ml > 0.001f) {
        mx /= ml; mz /= ml;
        walkPhase += (shiftDown ? 11.0f : 7.0f) * dt;
    }
    float nx = px + mx * sp, nz = pz + mz * sp;
    std::vector<Box> extra;
    for (int i = 0; i < 4; i++) if (doors[i].open < 0.5f) extra.push_back(doors[i].gap);
    if (gateOpen < 0.5f) {
        Box g = { -4, 0, 39.85f, 4, 2.6f, 40.15f };
        extra.push_back(g);
    }
    Box fr[] = {
        {-40.2f, 0,  39.9f, -4.0f,  2.6f,  40.1f},
        {  4.0f, 0,  39.9f, 40.2f,  2.6f,  40.1f},
        {-40.2f, 0, -40.1f, 40.2f,  2.6f, -39.9f},
        { 39.9f, 0, -40.2f, 40.1f,  2.6f,  40.2f},
        {-40.1f, 0, -40.2f, -39.9f, 2.6f,  40.2f}
    };
    for (int i = 0; i < 5; i++) extra.push_back(fr[i]);
    collide(nx, nz, extra);
    if (nx > 120) nx = 120;
    if (nx < -120) nx = -120;
    if (nz > 120) nz = 120;
    if (nz < -120) nz = -120;
    px = nx; pz = nz;
    float tf = floorH(px, pz, feet);
    feet += (tf - feet) * fminf(1.0f, dt * (tf > feet ? 14.0f : 8.0f));

    // atmosphere: fog density by zone (0.018 outdoors, 0.006 inside, 0.03 railway/forest)
    bool inside = fabsf(px) < 11.9f && pz < -14.1f && pz > -29.9f;
    float ti = inside ? 1.0f : 0.0f;
    indoorAmt += (ti - indoorAmt) * fminf(1.0f, dt * 3);
    float td = inside ? 0.006f : (px > 42 ? 0.03f : 0.018f);
    fogDens += (td - fogDens) * fminf(1.0f, dt * 2);
    glFogf(GL_FOG_DENSITY, fogDens);

    // flicker
    fChand = 0.85f + 0.10f * sinf(9 * simTime) + 0.06f * sinf(23 * simTime + 1) + 0.05f * (rnd() - 0.5f);
    fCandle = 0.65f + 0.20f * sinf(13 * simTime) + 0.12f * sinf(31 * simTime + 2) + 0.2f * (rnd() - 0.5f);
    fLamp = (fmodf(simTime, 7.0f) < 0.3f) ? 0.25f + 0.6f * fabsf(sinf(simTime * 45)) : 1.0f;

    // lightning
    nextFlash -= dt;
    if (nextFlash <= 0) {
        flashT = 0;
        nextFlash = rndr(12, 28);
    }
    if (flashT >= 0) {
        flashT += dt;
        flashV = (flashT < 0.06f || (flashT > 0.09f && flashT < 0.15f)) ? 1.0f : 0.0f;
        if (flashT > 0.2f) {
            flashT = -1;
            flashV = 0;
        }
    }
}

// ============================================================================
//  INPUT / GLUT
// ============================================================================
static void keyDown(unsigned char k, int, int) {
    shiftDown = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
    if (k >= 'A' && k <= 'Z') k = k - 'A' + 'a';
    switch (k) {
    case 'w': kW = true; break;
    case 'a': kA = true; break;
    case 's': kS = true; break;
    case 'd': kD = true; break;
    case 'f': flashOn = !flashOn; break;
    case 'e': interact(); break;
    case 'l': flashT = 0; break;
    case 'h': hudOn = !hudOn; break;
    case 'm':
        mouseLook = !mouseLook;
        glutSetCursor(mouseLook ? GLUT_CURSOR_NONE : GLUT_CURSOR_LEFT_ARROW);
        break;
    case '1': case '2': case '3': case '4':
        setView(k - '0');
        break;
    case 27:
        exit(0);
    }
}

static void keyUp(unsigned char k, int, int) {
    shiftDown = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
    if (k >= 'A' && k <= 'Z') k = k - 'A' + 'a';
    switch (k) {
    case 'w': kW = false; break;
    case 'a': kA = false; break;
    case 's': kS = false; break;
    case 'd': kD = false; break;
    }
}

static void specialDown(int k, int, int) {
    switch (k) {
    case GLUT_KEY_LEFT:  tL = true; break;
    case GLUT_KEY_RIGHT: tR = true; break;
    case GLUT_KEY_UP:    tU = true; break;
    case GLUT_KEY_DOWN:  tD = true; break;
    }
}

static void specialUp(int k, int, int) {
    switch (k) {
    case GLUT_KEY_LEFT:  tL = false; break;
    case GLUT_KEY_RIGHT: tR = false; break;
    case GLUT_KEY_UP:    tU = false; break;
    case GLUT_KEY_DOWN:  tD = false; break;
    }
}

static void mouseMove(int x, int y) {
    if (!mouseLook) return;
    int cx = winW / 2, cy = winH / 2;
    if (warpIgnore) {
        warpIgnore = false;
        if (x == cx && y == cy) return;
    }
    if (x == cx && y == cy) return;
    yaw += (x - cx) * 0.0025f;
    pitchA -= (y - cy) * 0.0025f;
    if (pitchA > 1.3f) pitchA = 1.3f;
    if (pitchA < -1.3f) pitchA = -1.3f;
    warpIgnore = true;
    glutWarpPointer(cx, cy);
}

static void reshape(int w, int h) {
    if (h < 1) h = 1;
    winW = w; winH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, (double)w / h, 0.1, 400.0);
    glMatrixMode(GL_MODELVIEW);
}

static void idle() {
    int ms = glutGet(GLUT_ELAPSED_TIME);
    float dt = (ms - lastMs) / 1000.0f;
    lastMs = ms;
    if (dt > 0.1f) dt = 0.1f;
    update(dt);
    fpsFrames++;
    fpsAcc += dt;
    if (fpsAcc >= 0.5f) {
        fpsVal = fpsFrames / fpsAcc;
        fpsFrames = 0;
        fpsAcc = 0;
    }
    glutPostRedisplay();
}

static void init() {
    Q = gluNewQuadric();
    gluQuadricNormals(Q, GLU_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogfv(GL_FOG_COLOR, fogBase);
    glFogf(GL_FOG_DENSITY, 0.018f);
    glHint(GL_FOG_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    initStars();

    base = glGenLists(LG_COUNT);
    genTrees();
    initDoors();
    buildGround();
    buildPath();
    buildFence();
    buildTrees();
    buildGrave();
    buildProps();
    buildHouseExt();
    buildHouseInt();
    buildRail();
    buildLeaf();

    setView(1);
    fwd[0] = 0; fwd[1] = 0; fwd[2] = -1;
}

// optional: ./haunted_house --shot <viewpoint 1-4> <file.ppm>
static int shotView = 0;
static const char* shotFile = 0;

static void shotIdle() {
    for (int i = 0; i < 90; i++) update(1.0f / 30.0f);
    if (shotView == 4) simTime = 16.0f;
    display();
    std::vector<unsigned char> px3(winW * winH * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, winW, winH, GL_RGB, GL_UNSIGNED_BYTE, &px3[0]);
    FILE* f = fopen(shotFile, "wb");
    if (f) {
        fprintf(f, "P6\n%d %d\n255\n", winW, winH);
        for (int y = winH - 1; y >= 0; y--) fwrite(&px3[y * winW * 3], 1, winW * 3, f);
        fclose(f);
    }
    exit(0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Interactive 3D Haunted House Simulation - Roll 2107091");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseMove);
    glutIgnoreKeyRepeat(1);
    if (argc >= 4 && !strcmp(argv[1], "--shot")) {
        shotView = atoi(argv[2]);
        shotFile = argv[3];
        setView(shotView);
        if (argc >= 6) { yaw = atof(argv[4]) / DEG; pitchA = atof(argv[5]) / DEG; }
        if (argc >= 8) { px = atof(argv[6]); pz = atof(argv[7]); }
        if (argc >= 9) feet = atof(argv[8]);
        glutIdleFunc(shotIdle);
    } else {
        glutSetCursor(GLUT_CURSOR_NONE);
        lastMs = glutGet(GLUT_ELAPSED_TIME);
        glutWarpPointer(winW / 2, winH / 2);
        warpIgnore = true;
    }
    glutMainLoop();
    return 0;
}
