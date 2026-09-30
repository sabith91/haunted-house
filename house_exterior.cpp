#include "house_exterior.h"

// ------------------------------------------------------------------ round tower and leaning turret
void towerAndTurret() {
    // round stone tower, front-left corner (R1)
    float tx = -13.5f, tz = -11.8f;
    solid(tx - 2.7f, 0, tz - 2.7f, tx + 2.7f, 14, tz + 2.7f);
    glPushMatrix();
    glTranslatef(tx, 0, tz);
    matStone();
    col(0.30f, 0.30f, 0.32f);
    cyl(2.9f, 2.8f, 14.0f, 20);
    col(0.22f, 0.22f, 0.24f);
    glPushMatrix();
    glTranslatef(0, 13.6f, 0);
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.2, 2.85, 8, 20);
    glPopMatrix();
    col(0.08f, 0.09f, 0.11f);
    glPushMatrix();
    glTranslatef(0, 14, 0);
    cone(3.4f, 6.0f, 20);
    glPopMatrix(); // slate cone roof
    col(0.06f, 0.06f, 0.07f);
    glPushMatrix();
    glTranslatef(0, 19.8f, 0);
    cone(0.16f, 3.0f, 6);
    glPopMatrix(); // spire
    for (int h = 0; h < 3; h++) {
        for (int a = 0; a < 6; a++) { // arched slits / lit slits
            glPushMatrix();
            glRotatef(a * 60.0f + 20.0f * h, 0, 1, 0);
            glTranslatef(0, 4.5f + h * 3.4f, 2.87f);
            glDisable(GL_LIGHTING);
            if ((a + h) % 4 == 0) glColor3f(1.0f, 0.78f, 0.30f);
            else glColor3f(0.02f, 0.03f, 0.04f);
            glBegin(GL_QUADS);
            glVertex3f(-0.22f, -0.6f, 0); glVertex3f(0.22f, -0.6f, 0);
            glVertex3f(0.22f, 0.5f, 0);  glVertex3f(-0.22f, 0.5f, 0);
            glEnd();
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, 0.5f, 0);
            for (int i = 0; i <= 8; i++) {
                float t = PI * i / 8;
                glVertex3f(0.22f * cosf(t), 0.5f + 0.22f * sinf(t), 0);
            }
            glEnd();
            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }
    glPopMatrix();

    // tilted timber turret, back-right (R5)
    matWood();
    col(0.16f, 0.10f, 0.06f); box(9, 11, -27, 3.0f, 4.0f, 3.0f);
    col(0.20f, 0.13f, 0.08f); box(9, 14.75f, -27, 2.6f, 3.5f, 2.6f);
    col(0.11f, 0.07f, 0.04f);
    box(9, 9.1f, -27, 3.3f, 0.2f, 3.3f);
    box(9, 13.0f, -27, 3.0f, 0.15f, 3.0f);
    box(9, 16.5f, -27, 2.8f, 0.15f, 2.8f);
    glPushMatrix();
    glTranslatef(9, 16.5f, -27);
    glRotatef(7, 0, 0, 1); // leaning top block
    col(0.17f, 0.11f, 0.07f);
    box(0, 1.5f, 0, 2.3f, 3.0f, 2.3f);
    col(0.07f, 0.08f, 0.10f);
    prism(0, 3.0f, 0, 3.0f, 1.8f, 2.9f);
    prismZ(0, 3.0f, 0, 3.0f, 1.8f, 2.9f);
    glPopMatrix();
    window(9, 14.7f, -27, 0, 0.6f, 0.9f, 1, -1);
    window(9, 11.5f, -27, 0, 0.6f, 0.9f, 0, -1);
    window(7.48f, 14.7f, -27, -90, 0.6f, 0.9f, 1, -1);
    solid(7.4f, 9, -28.6f, 10.6f, 20, -25.4f);
}

// ------------------------------------------------------------------ dormer window
void dormer(float x, float z, float rot) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(rot, 0, 1, 0);
    float y = 10.9f;
    matStone();
    col(0.24f, 0.24f, 0.26f);
    box(0, y, 0, 1.4f, 1.4f, 1.2f);
    col(0.08f, 0.09f, 0.11f);
    prismZ(0, y + 0.7f, 0, 1.5f, 0.9f, 1.7f);
    glPushMatrix();
    glTranslatef(0, y, 0.6f);
    glDisable(GL_LIGHTING);
    if (litHash(x, z, 1) == 0) glColor3f(1.0f, 0.78f, 0.30f);
    else glColor3f(0.02f, 0.03f, 0.04f);
    glBegin(GL_QUADS);
    glVertex3f(-0.35f, -0.45f, 0.02f); glVertex3f(0.35f, -0.45f, 0.02f);
    glVertex3f(0.35f, 0.4f, 0.02f);   glVertex3f(-0.35f, 0.4f, 0.02f);
    glEnd();
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glPopMatrix();
}

// ------------------------------------------------------------------ buildHouseExt (LG_EXT)
void buildHouseExt() {
    glNewList(base + LG_EXT, GL_COMPILE);
    rs = 707;
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
    matStone();
    col(0.09f, 0.10f, 0.12f);
    prism(0, 8.9f, -22, 25.6f, 3.1f, 17.8f);
    prismZ(-7.5f, 9.0f, -16.4f, 5.2f, 2.7f, 5.0f);
    prismZ(7.5f, 9.0f, -16.4f, 5.2f, 2.7f, 5.0f);
    dormer(-3, -18, 0);
    dormer(3, -18, 0);
    dormer(-3, -26, 180);
    dormer(3, -26, 180);
    window(-7.5f, 10.0f, -13.8f, 0, 0.8f, 1.0f, 1, -1, true);
    window(7.5f, 10.0f, -13.8f, 0, 0.8f, 1.0f, 0, -1, true);

    float ch[][3] = { {-8, -23, 15.5f}, {9.5f, -20, 14.0f}, {2.5f, -27.5f, 16.2f} };
    for (int i = 0; i < 3; i++) {
        col(0.26f, 0.24f, 0.24f);
        box(ch[i][0], (9.6f + ch[i][2]) / 2, ch[i][1], 1.1f, ch[i][2] - 9.6f, 1.1f);
        col(0.18f, 0.17f, 0.18f);
        box(ch[i][0], ch[i][2] + 0.12f, ch[i][1], 1.45f, 0.25f, 1.45f);
    }

    // ---- porch, steps, front door frame (R1 / R3)
    matStone();
    for (int s = -1; s <= 1; s += 2) {
        col(0.30f, 0.30f, 0.32f);
        glPushMatrix();
        glTranslatef(s * 2.3f, 0, -11.5f);
        cylCap(0.24f, 0.22f, 3.6f, 12);
        glPopMatrix();
    }
    col(0.26f, 0.26f, 0.28f);
    box(0, 3.85f, -11.4f, 5.3f, 0.5f, 0.6f);
    col(0.02f, 0.02f, 0.03f);
    glPushMatrix();
    glTranslatef(0, 3.4f, -11.05f);
    glScalef(1, 0.7f, 0.3f);
    glutSolidTorus(0.1, 1.0, 6, 14);
    glPopMatrix(); // arch lintel
    col(0.09f, 0.10f, 0.12f);
    prismZ(0, 4.1f, -12.4f, 3.8f, 1.5f, 5.8f);
    col(0.26f, 0.26f, 0.28f);
    box(-2.3f, 3.6f, -13.0f, 0.4f, 0.3f, 2.4f);
    box(2.3f, 3.6f, -13.0f, 0.4f, 0.3f, 2.4f);
    for (int k = 1; k <= 5; k++) {
        col(0.28f - 0.01f * k, 0.28f - 0.01f * k, 0.30f - 0.01f * k);
        box(0, 0.03f * k, -11.75f - 0.5f * (k - 1), 4.4f, 0.06f * k, 0.5f);
    }
    col(0.02f, 0.02f, 0.03f);
    solid(-2.6f, 0, -12.6f, -2.0f, 4, -12.0f);
    solid(2.0f, 0, -12.6f, 2.6f, 4, -12.0f);

    // ---- tower + turret
    towerAndTurret();

    // ---- windows: about one third lit, the others dark and cracked
    float wx[] = { -8, 8 };
    float wy[] = { 2.4f, 6.6f };
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            int o = litHash(wx[i], wy[j], -14) == 0 ? 1 : 0;
            window(wx[i], wy[j], -14, 0, 1.0f, 1.6f, o, wy[j] > 5 ? 2 : -1);
        }
    }
    window(-3.6f, 2.6f, -14, 0, 0.9f, 1.4f, 1, 2, true);
    window(3.6f, 2.6f, -14, 0, 0.9f, 1.4f, 0, 2, true);
    window(-3.6f, 6.8f, -14, 0, 0.9f, 1.4f, 0, 2, true);
    window(3.6f, 6.8f, -14, 0, 0.9f, 1.4f, 1, 2, true);

    float sz[] = { -18, -22, -26 };
    for (int j = 0; j < 3; j++) {
        for (int k = 0; k < 2; k++) {
            float y = k == 0 ? 2.4f : 6.6f;
            window(-12, y, sz[j], -90, 1.0f, 1.6f, litHash(-12, y, sz[j]) == 0 ? 1 : 0, k ? 3 : -1);
        }
    }

    float ez[] = { -18, -22, -27.5f };
    for (int j = 0; j < 3; j++) {
        for (int k = 0; k < 2; k++) {
            float y = k == 0 ? 2.4f : 6.6f;
            window(12, y, ez[j], 90, 1.0f, 1.6f, litHash(12, y, ez[j]) == 0 ? 1 : 0, k ? 2 : -1);
        }
    }

    window(-8, 2.4f, -30, 180, 1.0f, 1.6f, 0, -1);
    window(8, 2.4f, -30, 180, 1.0f, 1.6f, 1, -1);
    window(-8, 6.6f, -30, 180, 1.0f, 1.6f, 1, 2);
    window(8, 6.6f, -30, 180, 1.0f, 1.6f, 0, 2);
    window(-3.6f, 6.8f, -30, 180, 0.9f, 1.4f, 0, 2, true);
    window(0, 6.8f, -30, 180, 0.9f, 1.4f, 1, 2, true);
    window(3.6f, 6.8f, -30, 180, 0.9f, 1.4f, 0, 2, true);

    glEndList();
}
