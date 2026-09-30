#include "dynamic_objects.h"

// ------------------------------------------------------------------ animated estate & house features
void drawDoorLeaf(const Door& d) {
    glPushMatrix();
    glTranslatef(d.hx, d.hy, d.hz);
    glRotatef(d.baseRot + d.sgn * d.open * d.maxA, 0, 1, 0);
    matWood();
    col(0.22f, 0.14f, 0.08f);
    box(d.w / 2, d.h / 2, 0, d.w, d.h, 0.1f);
    col(0.15f, 0.09f, 0.05f);
    for (int s = -1; s <= 1; s += 2) {
        box(d.w / 2, d.h * 0.74f, s * 0.06f, d.w * 0.7f, d.h * 0.28f, 0.03f);
        box(d.w / 2, d.h * 0.28f, s * 0.06f, d.w * 0.7f, d.h * 0.34f, 0.03f);
    }
    col(0.55f, 0.42f, 0.2f);
    sph(d.w - 0.2f, d.h * 0.45f, 0.1f, 0.06f, 8);
    sph(d.w - 0.2f, d.h * 0.45f, -0.1f, 0.06f, 8);
    glPopMatrix();
}

void drawGate() {
    float a = gateOpen * 70.0f;
    glPushMatrix();
    glTranslatef(-4, 0, 40);
    glRotatef(a, 0, 1, 0);
    glCallList(base + LG_LEAF);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(4, 0, 40);
    glRotatef(180 - a, 0, 1, 0);
    glCallList(base + LG_LEAF);
    glPopMatrix();
}

void drawChandelier() {
    glPushMatrix();
    glTranslatef(2.0f, WH, -17.6f);
    glRotatef(3.0f * sinf(1.1f * simTime), 0, 0, 1);
    matIron();
    col(0.16f, 0.13f, 0.09f);
    glPushMatrix();
    glTranslatef(0, -1.7f, 0);
    cyl(0.03f, 0.03f, 1.7f, 6);
    glPopMatrix();
    glTranslatef(0, -1.7f, 0);
    col(0.30f, 0.24f, 0.10f);
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.05, 0.9, 8, 22);
    glPopMatrix();
    sph(0, 0, 0, 0.16f, 8);
    for (int i = 0; i < 6; i++) {
        glPushMatrix();
        glRotatef(i * 60.0f, 0, 1, 0);
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        gluCylinder(Q, 0.03, 0.03, 0.9, 6, 1);
        glPopMatrix();
        glTranslatef(0.9f, 0, 0);
        col(0.85f, 0.82f, 0.7f);
        cyl(0.04f, 0.04f, 0.24f, 6);
        flame(0, 0.25f, 0, 1.5f, (float)i);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawCandelabra(float x, float y, float z, float ph) {
    candelabraBase(x, y, z);
    for (int i = -1; i <= 1; i++) {
        flame(x + i * 0.25f, y + 0.66f, z, 1.0f, ph + i);
    }
}

void drape(float x, float y, float z, float w, float h, float rot,
           float ph, float r, float g, float b, float tear) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rot, 0, 1, 0);
    col(r, g, b);
    glNormal3f(0, 0, 1);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 12; i++) {
        float u = (float)i / 12, xs = -w / 2 + u * w, fold = 0.06f * sinf(u * 20 + ph), sw = 0.10f * sinf(simTime * 0.8f + u * 5 + ph);
        float bot = -h * (1.0f - tear * 0.3f * sinf(u * 9 + ph) * sinf(u * 9 + ph));
        glVertex3f(xs, 0, fold);
        glVertex3f(xs + sw, bot, fold + sw * 0.5f);
    }
    glEnd();
    glPopMatrix();
}

void rockingChair() {
    glPushMatrix();
    glTranslatef(-8.7f, F1, -19.0f);
    glRotatef(90, 0, 1, 0); // faces the room centre
    glRotatef(5.0f * sinf(1.3f * simTime), 1, 0, 0);
    matWood();
    col(0.22f, 0.14f, 0.08f);
    box(0, 0.5f, 0, 0.5f, 0.05f, 0.5f);
    box(0, 0.9f, -0.22f, 0.5f, 0.7f, 0.05f);
    for (int a = -1; a <= 1; a += 2) {
        box(a * 0.22f, 0.25f, 0.2f, 0.04f, 0.5f, 0.04f);
        box(a * 0.22f, 0.25f, -0.2f, 0.04f, 0.5f, 0.04f);
        box(a * 0.22f, 0.7f, 0, 0.04f, 0.04f, 0.4f);
        for (int s = -3; s <= 3; s++) {
            box(a * 0.24f, 0.03f + 0.012f * s * s * 0.3f, s * 0.15f, 0.05f, 0.05f, 0.17f); // curved runner
        }
    }
    glPopMatrix();
}

void drawLampBulbs() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f * fLamp, 0.85f * fLamp, 0.45f * fLamp);
    sph(-7, 3.5f, 38, 0.22f, 10);
    glColor3f(1.0f, 0.85f, 0.45f);
    sph(7, 3.5f, 38, 0.22f, 10);
    glColor3f(1.0f, 0.9f, 0.6f);
    sph(-9, WH - 1.65f, -22, 0.09f, 8); // bedroom A lamp
    glEnable(GL_LIGHTING);
}

void drawCandleB() {
    flame(8.2f, F1 + 0.78f, -29.3f, 1.4f, 5.0f);
}

// ------------------------------------------------------------------ animated creatures & atmosphere
void winged(float scale, float flapAmp, float flapSpeed, float ph, float r, float g, float b) {
    glPushMatrix();
    glScalef(scale, scale, scale);
    col(r, g, b);
    ell(0, 0, 0, 0.18f, 0.09f, 0.10f, 8);
    sph(0.17f, 0.03f, 0, 0.07f, 6);
    float a = flapAmp * sinf(flapSpeed * simTime + ph);
    glNormal3f(0, 1, 0);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glRotatef(s * a, 1, 0, 0);
        glBegin(GL_TRIANGLES);
        glVertex3f(0.10f, 0, 0);
        glVertex3f(-0.15f, 0, 0);
        glVertex3f(-0.02f, 0, s * 0.7f);
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();
}

void drawBatsCrows() {
    for (int i = 0; i < 5; i++) { // bats round the round tower
        float th = simTime * (1.3f + 0.15f * i) + i * 1.3f, rad = 4.5f + 0.8f * i;
        glPushMatrix();
        glTranslatef(-13.5f + rad * cosf(th), 14.5f + 1.2f * sinf(simTime * 2 + i), -11.8f + rad * sinf(th));
        glRotatef(-(th * DEG) - 90, 0, 1, 0);
        winged(1.0f, 35, 12, i, 0.04f, 0.04f, 0.05f);
        glPopMatrix();
    }
    for (int i = 0; i < 6; i++) { // crows circle over the trees (R5)
        float th = simTime * (0.35f + 0.03f * i) + i * 1.05f, rad = 15 + 3 * i;
        glPushMatrix();
        glTranslatef(8 + rad * cosf(th), 12.5f + 1.5f * sinf(simTime * 0.7f + i), 12 + rad * sinf(th));
        glRotatef(-(th * DEG) - 90, 0, 1, 0);
        winged(2.4f, 22, 4.5f, i, 0.02f, 0.02f, 0.025f);
        glPopMatrix();
    }
}

void ghostPos(float& x, float& y, float& z) {
    x = -30 + 6.0f * sinf(0.25f * simTime);
    z = 23 + 6.0f * cosf(0.2f * simTime);
    y = 1.5f + 0.3f * sinf(simTime);
}

void drawGhost(float cx, float cz) {
    float x, y, z;
    ghostPos(x, y, z);
    float a = 0.35f + 0.15f * sinf(0.7f * simTime);
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(atan2f(cx - x, cz - z) * DEG, 0, 1, 0);
    glDisable(GL_LIGHTING);
    glColor4f(0.75f, 0.85f, 0.95f, a);
    glPushMatrix();
    glTranslatef(0, -1.0f, 0);
    cone(0.75f, 1.9f, 14);
    glPopMatrix();
    sph(0, 1.1f, 0, 0.36f, 14);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(s * 0.3f, 0.5f, 0.1f);
        glRotatef(s * 70, 0, 0, 1);
        glRotatef(-30, 1, 0, 0);
        cone(0.12f, 0.9f, 6);
        glPopMatrix();
    }
    glColor4f(0.02f, 0.03f, 0.05f, 0.9f);
    sph(-0.13f, 1.17f, 0.31f, 0.06f, 8);
    sph(0.13f, 1.17f, 0.31f, 0.06f, 8);
    ell(0, 0.98f, 0.33f, 0.07f, 0.11f, 0.04f, 8);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

struct Beam {
    float wx, wy, wz, dir;
};

static Beam beams[] = {
    {-12, 6.6f, -18, 1},
    {-12, 6.6f, -22, 1},
    {-12, 6.6f, -26, 1},
    {12, 6.6f, -27.5f, -1},
    {12, 6.6f, -18, -1},
    {12, 6.6f, -22, -1}
};

void drawBeams() {
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    for (unsigned i = 0; i < sizeof(beams) / sizeof(beams[0]); i++) {
        Beam& b = beams[i];
        float w = 0.55f, xi = b.wx + b.dir * 0.32f, xf = b.wx + b.dir * 4.2f, dr = 0.9f;
        float f = 0.8f + 0.2f * sinf(simTime * 0.5f + i);
        glColor4f(0.55f, 0.68f, 0.95f, 0.16f * f);
        glVertex3f(xi, b.wy + 0.8f, b.wz - w);
        glVertex3f(xi, b.wy + 0.8f, b.wz + w);
        glColor4f(0.55f, 0.68f, 0.95f, 0.05f * f);
        glVertex3f(xf, F1 + 0.03f, b.wz + w + dr);
        glVertex3f(xf, F1 + 0.03f, b.wz - w + dr);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawFogSheets() {
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    for (int i = 0; i < 7; i++) {
        float x = 30 * sinf(0.05f * simTime + i * 2.1f);
        float z = -10 + 35 * cosf(0.04f * simTime + i * 1.3f) + i * 4;
        float y = 0.45f + 0.15f * i;
        glColor4f(0.30f, 0.42f, 0.42f, 0.055f);
        glBegin(GL_QUADS);
        glVertex3f(x - 16, y, z - 16);
        glVertex3f(x + 16, y, z - 16);
        glVertex3f(x + 16, y, z + 16);
        glVertex3f(x - 16, y, z + 16);
        glEnd();
    }
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}
