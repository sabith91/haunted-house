#include "lighting_sky.h"

// ------------------------------------------------------------------ sky dome & starfield
float stars[200][3];

void initStars() {
    rs = 31337;
    for (int i = 0; i < 200; i++) { // star field on a dome
        float a = rndr(0, 2 * PI), e = rndr(0.12f, 1.4f);
        stars[i][0] = 150 * cosf(e) * cosf(a);
        stars[i][1] = 150 * sinf(e);
        stars[i][2] = 150 * cosf(e) * sinf(a);
    }
}

void drawSky(float cx, float cy, float cz) {
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    float R = 190, f = flashV;
    float hor[3] = { fogBase[0] + f * 0.5f, fogBase[1] + f * 0.5f, fogBase[2] + f * 0.55f };
    float mid[3] = { 0.03f + f * 0.4f, 0.05f + f * 0.42f, 0.09f + f * 0.5f };
    float top[3] = { 0.01f + f * 0.3f, 0.015f + f * 0.32f, 0.04f + f * 0.4f };
    for (int s = 0; s < 4; s++) {
        glPushMatrix();
        glRotatef(s * 90.0f, 0, 1, 0);
        glBegin(GL_QUADS);
        glColor3fv(hor); glVertex3f(-R, -40, -R); glVertex3f(R, -40, -R);
        glColor3fv(mid); glVertex3f(R, 60, -R);   glVertex3f(-R, 60, -R);
        glColor3fv(mid); glVertex3f(-R, 60, -R);  glVertex3f(R, 60, -R);
        glColor3fv(top); glVertex3f(R, 190, -R);  glVertex3f(-R, 190, -R);
        glEnd();
        glPopMatrix();
    }
    glColor3fv(top);
    glBegin(GL_QUADS);
    glVertex3f(-R, 190, -R); glVertex3f(R, 190, -R);
    glVertex3f(R, 190, R);   glVertex3f(-R, 190, R);
    glEnd();

    // stars
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++) {
        float t = 0.55f + 0.45f * sinf(simTime * 1.5f + i);
        glColor3f(0.75f * t, 0.8f * t, 0.9f * t);
        glVertex3fv(stars[i]);
    }
    glEnd();

    // moon (near white) with a soft halo
    float md[3] = { -0.30f, 0.36f, -0.88f };
    float ml = sqrtf(md[0] * md[0] + md[1] * md[1] + md[2] * md[2]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glTranslatef(md[0] / ml * 150, md[1] / ml * 150, md[2] / ml * 150);
    glColor4f(0.5f, 0.6f, 0.8f, 0.05f); glutSolidSphere(22, 40, 40);
    glColor4f(0.6f, 0.7f, 0.9f, 0.08f); glutSolidSphere(15, 40, 40);
    glColor3f(0.96f, 0.96f, 0.90f);     glutSolidSphere(9, 40, 40);
    glColor3f(0.78f, 0.78f, 0.74f);
    glPushMatrix();
    glTranslatef(-2.5f, 2.0f, 8.0f);
    glutSolidSphere(1.6, 8, 8);
    glTranslatef(4.0f, -3.5f, 0.3f);
    glutSolidSphere(1.1, 8, 8);
    glPopMatrix();
    glPopMatrix();

    // slow drifting cloud layers
    for (int i = 0; i < 9; i++) {
        float ox = fmodf(simTime * (1.2f + 0.2f * i) + i * 90, 400.0f) - 200, oz = -160 + i * 40, y = 70 + 8 * (i % 3);
        glColor4f(0.10f + f * 0.5f, 0.13f + f * 0.5f, 0.16f + f * 0.5f, 0.30f);
        glBegin(GL_QUADS);
        glVertex3f(ox - 90, y, oz - 30);
        glVertex3f(ox + 90, y, oz - 30);
        glVertex3f(ox + 90, y, oz + 30);
        glVertex3f(ox - 90, y, oz + 30);
        glEnd();
    }
    glDisable(GL_BLEND);
    glPopMatrix();
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

// ------------------------------------------------------------------ fixed-function lighting
void setPointLight(GLenum L, float x, float y, float z, float r, float g, float b, float c, float l, float q) {
    GLfloat pos[4] = { x, y, z, 1 }, dif[4] = { r, g, b, 1 }, amb[4] = { 0, 0, 0, 1 }, spc[4] = { r * 0.3f, g * 0.3f, b * 0.3f, 1 };
    glLightfv(L, GL_POSITION, pos);
    glLightfv(L, GL_DIFFUSE, dif);
    glLightfv(L, GL_AMBIENT, amb);
    glLightfv(L, GL_SPECULAR, spc);
    glLightf(L, GL_CONSTANT_ATTENUATION, c);
    glLightf(L, GL_LINEAR_ATTENUATION, l);
    glLightf(L, GL_QUADRATIC_ATTENUATION, q);
    glLightf(L, GL_SPOT_CUTOFF, 180.0f);
}

bool closeTo(float x, float y, float z, float R, float ex, float ey, float ez) {
    float dx = x - ex, dy = y - ey, dz = z - ez;
    return dx * dx + dy * dy + dz * dz < R * R;
}

void setupWorldLights(float ex, float ey, float ez) {
    // GL_LIGHT0 moon: directional (w = 0), cold and dim
    GLfloat moonPos[] = { -0.4f, 1.0f, -0.3f, 0.0f };
    float mk = (1.0f - 0.85f * indoorAmt) + flashV * 1.5f;
    GLfloat moonDiff[] = { 0.30f * mk, 0.38f * mk, 0.55f * mk, 1.0f }, zero[] = { 0, 0, 0, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, moonPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, moonDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, zero);
    glLightfv(GL_LIGHT0, GL_SPECULAR, moonDiff);
    glEnable(GL_LIGHT0);

    float cf = fChand; // GL_LIGHT2 chandelier
    setPointLight(GL_LIGHT2, 2.0f + 0.1f * sinf(1.1f * simTime), WH - 2.4f, -17.6f,
                  1.5f * cf, 1.0f * cf, 0.45f * cf, 0.5f, 0.04f, 0.018f);
    if (closeTo(2, 6, -17.6f, 20, ex, ey, ez)) glEnable(GL_LIGHT2); else glDisable(GL_LIGHT2);

    // GL_LIGHT3 bedroom candle (strong flicker)
    setPointLight(GL_LIGHT3, 8.2f, F1 + 1.0f, -29.3f,
                  1.4f * fCandle, 0.65f * fCandle, 0.18f * fCandle, 0.4f, 0.10f, 0.08f);
    if (closeTo(8.2f, 6, -29.3f, 16, ex, ey, ez)) glEnable(GL_LIGHT3); else glDisable(GL_LIGHT3);

    // GL_LIGHT4 lamp post near the gate
    setPointLight(GL_LIGHT4, -7, 3.4f, 38,
                  1.3f * fLamp, 1.0f * fLamp, 0.45f * fLamp, 0.6f, 0.04f, 0.012f);
    if (closeTo(-7, 3, 38, 45, ex, ey, ez)) glEnable(GL_LIGHT4); else glDisable(GL_LIGHT4);

    // GL_LIGHT5 train headlight: spotlight that moves with the train
    {
        float zh = trainHeadZ(), zc = zh - 4.0f, yw = trackYaw(zc) / DEG;
        GLfloat pos[4] = { trackX(zc) + sinf(yw) * 4.5f, 2.0f, zc + cosf(yw) * 4.5f, 1 };
        GLfloat dir[3] = { sinf(yw), -0.04f, cosf(yw) };
        GLfloat dif[4] = { 1.5f, 1.4f, 1.0f, 1 };
        glLightfv(GL_LIGHT5, GL_POSITION, pos);
        glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, dir);
        glLightfv(GL_LIGHT5, GL_DIFFUSE, dif);
        glLightfv(GL_LIGHT5, GL_AMBIENT, zero);
        glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 6.0f);
        glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.02f);
        glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.0015f);
        if (closeTo(pos[0], 2, pos[2], 110, ex, ey, ez)) glEnable(GL_LIGHT5); else glDisable(GL_LIGHT5);
    }

    // GL_LIGHT6 warm window glow in front of the house
    setPointLight(GL_LIGHT6, 0, 4.5f, -10.5f, 0.55f, 0.38f, 0.14f, 0.6f, 0.05f, 0.03f);
    if (closeTo(0, 4, -10, 30, ex, ey, ez)) glEnable(GL_LIGHT6); else glDisable(GL_LIGHT6);

    // GL_LIGHT7 ghost glow (follows the ghost)
    {
        float gx, gy, gz;
        ghostPos(gx, gy, gz);
        setPointLight(GL_LIGHT7, gx, gy, gz, 0.35f, 0.5f, 0.7f, 0.6f, 0.06f, 0.04f);
        if (closeTo(gx, gy, gz, 26, ex, ey, ez)) glEnable(GL_LIGHT7); else glDisable(GL_LIGHT7);
    }
}

// ------------------------------------------------------------------ 2D heads-up display
void text(float x, float y, const char* s, void* font) {
    glRasterPos2f(x, y);
    for (; *s; s++) glutBitmapCharacter(font, *s);
}

const char* zoneName() {
    if (fabsf(px) < 12 && pz < -14 && pz > -30) return feet > 3.0f ? "Upstairs" : "Mansion hall";
    if (px > 42) return "Railway / forest";
    if (fabsf(px) > 40 || fabsf(pz) > 40) return "Outside the fence";
    return "Haunted grounds";
}

void drawHUD() {
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    char buf[160];
    glColor3f(0.85f, 0.75f, 0.45f);
    snprintf(buf, sizeof buf, "Interactive 3D Haunted House  |  %s  |  Flashlight: %s  |  %.0f fps",
             zoneName(), flashOn ? "ON" : "off", fpsVal);
    text(12, winH - 20, buf);
    if (hudOn) {
        glColor3f(0.6f, 0.65f, 0.7f);
        text(12, 46, "WASD move (Shift run)   Mouse/arrows look   F flashlight   E door/gate   L lightning");
        text(12, 28, "1 gate   2 hall   3 bedroom   4 railway   M mouse look   H hide help   Esc quit");
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}
