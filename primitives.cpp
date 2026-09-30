#include "primitives.h"

void box(float cx, float cy, float cz, float w, float h, float d) {
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glScalef(w, h, d);
    glutSolidCube(1.0);
    glPopMatrix();
}

void cyl(float r0, float r1, float h, int sl) {
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(Q, r0, r1, h, sl, 1);
    glPopMatrix();
}

void cylCap(float r0, float r1, float h, int sl) {
    cyl(r0, r1, h, sl);
    glPushMatrix();
    glTranslatef(0, h, 0);
    glRotatef(-90, 1, 0, 0);
    gluDisk(Q, 0, r1, sl, 1);
    glPopMatrix();
}

void cone(float r, float h, int sl) {
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(r, h, sl, 1);
    glPopMatrix();
}

void sph(float x, float y, float z, float r, int sl) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(r, sl, sl);
    glPopMatrix();
}

void ell(float x, float y, float z, float rx, float ry, float rz, int sl) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(rx, ry, rz);
    glutSolidSphere(1.0, sl, sl);
    glPopMatrix();
}

// triangular prism: ridge along X (length w), cross-section d wide, h high, base on y0
void prism(float cx, float y0, float cz, float w, float h, float d) {
    float hw = w / 2, hd = d / 2, l = sqrtf(h * h + hd * hd), ny = hd / l, nz = h / l;
    glPushMatrix();
    glTranslatef(cx, y0, cz);
    glBegin(GL_QUADS);
    glNormal3f(0, ny, nz);
    glVertex3f(-hw, 0, hd);  glVertex3f(hw, 0, hd);  glVertex3f(hw, h, 0);  glVertex3f(-hw, h, 0);
    glNormal3f(0, ny, -nz);
    glVertex3f(hw, 0, -hd); glVertex3f(-hw, 0, -hd); glVertex3f(-hw, h, 0); glVertex3f(hw, h, 0);
    glNormal3f(0, -1, 0);
    glVertex3f(-hw, 0, hd); glVertex3f(-hw, 0, -hd); glVertex3f(hw, 0, -hd); glVertex3f(hw, 0, hd);
    glEnd();
    glBegin(GL_TRIANGLES);
    glNormal3f(1, 0, 0);
    glVertex3f(hw, 0, hd);  glVertex3f(hw, 0, -hd);  glVertex3f(hw, h, 0);
    glNormal3f(-1, 0, 0);
    glVertex3f(-hw, 0, -hd); glVertex3f(-hw, 0, hd);  glVertex3f(-hw, h, 0);
    glEnd();
    glPopMatrix();
}

// prism whose ridge runs along Z (front-facing gable)
void prismZ(float cx, float y0, float cz, float len, float h, float wid) {
    glPushMatrix();
    glTranslatef(cx, 0, cz);
    glRotatef(90, 0, 1, 0);
    prism(0, y0, 0, len, h, wid);
    glPopMatrix();
}

void quadUp(float x0, float z0, float x1, float z1, float y) {
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(x0, y, z1);
    glVertex3f(x1, y, z1);
    glVertex3f(x1, y, z0);
    glVertex3f(x0, y, z0);
    glEnd();
}

// One face of a sub-divided box
void faceGrid(float ax, float ay, float az, float ux, float uy, float uz, int nu,
              float vx, float vy, float vz, int nv, float nx, float ny, float nz,
              float r, float g, float b, float jit) {
    glNormal3f(nx, ny, nz);
    glBegin(GL_QUADS);
    for (int i = 0; i < nu; i++) {
        for (int j = 0; j < nv; j++) {
            float k = 1.0f + (rnd() - 0.5f) * 2 * jit;
            glColor3f(r * k, g * k, b * k);
            float u0 = (float)i / nu, u1 = (float)(i + 1) / nu, v0 = (float)j / nv, v1 = (float)(j + 1) / nv;
            glVertex3f(ax + ux * u0 + vx * v0, ay + uy * u0 + vy * v0, az + uz * u0 + vz * v0);
            glVertex3f(ax + ux * u1 + vx * v0, ay + uy * u1 + vy * v0, az + uz * u1 + vz * v0);
            glVertex3f(ax + ux * u1 + vx * v1, ay + uy * u1 + vy * v1, az + uz * u1 + vz * v1);
            glVertex3f(ax + ux * u0 + vx * v1, ay + uy * u0 + vy * v1, az + uz * u0 + vz * v1);
        }
    }
    glEnd();
}

int cells(float len, float cell) {
    int n = (int)ceilf(len / cell);
    return n < 1 ? 1 : n;
}

void bigBox(float x0, float y0, float z0, float x1, float y1, float z1,
            float r, float g, float b, float jit, float cell) {
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
void wallBox(float x0, float y0, float z0, float x1, float y1, float z1,
             float r, float g, float b, float jit, float cell) {
    solid(x0, y0, z0, x1, y1, z1);
    bigBox(x0, y0, z0, x1, y1, z1, r, g, b, jit, cell);
}

// interior lining / wallpaper layer (no collision)
void lining(float x0, float y0, float z0, float x1, float y1, float z1,
            float r, float g, float b) {
    bigBox(x0, y0, z0, x1, y1, z1, r, g, b, 0.14f, 0.8f);
}

// ------------------------------------------------------------------ materials
void mat(float r, float g, float b, float spec, float shin) {
    glColor3f(r, g, b);
    GLfloat s[4] = { spec, spec, spec, 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shin);
}

void matStone() { mat(0.32f, 0.32f, 0.34f, 0.03f, 4); }
void matWood()  { mat(0.25f, 0.16f, 0.09f, 0.05f, 8); }
void matRust()  { mat(0.45f, 0.22f, 0.10f, 0.10f, 12); }
void matIron()  { mat(0.07f, 0.07f, 0.08f, 0.35f, 30); }

// ------------------------------------------------------------------ candle flame
void flame(float x, float y, float z, float s, float ph) {
    float f = 0.75f + 0.25f * sinf(simTime * 13 + ph) + 0.15f * sinf(simTime * 29 + ph * 2);
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(1.0f, 0.62f + 0.2f * f, 0.15f);
    glScalef(1, f, 1);
    cone(0.028f * s, 0.11f * s, 6);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// ------------------------------------------------------------------ windows
void windowFace(float w, float h, int type, bool arch) {
    glDisable(GL_LIGHTING);
    float r = 0.02f, g = 0.03f, b = 0.04f;
    if (type == 1) { r = 1.0f; g = 0.78f; b = 0.30f; }
    if (type >= 2) { r = 0.30f; g = 0.44f; b = 0.50f; }
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(-w / 2, -h / 2, 0); glVertex3f(w / 2, -h / 2, 0);
    glVertex3f(w / 2, h / 2, 0); glVertex3f(-w / 2, h / 2, 0);
    glEnd();
    if (arch) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, h / 2, 0);
        for (int i = 0; i <= 12; i++) {
            float a = PI * i / 12;
            glVertex3f(w / 2 * cosf(a), h / 2 + w / 2 * sinf(a), 0);
        }
        glEnd();
    }
    float top = arch ? h / 2 + w / 2 : h / 2;
    glColor3f(0.05f, 0.04f, 0.03f);                        // mullions
    glBegin(GL_QUADS);
    glVertex3f(-0.03f, -h / 2, 0.005f); glVertex3f(0.03f, -h / 2, 0.005f);
    glVertex3f(0.03f, top, 0.005f); glVertex3f(-0.03f, top, 0.005f);
    glVertex3f(-w / 2, -0.03f, 0.005f); glVertex3f(w / 2, -0.03f, 0.005f);
    glVertex3f(w / 2, 0.03f, 0.005f); glVertex3f(-w / 2, 0.03f, 0.005f);
    if (type == 3) {
        for (float x = -w / 2 + 0.15f; x < w / 2; x += 0.2f) {
            glVertex3f(x - 0.015f, -h / 2, 0.01f); glVertex3f(x + 0.015f, -h / 2, 0.01f);
            glVertex3f(x + 0.015f, h / 2, 0.01f);  glVertex3f(x - 0.015f, h / 2, 0.01f);
        }
    }
    glEnd();
    if (type == 0) {                                        // cracked glass
        glColor3f(0.25f, 0.28f, 0.30f);
        glBegin(GL_LINES);
        glVertex3f(-w * 0.3f, h * 0.3f, 0.006f); glVertex3f(0.0f, 0.0f, 0.06f);
        glVertex3f(0.0f, 0.0f, 0.006f); glVertex3f(w * 0.25f, -h * 0.35f, 0.006f);
        glVertex3f(0.0f, 0.0f, 0.006f); glVertex3f(w * 0.35f, h * 0.2f, 0.006f);
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

void window(float x, float y, float z, float rot, float w, float h, int outer, int inner, bool arch) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rot, 0, 1, 0);
    glPushMatrix();
    glTranslatef(0, 0, 0.22f);
    windowFace(w, h, outer, arch);
    glPopMatrix();
    if (inner >= 0) {
        glPushMatrix();
        glTranslatef(0, 0, -0.30f);
        glRotatef(180, 0, 1, 0);
        windowFace(w, h, inner, arch);
        glPopMatrix();
    }
    glPopMatrix();
}

int litHash(float a, float b, float c) {
    return (((int)(a * 3.f + b * 7.f + c * 5.f + 100)) % 3 + 3) % 3;
}
