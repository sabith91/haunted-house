#include "house_interior.h"

// ------------------------------------------------------------------ interior furniture & decor
void portrait(float x, float y, float z, float rotY, float w, float h, float tilt) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotY, 0, 1, 0);
    glRotatef(tilt, 0, 0, 1);
    col(0.30f, 0.20f, 0.08f);
    box(0, 0, 0, w, h, 0.07f);
    col(0.05f, 0.06f, 0.05f);
    glNormal3f(0, 0, 1);
    glBegin(GL_QUADS);
    glVertex3f(-w / 2 + 0.08f, -h / 2 + 0.08f, 0.04f);
    glVertex3f(w / 2 - 0.08f, -h / 2 + 0.08f, 0.04f);
    glVertex3f(w / 2 - 0.08f, h / 2 - 0.08f, 0.04f);
    glVertex3f(-w / 2 + 0.08f, h / 2 - 0.08f, 0.04f);
    glEnd();
    col(0.34f, 0.31f, 0.26f); // pale face
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, h * 0.12f, 0.045f);
    for (int i = 0; i <= 14; i++) {
        float a = 2 * PI * i / 14;
        glVertex3f(w * 0.17f * cosf(a), h * 0.12f + h * 0.2f * sinf(a), 0.045f);
    }
    glEnd();
    col(0.10f, 0.09f, 0.08f); // shoulders
    glBegin(GL_QUADS);
    glVertex3f(-w * 0.32f, -h / 2 + 0.08f, 0.045f);
    glVertex3f(w * 0.32f, -h / 2 + 0.08f, 0.045f);
    glVertex3f(w * 0.2f, -h * 0.12f, 0.045f);
    glVertex3f(-w * 0.2f, -h * 0.12f, 0.045f);
    glEnd();
    glPopMatrix();
}

void bed(float x, float y, float z, float rot, float wid, float len,
         float fr, float fg, float fb, float mr, float mg, float mb) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rot, 0, 1, 0);
    col(fr, fg, fb);
    box(0, 0.25f, 0, wid, 0.2f, len);
    box(0, 0.75f, -len / 2 + 0.05f, wid, 0.9f, 0.1f);
    box(0, 0.5f, len / 2 - 0.05f, wid, 0.5f, 0.08f);
    for (int a = -1; a <= 1; a += 2) {
        for (int b = -1; b <= 1; b += 2) {
            box(a * (wid / 2 - 0.05f), 0.1f, b * (len / 2 - 0.05f), 0.1f, 0.2f, 0.1f);
        }
    }
    col(mr, mg, mb);
    box(0, 0.45f, 0.02f, wid - 0.1f, 0.2f, len - 0.15f);
    col(mr * 1.2f, mg * 1.2f, mb * 1.2f);
    ell(0, 0.62f, -len / 2 + 0.35f, wid * 0.32f, 0.09f, 0.24f);
    glPopMatrix();
}

void ironBed(float x, float y, float z, float rot) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rot, 0, 1, 0);
    matIron();
    for (int a = -1; a <= 1; a += 2) {
        glPushMatrix(); glTranslatef(a * 0.5f, 0, -1.02f); cyl(0.03f, 0.03f, 1.1f, 6); glPopMatrix();
        glPushMatrix(); glTranslatef(a * 0.5f, 0, 1.02f);  cyl(0.03f, 0.03f, 0.7f, 6); glPopMatrix();
        for (float t = -0.4f; t <= 0.41f; t += 0.2f) {
            glPushMatrix(); glTranslatef(t, 0.25f, -1.02f); cyl(0.015f, 0.015f, 0.8f, 5); glPopMatrix();
        }
    }
    box(0, 1.0f, -1.02f, 1.0f, 0.04f, 0.04f);
    box(0, 0.65f, 1.02f, 1.0f, 0.04f, 0.04f);
    box(0, 0.35f, 0, 0.96f, 0.03f, 2.0f);
    col(0.36f, 0.34f, 0.30f);
    box(0, 0.45f, 0, 0.9f, 0.14f, 1.95f);
    col(0.42f, 0.40f, 0.36f);
    ell(0, 0.58f, -0.75f, 0.28f, 0.07f, 0.2f);
    glPopMatrix();
}

void dresser(float x, float y, float z) { // faces +X
    glPushMatrix();
    glTranslatef(x, y, z);
    matWood();
    col(0.22f, 0.14f, 0.08f);
    box(0, 0.5f, 0, 0.9f, 1.0f, 1.6f);
    box(0, 1.03f, 0, 1.0f, 0.06f, 1.7f);
    col(0.10f, 0.07f, 0.04f);
    for (int i = 0; i < 3; i++) {
        box(0.46f, 0.2f + i * 0.3f, 0, 0.03f, 0.02f, 1.5f);
        col(0.6f, 0.5f, 0.25f);
        sph(0.49f, 0.32f + i * 0.3f - 0.12f, -0.3f, 0.035f, 6);
        sph(0.49f, 0.32f + i * 0.3f - 0.12f, 0.3f, 0.035f, 6);
        col(0.10f, 0.07f, 0.04f);
    }
    glPopMatrix();
}

void radiator(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    matIron();
    col(0.16f, 0.15f, 0.14f);
    for (int i = 0; i < 9; i++) {
        box(0, 0.4f, -0.5f + i * 0.125f, 0.12f, 0.6f, 0.06f);
    }
    box(0, 0.72f, 0, 0.14f, 0.05f, 1.15f);
    box(0, 0.1f, 0, 0.14f, 0.05f, 1.15f);
    glPopMatrix();
}

void candelabraBase(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    col(0.45f, 0.36f, 0.15f);
    cyl(0.12f, 0.06f, 0.08f, 8);
    cyl(0.03f, 0.03f, 0.5f, 6);
    box(0, 0.5f, 0, 0.55f, 0.03f, 0.03f);
    col(0.85f, 0.82f, 0.7f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.25f, 0.5f, 0);
        cyl(0.025f, 0.025f, 0.16f, 6);
        glPopMatrix();
    }
    glPopMatrix();
}

// ------------------------------------------------------------------ buildHouseInt (LG_INT)
void buildHouseInt() {
    glNewList(base + LG_INT, GL_COMPILE);
    rs = 808;
    // colours
    const float hr = 0.17f, hg = 0.21f, hb = 0.20f; // hall walls: dark green-grey (R8)

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
    wallBox(6.2f, F1, -25.2f, 11.8f, WH, -24.8f, 0.14f, 0.15f, 0.18f, 0.1f); // partition B | C

    // ---- upstairs floor slabs, ceiling
    bigBox(-5.8f, 4.2f, -29.8f, 5.8f, F1, -21.3f, 0.20f, 0.13f, 0.08f, 0.22f, 0.5f);
    bigBox(-11.8f, 4.2f, -29.8f, -6.2f, F1, -14.2f, 0.20f, 0.13f, 0.08f, 0.22f, 0.5f);
    bigBox(6.2f, 4.2f, -29.8f, 11.8f, F1, -14.2f, 0.20f, 0.13f, 0.08f, 0.22f, 0.5f);
    bigBox(-12.2f, WH, -30.2f, 12.2f, WH + 0.1f, -13.8f, 0.12f, 0.09f, 0.07f, 0.12f, 1.0f);
    for (float z = -16.5f; z > -29; z -= 3) {
        col(0.10f, 0.07f, 0.05f);
        box(0, WH - 0.15f, z, 11.6f, 0.3f, 0.3f); // ceiling beams
    }

    // ---- ground floor of the hall: alternating dark / lighter planks, some missing or raised
    col(0.02f, 0.02f, 0.02f);
    quadUp(-5.8f, -29.8f, 5.8f, -14.2f, F0 - 0.02f);
    for (int p = 0; p < 24; p++) {
        for (int s = 0; s < 8; s++) {
            float x0 = -5.8f + p * 0.4833f, z0 = -29.8f + s * 1.95f;
            int r = (int)(rnd() * 40);
            if (r == 0) continue; // missing plank
            float k = (p % 2 ? 0.75f : 1.05f) * rndr(0.85f, 1.1f);
            float dy = (r == 1) ? 0.06f : 0.0f;
            col(0.22f * k, 0.14f * k, 0.08f * k);
            quadUp(x0 + 0.02f, z0, x0 + 0.46f, z0 + 1.95f, F0 + dy);
        }
    }

    // ---- linings (dark green-grey wallpaper, peeling patches)
    lining(-5.8f, F0, -29.8f, 5.8f, WH, -29.76f, hr * 1.1f, hg * 1.1f, hb * 1.1f);
    for (int i = 0; i < 9; i++) { // peeling patches
        float px0 = rndr(-5.5f, 4.5f), py0 = rndr(0.8f, 7.5f);
        col(0.10f, 0.12f, 0.11f);
        glNormal3f(0, 0, 1);
        glBegin(GL_QUADS);
        glVertex3f(px0, py0, -29.75f);
        glVertex3f(px0 + rndr(0.5f, 1.2f), py0 + 0.1f, -29.75f);
        glVertex3f(px0 + 0.8f, py0 + rndr(0.6f, 1.4f), -29.75f);
        glVertex3f(px0 + 0.1f, py0 + 0.5f, -29.75f);
        glEnd();
    }

    // ---- staircase: 14 stacked cubes (rise 0.3, run 0.45, width 4) in the west half of the hall
    for (int i = 0; i < 14; i++) {
        float top = F0 + 0.3f * (i + 1), z1 = -15.0f - 0.45f * i, z0 = z1 - 0.45f;
        float k = (i % 2) ? 0.9f : 1.1f;
        bigBox(-5.8f, 0, z0, -2.0f, top, z1, 0.20f * k, 0.13f * k, 0.08f * k, 0.10f, 0.5f);
    }

    // ---- balustrade along the stairs (posts every step + sloped handrail), newel posts
    matWood();
    col(0.16f, 0.10f, 0.06f);
    for (int i = 0; i < 14; i++) {
        float top = F0 + 0.3f * (i + 1), z = -15.0f - 0.45f * i - 0.22f;
        glPushMatrix();
        glTranslatef(-2.05f, top, z);
        cyl(0.03f, 0.03f, 0.9f, 6);
        glPopMatrix();
    }
    {
        float rise = F1 - F0, run = 6.3f, len = sqrtf(rise * rise + run * run);
        glPushMatrix();
        glTranslatef(-2.05f, F0 + rise / 2 + 1.0f, -15.0f - run / 2);
        glRotatef(atan2f(rise, run) * DEG, 1, 0, 0);
        box(0, 0, 0, 0.1f, 0.09f, len);
        glPopMatrix();
    }
    float nz[] = { -15.0f, -21.3f };
    float ny[] = { F0, F1 };
    for (int i = 0; i < 2; i++) {
        col(0.13f, 0.08f, 0.05f);
        box(-2.05f, ny[i] + 0.65f, nz[i], 0.2f, 1.3f, 0.2f);
        col(0.2f, 0.13f, 0.08f);
        sph(-2.05f, ny[i] + 1.4f, nz[i], 0.14f, 8);
    }
    solid(-2.15f, F0, -21.4f, -1.95f, F1 + 1.2f, -14.9f);

    // ---- gallery rail (front edge of the upstairs gallery)
    matWood();
    col(0.16f, 0.10f, 0.06f);
    for (float x = -1.9f; x <= 6.0f; x += 0.3f) {
        glPushMatrix();
        glTranslatef(x, F1, -21.3f);
        cyl(0.03f, 0.03f, 0.9f, 6);
        glPopMatrix();
    }
    box(2.0f, F1 + 0.95f, -21.3f, 8.0f, 0.09f, 0.11f);
    col(0.13f, 0.08f, 0.05f);
    box(6.0f, F1 + 0.65f, -21.3f, 0.2f, 1.3f, 0.2f);
    sph(6.0f, F1 + 1.4f, -21.3f, 0.14f, 8);
    solid(-2.1f, F1, -21.45f, 6.1f, F1 + 1.2f, -21.15f);

    // ---- side tables under candelabras
    col(0.18f, 0.11f, 0.07f);
    box(5.0f, F0 + 0.4f, -15.2f, 1.2f, 0.8f, 0.5f);
    box(5.0f, F0 + 0.4f, -27.5f, 1.2f, 0.8f, 0.5f);
    solid(4.4f, F0, -15.5f, 5.6f, F0 + 0.8f, -14.9f);
    solid(4.4f, F0, -27.8f, 5.6f, F0 + 0.8f, -27.2f);

    // ---- portraits (six, one crooked)
    portrait(-5.75f, 2.9f, -17.6f, 90, 0.9f, 1.3f, 0);
    portrait(-5.75f, 4.0f, -20.3f, 90, 0.9f, 1.3f, 0);
    portrait(5.75f, 2.7f, -16.6f, -90, 0.9f, 1.3f, 0);
    portrait(5.75f, 2.7f, -19.6f, -90, 0.9f, 1.3f, 7); // the crooked one
    portrait(-2.2f, 6.9f, -29.72f, 0, 1.1f, 1.5f, 0);
    portrait(2.2f, 6.9f, -29.72f, 0, 1.1f, 1.5f, 0);

    // ================================================================= BEDROOM A (child room, R9)
    lining(-11.8f, F1, -29.8f, -6.2f, WH, -29.76f, 0.20f, 0.16f, 0.12f);
    lining(-11.8f, F1, -14.24f, -6.2f, WH, -14.2f, 0.20f, 0.16f, 0.12f);
    for (int k = 0; k < 9; k++) {
        float s = rndr(0.8f, 1.15f);
        col(0.22f * s, 0.15f * s, 0.10f * s);
        box(-11.76f, F1 + 0.25f + k * 0.5f, -22, 0.04f, 0.46f, 15.6f); // plank wall
    }
    bed(-9.5f, F1, -28.7f, 0, 1.6f, 2.2f, 0.18f, 0.12f, 0.07f, 0.40f, 0.36f, 0.30f);
    solid(-10.4f, F1, -29.8f, -8.6f, F1 + 1.0f, -27.5f);
    dresser(-11.35f, F1, -24.0f);
    solid(-11.8f, F1, -24.9f, -10.85f, F1 + 1.1f, -23.1f);
    radiator(-11.6f, F1, -18.0f);
    // rug + open book on the floor
    col(0.20f, 0.06f, 0.06f);
    quadUp(-10.2f, -21.8f, -7.6f, -19.6f, F1 + 0.012f);
    col(0.55f, 0.52f, 0.42f);
    quadUp(-9.2f, -16.7f, -8.7f, -16.4f, F1 + 0.02f);
    col(0.5f, 0.47f, 0.38f);
    quadUp(-8.7f, -16.7f, -8.2f, -16.4f, F1 + 0.02f);
    // hanging lamp (bulb is emissive, drawn dynamically)
    col(0.10f, 0.10f, 0.10f);
    glPushMatrix(); glTranslatef(-9, WH - 1.3f, -22); cyl(0.01f, 0.01f, 1.3f, 4); glPopMatrix();
    col(0.15f, 0.25f, 0.12f);
    glPushMatrix(); glTranslatef(-9, WH - 1.5f, -22); cone(0.38f, 0.28f, 12); glPopMatrix();

    // ================================================================= BEDROOM B (draped room, R10)
    lining(6.2f, F1, -29.8f, 11.8f, WH, -29.76f, 0.09f, 0.11f, 0.15f);
    lining(11.76f, F1, -29.8f, 11.8f, WH, -25.2f, 0.09f, 0.11f, 0.15f);
    lining(6.2f, F1, -25.24f, 11.8f, WH, -25.2f, 0.09f, 0.11f, 0.15f);
    bed(10.0f, F1, -28.7f, 0, 2.0f, 2.2f, 0.06f, 0.05f, 0.06f, 0.08f, 0.09f, 0.12f);
    solid(9.0f, F1, -29.8f, 11.0f, F1 + 1.0f, -27.5f);
    col(0.15f, 0.10f, 0.07f);
    box(8.2f, F1 + 0.3f, -29.3f, 0.8f, 0.6f, 0.7f);
    solid(7.8f, F1, -29.65f, 8.6f, F1 + 0.6f, -28.95f);
    col(0.85f, 0.82f, 0.7f);
    glPushMatrix(); glTranslatef(8.2f, F1 + 0.6f, -29.3f); cyl(0.04f, 0.04f, 0.18f, 8); glPopMatrix();
    for (int i = 0; i < 6; i++) { // rags
        col(0.05f, 0.05f, 0.06f);
        float x = rndr(6.8f, 10.8f), z = rndr(-27, -25.6f);
        glNormal3f(0, 1, 0);
        glBegin(GL_QUADS);
        glVertex3f(x, F1 + 0.015f, z);
        glVertex3f(x + rndr(0.4f, 1.0f), F1 + 0.015f, z + 0.1f);
        glVertex3f(x + 0.6f, F1 + 0.015f, z + rndr(0.4f, 0.8f));
        glVertex3f(x - 0.1f, F1 + 0.015f, z + 0.5f);
        glEnd();
    }

    // ================================================================= BEDROOM C (abandoned, R11)
    lining(6.2f, F1, -14.24f, 11.8f, WH, -14.2f, 0.24f, 0.22f, 0.18f);
    lining(6.2f, F1, -24.8f, 11.8f, WH, -24.76f, 0.24f, 0.22f, 0.18f);
    for (float z = -24.6f; z < -14.3f; z += 0.5f) { // striped peeling wallpaper on the east wall
        int st = ((int)((z + 30) / 0.5f)) % 2;
        col(st ? 0.34f : 0.26f, st ? 0.30f : 0.23f, st ? 0.22f : 0.17f);
        box(11.74f, F1 + 2.2f, z + 0.25f, 0.04f, 4.4f, 0.48f);
    }
    ironBed(10.7f, F1, -23.3f, -90);
    solid(9.6f, F1, -23.8f, 11.8f, F1 + 1.0f, -22.8f);
    ironBed(10.7f, F1, -19.5f, -90);
    solid(9.6f, F1, -20.0f, 11.8f, F1 + 1.0f, -19.0f);
    matWood();
    col(0.13f, 0.09f, 0.06f);
    box(11.4f, F1 + 1.1f, -15.9f, 0.7f, 2.2f, 1.6f);
    col(0.08f, 0.05f, 0.03f);
    box(11.03f, F1 + 1.1f, -15.9f, 0.03f, 2.0f, 0.02f);
    col(0.6f, 0.5f, 0.25f);
    sph(11.02f, F1 + 1.1f, -15.7f, 0.04f, 6);
    sph(11.02f, F1 + 1.1f, -16.1f, 0.04f, 6);
    solid(11.05f, F1, -16.7f, 11.8f, F1 + 2.2f, -15.1f);

    float hl[][2] = { {8.5f, -17.5f}, {9.2f, -15.8f}, {7.2f, -19.6f} }; // holes in the floor
    for (int i = 0; i < 3; i++) {
        col(0.01f, 0.01f, 0.01f);
        quadUp(hl[i][0] - 0.6f, hl[i][1] - 0.5f, hl[i][0] + 0.6f, hl[i][1] + 0.5f, F1 + 0.016f);
    }
    col(0.32f, 0.30f, 0.27f);
    quadUp(7.0f, -23.0f, 9.5f, -21.0f, F1 + 0.011f); // dust on the floor
    glEndList();
}
