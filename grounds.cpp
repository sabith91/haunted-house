#include "grounds.h"

// ------------------------------------------------------------------ vegetation generators
void branch(float len, float r, int depth) {
    col(0.16f, 0.12f, 0.09f);
    cyl(r, r * 0.5f, len, 6);
    if (depth <= 0) return;
    int n = 2 + (int)(rnd() * 2);
    for (int i = 0; i < n; i++) {
        glPushMatrix();
        glTranslatef(0, len * (0.5f + 0.45f * rnd()), 0);
        glRotatef(rnd() * 360, 0, 1, 0);
        glRotatef(25 + rnd() * 35, 0, 0, 1);
        branch(len * (0.55f + 0.2f * rnd()), r * 0.55f, depth - 1);
        glPopMatrix();
    }
}

void deadTree(unsigned seed, float s) {
    rs = seed;
    glPushMatrix();
    glScalef(s, s, s);
    col(0.15f, 0.11f, 0.08f);
    cyl(0.38f, 0.11f, 4.6f, 8);
    int nb = 4 + (int)(rnd() * 3);
    for (int i = 0; i < nb; i++) {
        glPushMatrix();
        glTranslatef(0, 1.8f + rnd() * 2.6f, 0);
        glRotatef(rnd() * 360, 0, 1, 0);
        glRotatef(30 + rnd() * 30, 0, 0, 1);
        branch(1.5f + rnd(), 0.12f, 1);
        glPopMatrix();
    }
    glPopMatrix();
}

void pineTree(float s) {
    glPushMatrix();
    glScalef(s, s, s);
    col(0.15f, 0.10f, 0.07f);
    cyl(0.28f, 0.18f, 3.2f, 6);
    col(0.03f, 0.10f, 0.06f);
    glPushMatrix(); glTranslatef(0, 2.0f, 0); cone(2.3f, 3.6f, 8); glPopMatrix();
    col(0.035f, 0.115f, 0.065f);
    glPushMatrix(); glTranslatef(0, 4.0f, 0); cone(1.8f, 3.2f, 8); glPopMatrix();
    col(0.04f, 0.12f, 0.07f);
    glPushMatrix(); glTranslatef(0, 5.9f, 0); cone(1.2f, 3.0f, 8); glPopMatrix();
    glPopMatrix();
}

bool inHouseZone(float x, float z, float m) {
    return fabsf(x) < 12 + m && z < -12 + m && z > -31 - m;
}

void genTrees() {
    rs = 4242;
    while ((int)deadTrees.size() < 40) {
        Spot t;
        t.x = rndr(-38, 38);
        t.z = rndr(-38, 38);
        t.s = rndr(0.85f, 1.3f);
        t.seed = (unsigned)(rnd() * 100000) + 7;
        if (fabsf(t.x) < 3.5f && t.z > -16) continue; // keep the path free
        if (inHouseZone(t.x, t.z, 4)) continue;
        if (fabsf(t.x - 13) < 4 && fabsf(t.z + 6) < 4) continue; // wagon
        if (fabsf(t.x) < 10 && t.z > 32) continue; // gate area
        deadTrees.push_back(t);
    }
    rs = 9191;
    // rows on both sides of the railway
    for (float z = -150; z < 150; z += 5.5f) {
        for (int side = -1; side <= 1; side += 2) {
            for (int row = 0; row < 2; row++) {
                Spot t;
                float off = (row == 0 ? rndr(6.5f, 9.5f) : rndr(12, 19)) * side;
                t.x = trackX(z) + off;
                t.z = z + rndr(-2, 2);
                t.s = rndr(0.9f, 1.6f);
                t.seed = 0;
                pines.push_back(t);
            }
        }
    }
    // forest ring around the fence
    int n = 0;
    while (n < 230) {
        Spot t;
        t.x = rndr(-125, 125);
        t.z = rndr(-125, 125);
        t.s = rndr(0.9f, 1.6f);
        t.seed = 0;
        if (fabsf(t.x) < 47 && fabsf(t.z) < 47) continue;
        if (fabsf(t.x - trackX(t.z)) < 22) continue;
        if (fabsf(t.x) < 8 && t.z > 40) continue; // keep the approach to the gate clear
        pines.push_back(t);
        n++;
    }
}

// ------------------------------------------------------------------ static display list builders
void buildGround() {
    glNewList(base + LG_GROUND, GL_COMPILE);
    rs = 101;
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    for (float x = -144; x < 144; x += 8) {
        for (float z = -144; z < 144; z += 8) {
            if (x >= -48 && x + 8 <= 48 && z >= -48 && z + 8 <= 48) continue;
            float k = 0.85f + 0.3f * rnd();
            glColor3f(0.04f * k, 0.09f * k, 0.05f * k);
            glVertex3f(x, 0, z + 8);
            glVertex3f(x + 8, 0, z + 8);
            glVertex3f(x + 8, 0, z);
            glVertex3f(x, 0, z);
        }
    }
    for (float x = -48; x < 48; x += 2) {
        for (float z = -48; z < 48; z += 2) {
            float k = 0.80f + 0.4f * rnd();
            glColor3f(0.06f * k, 0.14f * k, 0.07f * k);
            glVertex3f(x, 0, z + 2);
            glVertex3f(x + 2, 0, z + 2);
            glVertex3f(x + 2, 0, z);
            glVertex3f(x, 0, z);
        }
    }
    glEnd();
    col(0.07f, 0.17f, 0.08f); // grass tufts (thin cones)
    for (int i = 0; i < 260; i++) {
        float x = rndr(-44, 44), z = rndr(-44, 44);
        if (fabsf(x) < 2.2f && z > -14) continue;
        if (fabsf(x) < 13 && z < -13 && z > -31) continue;
        glPushMatrix();
        glTranslatef(x, 0, z);
        for (int j = 0; j < 3; j++) {
            glPushMatrix();
            glRotatef(rndr(0, 360), 0, 1, 0);
            glTranslatef(0.06f, 0, 0);
            glRotatef(rndr(-14, 14), 0, 0, 1);
            cone(0.035f, rndr(0.4f, 0.8f), 4);
            glPopMatrix();
        }
        glPopMatrix();
    }
    glEndList();
}

void buildPath() {
    glNewList(base + LG_PATH, GL_COMPILE);
    rs = 202;
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    for (float x = -1.5f; x < 1.5f; x += 0.75f) {
        for (float z = -11.5f; z < 60; z += 0.75f) {
            float k = 0.65f + 0.7f * rnd(), j = rndr(-0.02f, 0.02f);
            glColor3f(0.30f * k, 0.30f * k, 0.32f * k);
            glVertex3f(x + 0.03f, 0.02f, z + 0.72f + j);
            glVertex3f(x + 0.72f, 0.02f, z + 0.72f);
            glVertex3f(x + 0.72f, 0.02f, z + 0.03f);
            glVertex3f(x + 0.03f, 0.02f, z + 0.03f - j);
        }
    }
    glEnd();
    glEndList();
}

void pillar(float x, float z) {
    matStone();
    box(x, 1.6f, z, 0.9f, 3.2f, 0.9f);
    box(x, 3.3f, z, 1.15f, 0.2f, 1.15f);
    box(x, 0.25f, z, 1.1f, 0.5f, 1.1f);
    col(0.28f, 0.28f, 0.3f);
    sph(x, 3.75f, z, 0.3f);
}

void gargoyle(float x, float y, float z, float rotY) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotY, 0, 1, 0);
    col(0.18f, 0.18f, 0.20f);
    box(0, 0.3f, 0, 0.5f, 0.6f, 0.4f);
    sph(0, 0.78f, 0.12f, 0.2f, 10);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(s * 0.09f, 0.94f, 0.12f);
        glRotatef(-s * 20, 0, 0, 1);
        cone(0.04f, 0.16f, 6); // horns
        glPopMatrix();
        glPushMatrix();
        glTranslatef(s * 0.25f, 0.4f, -0.12f);
        glRotatef(-s * 55, 0, 0, 1);
        glScalef(1, 1, 0.18f);
        cone(0.32f, 0.75f, 6); // wings
        glPopMatrix();
    }
    glPopMatrix();
}

void woodSection(float x0, float z0, float x1, float z1) {
    float L = sqrtf((x1 - x0) * (x1 - x0) + (z1 - z0) * (z1 - z0));
    int n = (int)(L / 4 + 0.5f);
    float dx = (x1 - x0) / n, dz = (z1 - z0) / n;
    bool alongX = fabsf(dx) > 0.01f;
    matWood();
    for (int i = 0; i <= n; i++) { // posts with cone caps
        glPushMatrix();
        glTranslatef(x0 + dx * i, 0, z0 + dz * i);
        if (rnd() < 0.25f) glRotatef(rndr(-6, 6), alongX ? 0 : 1, 0, alongX ? 1 : 0);
        col(0.24f, 0.16f, 0.09f);
        cyl(0.15f, 0.13f, 2.0f, 8);
        glTranslatef(0, 2.0f, 0);
        cone(0.17f, 0.25f, 8);
        glPopMatrix();
    }
    for (int i = 0; i < n; i++) {
        for (int r = 0; r < 2; r++) {
            glPushMatrix();
            glTranslatef(x0 + dx * (i + 0.5f), r == 0 ? 1.6f : 0.7f, z0 + dz * (i + 0.5f));
            if (rnd() < 0.3f) glRotatef(rndr(-5, 5), alongX ? 0 : 1, 0, alongX ? 1 : 0);
            col(0.22f, 0.15f, 0.09f);
            if (alongX) glScalef(fabsf(dx), 0.1f, 0.06f);
            else glScalef(0.06f, 0.1f, fabsf(dz));
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }
}

void ironFront(float xa, float xb) {
    matIron();
    for (float x = xa; x <= xb + 0.001f; x += 0.25f) {
        box(x, 1.0f, 40, 0.06f, 2.0f, 0.06f);
        glPushMatrix();
        glTranslatef(x, 2.0f, 40);
        cone(0.05f, 0.18f, 4);
        glPopMatrix();
    }
    box((xa + xb) / 2, 1.8f, 40, xb - xa, 0.08f, 0.08f);
    box((xa + xb) / 2, 0.5f, 40, xb - xa, 0.08f, 0.08f);
}

void buildFence() {
    glNewList(base + LG_FENCE, GL_COMPILE);
    rs = 303;
    woodSection(-40, -40, 40, -40); // back
    woodSection(40, -40, 40, 40);   // east
    woodSection(-40, -40, -40, 12); // west (wooden part)
    woodSection(-40, 36, -40, 40);  // west corner
    ironFront(-40, -4.95f);
    ironFront(4.95f, 40);
    // brick wall along the graveyard
    bigBox(-40.15f, 0, 12, -39.85f, 2.2f, 36, 0.30f, 0.13f, 0.10f, 0.16f, 0.5f);
    matStone();
    box(-40, 2.3f, 24, 0.5f, 0.15f, 24.4f);
    // stone pillars: corners, sides of the gate, extra along the front
    float pp[][2] = {
        {-40, -40}, {40, -40}, {-40, 40}, {40, 40},
        {-4.5f, 40}, {4.5f, 40}, {-22, 40}, {22, 40},
        {0, -40}, {40, 0}, {-40, 0}, {-40, 12}, {-40, 36}
    };
    for (unsigned i = 0; i < sizeof(pp) / sizeof(pp[0]); i++) pillar(pp[i][0], pp[i][1]);
    gargoyle(-4.5f, 3.4f, 40, 0);
    gargoyle(4.5f, 3.4f, 40, 0);
    glEndList();
}

void buildLeaf() {
    glNewList(base + LG_LEAF, GL_COMPILE);
    matIron();
    box(2.0f, 0.35f, 0, 4.0f, 0.12f, 0.08f);
    box(2.0f, 2.45f, 0, 4.0f, 0.12f, 0.08f);
    box(0.05f, 1.4f, 0, 0.1f, 2.6f, 0.1f);
    box(3.95f, 1.4f, 0, 0.1f, 2.6f, 0.1f);
    for (float x = 0.3f; x < 3.9f; x += 0.3f) {
        glPushMatrix();
        glTranslatef(x, 0.35f, 0);
        cyl(0.03f, 0.03f, 2.2f, 6);
        glTranslatef(0, 2.2f, 0);
        cone(0.05f, 0.2f, 5);
        glPopMatrix();
    }
    matRust();
    glPushMatrix();
    glTranslatef(2.0f, 1.4f, 0);
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.03, 0.4, 6, 16); // ring ornament
    glPopMatrix();
    glEndList();
}

void tombstone(float x, float z, int type, float ry) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(ry, 0, 1, 0);
    glRotatef(rndr(-7, 7), 0, 0, 1);
    glRotatef(rndr(-5, 5), 1, 0, 0);
    float k = rndr(0.8f, 1.15f);
    col(0.32f * k, 0.32f * k, 0.34f * k);
    if (type == 0) {
        box(0, 0.45f, 0, 0.7f, 0.9f, 0.16f);
        ell(0, 0.9f, 0, 0.35f, 0.2f, 0.08f, 10);
    } else if (type == 1) {
        box(0, 0.4f, 0, 0.6f, 0.8f, 0.14f);
        box(0, 0.83f, 0, 0.66f, 0.07f, 0.18f);
    } else {
        box(0, 0.7f, 0, 0.14f, 1.4f, 0.14f);
        box(0, 1.0f, 0, 0.7f, 0.14f, 0.14f); // cross
    }
    box(0, 0.06f, 0, 0.9f, 0.12f, 0.5f);
    glPopMatrix();
}

void buildGrave() {
    glNewList(base + LG_GRAVE, GL_COMPILE);
    rs = 404;
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 4; c++) {
            float x = -36 + c * 3.8f + rndr(-0.6f, 0.6f);
            float z = 14.5f + r * 3.6f + rndr(-0.5f, 0.5f);
            tombstone(x, z, (int)(rnd() * 3), rndr(-15, 15));
            solid(x - 0.4f, 0, z - 0.3f, x + 0.4f, 1.0f, z + 0.3f);
        }
    }
    glEndList();
}

void buildProps() {
    glNewList(base + LG_PROPS, GL_COMPILE);
    rs = 505;
    // ---- old wagon (R1 / R5) at (13,-6)
    glPushMatrix();
    glTranslatef(13, 0, -6);
    glRotatef(25, 0, 1, 0);
    matWood();
    box(0, 0.95f, 0, 3.2f, 0.12f, 1.5f);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(0, 1.25f, s * 0.75f);
        glRotatef(-s * 12, 1, 0, 0);
        box(0, 0, 0, 3.2f, 0.55f, 0.06f);
        glPopMatrix();
    }
    box(-1.6f, 1.2f, 0, 0.06f, 0.5f, 1.5f);
    box(2.4f, 0.8f, 0, 1.8f, 0.08f, 0.1f); // end board + tongue
    for (int wx = -1; wx <= 1; wx += 2) {
        for (int wz = -1; wz <= 1; wz += 2) {
            if (wx == 1 && wz == 1) continue; // one wheel is missing
            glPushMatrix();
            glTranslatef(wx * 1.0f, 0.55f, wz * 0.9f);
            if (wx == -1 && wz == 1) glRotatef(14, 1, 0, 0); // cracked / leaning wheel
            col(0.20f, 0.13f, 0.08f);
            glutSolidTorus(0.06, 0.5, 8, 18);
            for (int k = 0; k < 4; k++) {
                glPushMatrix();
                glRotatef(k * 45, 0, 0, 1);
                box(0, 0, 0, 1.0f, 0.06f, 0.06f);
                glPopMatrix();
            }
            sph(0, 0, 0, 0.09f, 8);
            glPopMatrix();
        }
    }
    glPopMatrix();
    solid(11.4f, 0, -7.4f, 14.6f, 1.6f, -4.6f);

    // ---- pumpkins
    float pk[][3] = { {-4, 0, -9}, {5.5f, 0, -9.5f}, {-9, 0, -8}, {-33, 0, 15} };
    for (int i = 0; i < 4; i++) {
        col(0.85f, 0.38f, 0.05f);
        ell(pk[i][0], 0.27f, pk[i][2], 0.36f, 0.27f, 0.36f);
        col(0.2f, 0.28f, 0.08f);
        glPushMatrix();
        glTranslatef(pk[i][0], 0.5f, pk[i][2]);
        cyl(0.04f, 0.03f, 0.14f, 6);
        glPopMatrix();
    }

    // ---- lamp posts beside the gate (bulbs are dynamic)
    for (int s = -1; s <= 1; s += 2) {
        matRust();
        glPushMatrix();
        glTranslatef(s * 7.0f, 0, 38);
        col(0.25f, 0.12f, 0.06f);
        cyl(0.2f, 0.12f, 0.5f, 8);
        cyl(0.07f, 0.06f, 3.2f, 8);
        glTranslatef(0, 3.25f, 0);
        cone(0.42f, 0.35f, 8);
        glPopMatrix();
    }

    // ---- blob "shadows" under the wagon and trees (no real shadows in OpenGL 1.x)
    glDisable(GL_LIGHTING);
    glColor3f(0.02f, 0.05f, 0.03f);
    glBegin(GL_QUADS);
    for (unsigned i = 0; i < deadTrees.size(); i++) {
        float x = deadTrees[i].x, z = deadTrees[i].z;
        glVertex3f(x - 1, 0.015f, z + 1);
        glVertex3f(x + 1, 0.015f, z + 1);
        glVertex3f(x + 1, 0.015f, z - 1);
        glVertex3f(x - 1, 0.015f, z - 1);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEndList();
}

void buildTrees() {
    glNewList(base + LG_TREES, GL_COMPILE);
    for (unsigned i = 0; i < deadTrees.size(); i++) {
        glPushMatrix();
        glTranslatef(deadTrees[i].x, 0, deadTrees[i].z);
        deadTree(deadTrees[i].seed, deadTrees[i].s);
        glPopMatrix();
        solid(deadTrees[i].x - 0.35f, 0, deadTrees[i].z - 0.35f,
              deadTrees[i].x + 0.35f, 5, deadTrees[i].z + 0.35f);
    }
    glEndList();

    glNewList(base + LG_PINES, GL_COMPILE);
    for (unsigned i = 0; i < pines.size(); i++) {
        glPushMatrix();
        glTranslatef(pines[i].x, 0, pines[i].z);
        pineTree(pines[i].s);
        glPopMatrix();
    }
    glEndList();
}
