#include "train_railway.h"

// ------------------------------------------------------------------ track path geometry
float trackX(float z) {
    return 55.0f + 8.0f * sinf(z / 25.0f);
}

float trackYaw(float z) {
    return atan2f(8.0f / 25.0f * cosf(z / 25.0f), 1.0f) * DEG;
}

// ------------------------------------------------------------------ static railway list
void buildRail() {
    glNewList(base + LG_RAIL, GL_COMPILE);
    rs = 606;
    for (float z = -170; z < 170; z += 2) { // gravel bed + two rails per 2-unit segment
        float zm = z + 1, xm = trackX(zm);
        glPushMatrix();
        glTranslatef(xm, 0, zm);
        glRotatef(trackYaw(zm), 0, 1, 0);
        col(0.13f, 0.13f, 0.14f);
        box(0, 0.05f, 0, 3.4f, 0.1f, 2.06f);
        matRust();
        col(0.24f, 0.17f, 0.13f);
        box(-0.75f, 0.24f, 0, 0.1f, 0.16f, 2.05f);
        box(0.75f, 0.24f, 0, 0.1f, 0.16f, 2.05f);
        glPopMatrix();
    }
    for (float z = -170; z < 170; z += 0.7f) { // sleepers
        glPushMatrix();
        glTranslatef(trackX(z), 0.15f, z);
        glRotatef(trackYaw(z), 0, 1, 0);
        float k = rndr(0.8f, 1.1f);
        col(0.20f * k, 0.13f * k, 0.08f * k);
        box(0, 0, 0, 2.4f, 0.1f, 0.26f);
        glPopMatrix();
    }
    glEndList();
}

// ------------------------------------------------------------------ dynamic train animation
void wheel(float x, float y, float z, float ang) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(90, 0, 1, 0);
    glRotatef(ang, 0, 0, 1);
    glTranslatef(0, 0, -0.08f);
    col(0.10f, 0.08f, 0.07f);
    gluCylinder(Q, 0.45, 0.45, 0.16, 14, 1);
    gluDisk(Q, 0, 0.45, 14, 1);
    glTranslatef(0, 0, 0.16f);
    gluDisk(Q, 0, 0.45, 14, 1);
    col(0.30f, 0.15f, 0.08f);
    box(0, 0, 0.02f, 0.9f, 0.07f, 0.06f);
    box(0, 0, 0.02f, 0.07f, 0.9f, 0.06f);
    glPopMatrix();
}

float trainHeadZ() {
    return -160.0f + 320.0f * (fmodf(simTime, 30.0f) / 30.0f);
}

void drawTrain() {
    float zh = trainHeadZ(), wa = simTime * 1273.0f;
    float zc[3] = { zh - 4.0f, zh - 13.0f, zh - 22.0f };
    for (int c = 0; c < 3; c++) {
        glPushMatrix();
        glTranslatef(trackX(zc[c]), 0, zc[c]);
        glRotatef(trackYaw(zc[c]), 0, 1, 0);
        matRust();
        col(0.42f, 0.20f, 0.09f);
        box(0, 1.0f, 0, 2.4f, 0.35f, c == 0 ? 8.0f : 8.4f); // chassis
        if (c == 0) { // locomotive
            col(0.44f, 0.21f, 0.09f);
            box(0, 2.1f, 0.5f, 2.6f, 2.4f, 5.0f);
            box(0, 1.6f, 3.6f, 2.2f, 1.6f, 1.6f); // nose
            col(0.38f, 0.17f, 0.08f);
            box(0, 3.0f, -2.5f, 2.8f, 3.2f, 2.6f); // cab
            col(0.16f, 0.09f, 0.06f);
            prism(0, 4.6f, -2.5f, 2.9f, 0.6f, 2.8f);
            col(0.85f, 0.55f, 0.15f);
            glNormal3f(1, 0, 0); // orange patches
            for (int s = -1; s <= 1; s += 2) {
                glBegin(GL_QUADS);
                float x = s * 1.32f;
                glVertex3f(x, 1.6f, 1.0f);  glVertex3f(x, 1.6f, 2.4f);
                glVertex3f(x, 2.4f, 2.4f);  glVertex3f(x, 2.4f, 1.0f);
                glVertex3f(x, 2.5f, -0.6f); glVertex3f(x, 2.5f, 0.3f);
                glVertex3f(x, 3.1f, 0.3f);  glVertex3f(x, 3.1f, -0.6f);
                glEnd();
            }
            glDisable(GL_LIGHTING);
            glColor3f(0.03f, 0.04f, 0.05f); // cab windows
            for (int s = -1; s <= 1; s += 2) {
                glBegin(GL_QUADS);
                float x = s * 1.42f;
                glVertex3f(x, 3.1f, -3.3f); glVertex3f(x, 3.1f, -1.7f);
                glVertex3f(x, 4.1f, -1.7f); glVertex3f(x, 4.1f, -3.3f);
                glEnd();
            }
            glColor3f(1.0f, 0.95f, 0.7f);
            sph(0, 2.0f, 4.45f, 0.28f, 10); // headlight
            glEnable(GL_LIGHTING);
            col(0.20f, 0.10f, 0.06f);
            box(0, 3.9f, 1.0f, 0.5f, 0.9f, 0.5f); // chimney
            float wz[4] = { -2.7f, -0.9f, 1.3f, 3.0f };
            for (int i = 0; i < 4; i++) {
                for (int s = -1; s <= 1; s += 2) {
                    wheel(s * 1.1f, 0.75f, wz[i], wa * (i == 3 ? 0.7f : 1));
                }
            }
            col(0.25f, 0.14f, 0.10f);
            for (int s = -1; s <= 1; s += 2) {
                box(s * 1.28f, 0.75f, -0.1f, 0.05f, 0.09f, 4.0f);
            }
        } else { // carriages
            col(0.36f, 0.16f, 0.09f);
            box(0, 2.4f, 0, 2.8f, 2.8f, 8.2f);
            col(0.20f, 0.10f, 0.07f);
            box(0, 3.9f, 0, 2.6f, 0.2f, 8.2f);
            glDisable(GL_LIGHTING);
            glColor3f(0.03f, 0.04f, 0.05f);
            for (int s = -1; s <= 1; s += 2) {
                for (int w = 0; w < 4; w++) {
                    float x = s * 1.42f, z = -3.0f + w * 2.0f;
                    glBegin(GL_QUADS);
                    glVertex3f(x, 2.6f, z - 0.6f); glVertex3f(x, 2.6f, z + 0.6f);
                    glVertex3f(x, 3.5f, z + 0.6f); glVertex3f(x, 3.5f, z - 0.6f);
                    glEnd();
                }
            }
            glEnable(GL_LIGHTING);
            for (int i = 0; i < 4; i++) {
                for (int s = -1; s <= 1; s += 2) {
                    wheel(s * 1.1f, 0.75f, (i < 2 ? -3.4f : 3.4f) + (i % 2) * 0.9f, wa);
                }
            }
        }
        glPopMatrix();
    }
}
