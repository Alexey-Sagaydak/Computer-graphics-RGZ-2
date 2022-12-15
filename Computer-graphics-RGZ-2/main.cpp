#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <glut.h>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

enum DISPLAY_MODE { DISPLAY_WIREFRAME = 0, DISPLAY_SOLID, DISPLAY_SOLID_TEXTURED };
DISPLAY_MODE displayMode;
enum PROJECTION_MODE { PROJECTION_AXONOMETRIC = 0, PROJECTION_PERSPECTIVE };
PROJECTION_MODE projectionMode;
enum LIGHT_MODE { LIGHT_OFF = 0, LIGHT_ON };
LIGHT_MODE lightMode;

const int dimensions = 400;
const float PI = 3.1415;
float deltaPos = 0.05;
float tetaPos = 1;
float phiPos = 0.1;
float cameraDistance = 20;

float deltaLightPos = 0.05;
float tetaLightPos = 1;
float phiLightPos = 0.1;
float lightDistance = 20;

GLuint tex;

struct Point
{
    float x;
    float y;
    float z;

    Point(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Point() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }

    float convertRadToDeg(float radian) {
        const float PI = 3.1415;
        return (180 * radian) / PI;
    }

    float convertDegToRad(float degree) {
        const float PI = 3.1415;
        return (degree * PI) / 180;
    }

    float getModifiedRadius(float radius, float h, int l) {
        float a = 2 * h * PI + 0.5 * convertDegToRad(l);
        return radius * (1 + fabs(sin(a)));
    }

    void calsVaseCoord(float radius, float height, float h, float l) {
        float modR = radius * (1 - 0.3f * sin(2 * h * PI));
        l = convertDegToRad(l);
        this->x = modR * sin(l);
        this->y = modR * cos(l);
        this->z = height * h;
    }
};

void drawWireVase(float radius, float height) {
    Point p;

    glPolygonMode(GL_FRONT, GL_LINE);
    for (int l = 0; l <= 360; l += 5) {
        glBegin(GL_LINE_STRIP);
        for (float h = -0.5; h <= 0.5; h += 0.1) {
            p.calsVaseCoord(radius, height, h, l);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }

    for (float h = -0.5; h <= 0.5; h += 0.1) {
        glBegin(GL_LINE_STRIP);
        for (int l = 0; l <= 360; l += 5) {
            p.calsVaseCoord(radius, height, h, l);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }
}

void drawSolidVase(float radius, float height, GLuint texture) {
    Point p;

    int dl = 5;
    float dh = 0.01f;

    GLfloat ix = 0;
    GLfloat iy = 0;
    GLfloat iwx = 1.0f / (360.0f / dl);
    GLfloat iwy = dh;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPolygonMode(GL_FRONT, GL_FILL);
    for (int l = 0; l < 360; l += dl) {
        glBegin(GL_QUAD_STRIP);
        glColor3f(1, 1, 1);
        for (float h = -0.5; h <= 0.5; h += dh) {
            int k = fabs(h) * 10;

            if (k % 2 != 0)
                glTexCoord2d(ix, iy);
            else
                glTexCoord2d(ix, iy + iwy);

            p.calsVaseCoord(radius, height, h, l);
            glVertex3f(p.x, p.y, p.z);

            if (k % 2 != 0)
                glTexCoord2d(ix + iwx, iy + iwy);
            else
                glTexCoord2d(ix + iwx, iy);

            p.calsVaseCoord(radius, height, h, l + dl);
            glVertex3f(p.x, p.y, p.z);

            ix += iwx;
        }
        iy += iwy;
        ix = 0;

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}

void drawSolidVase(float radius, float height) {
    Point p;

    int dl = 5;
    float dh = 0.01f;

    glPolygonMode(GL_FRONT, GL_FILL);
    for (int l = 0; l < 360; l += dl) {
        glBegin(GL_QUAD_STRIP);
        glColor3f(1, 1, 1);
        for (float h = -0.5; h <= 0.5; h += dh) {
            p.calsVaseCoord(radius, height, h, l);
            glColor3f(h, 0.5, 1 - h);
            glVertex3f(p.x, p.y, p.z);

            p.calsVaseCoord(radius, height, h, l + dl);
            glVertex3f(p.x, p.y, p.z);
        }

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}

GLuint createTexture(char* path) {
    GLuint texture;
    int width, height, nrChannels;
    unsigned char* data;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    data = stbi_load(path, &width, &height, &nrChannels, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        width, height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, data);

    return texture;
}

void cameraFunc() {
    GLfloat camX, camY, camZ = 0;

    camX = cameraDistance * sin(tetaPos) * cos(phiPos);
    camY = cameraDistance * sin(tetaPos) * sin(phiPos);
    camZ = cameraDistance * cos(tetaPos);

    gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 0, 1);
}

void displayCylinder(DISPLAY_MODE mode) {

    if (mode == DISPLAY_WIREFRAME) {
        drawWireVase(3, 8);
    }
    if (mode == DISPLAY_SOLID) {
        drawSolidVase(3, 8);
    }
    if (mode == DISPLAY_SOLID_TEXTURED) {
        drawSolidVase(3, 8, tex);
    }
}

void setProjection(PROJECTION_MODE mode) {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (mode == PROJECTION_AXONOMETRIC) {

        glRotatef(45.0f, 0, 0, 1);
        glRotatef(30.0f, 1, 0, 0);

        glOrtho(-10, 10, -10, 10, 1, 50);

    }
    if (mode == PROJECTION_PERSPECTIVE) {
        gluPerspective(60, 1, 1, 100);
    }

    glMatrixMode(GL_MODELVIEW);
}

void setLight(LIGHT_MODE mode) {

    GLfloat lightX, lightY, lightZ = 0;

    lightX = lightDistance * sin(tetaLightPos) * cos(phiLightPos);
    lightY = lightDistance * sin(tetaLightPos) * sin(phiLightPos);
    lightZ = lightDistance * cos(tetaLightPos);

    GLfloat position[] = { lightX, lightY, lightZ, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    if (mode == LIGHT_ON) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }
    if (mode == LIGHT_OFF) {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
}

void renderScene() {
    setProjection(projectionMode);
    setLight(lightMode);
    displayCylinder(displayMode);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    cameraFunc();
    renderScene();
    glutSwapBuffers();
}

void lighting() {
    GLfloat ambient[] = { 0.0, 1.0, 0.5, 1.0 };
    GLfloat diffuse[] = { 0.5, 1.0, 1.0, 1.0 };
    GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };

    GLfloat Mambient[] = { 0.25, 0.25, 0.5, 1.0 };
    GLfloat Mdiffuse[] = { 0.3,0.5, 0.7, 1.0 };
    GLfloat Mspecular[] = { 0.1,0.0, 0.2, 1.0 };

    glMaterialfv(GL_FRONT, GL_SPECULAR, Mambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, Mdiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, Mspecular);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

void init() {
    char fname[] = "texture.bmp";
    tex = createTexture(fname);
    displayMode = DISPLAY_WIREFRAME;
    projectionMode = PROJECTION_AXONOMETRIC;
    lightMode = LIGHT_ON;

    lighting();

    glShadeModel(GL_FLAT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    //проекция
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setProjection(PROJECTION_AXONOMETRIC);
    glMatrixMode(GL_MODELVIEW);
}

void reshape(int width, int height) {
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glShadeModel(GL_SMOOTH);
}

void processSpecialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_RIGHT)
        phiPos += deltaPos;
    else if (key == GLUT_KEY_LEFT)
        phiPos -= deltaPos;

    if (key == GLUT_KEY_UP)
        tetaPos += deltaPos;
    else if (key == GLUT_KEY_DOWN)
        tetaPos -= deltaPos;

    if (key == GLUT_KEY_PAGE_UP)
        cameraDistance -= 0.5f;
    else if (key == GLUT_KEY_PAGE_DOWN)
        cameraDistance += 0.5f;

    if (key == GLUT_KEY_RIGHT)
        phiLightPos += deltaLightPos;
    else if (key == GLUT_KEY_LEFT)
        phiLightPos -= deltaLightPos;

    glutPostRedisplay();
}

void processRegularKeys(unsigned char key, int x, int y) {

    if (key == '1')
        displayMode = DISPLAY_WIREFRAME;
    if (key == '2')
        displayMode = DISPLAY_SOLID;
    if (key == '3')
        displayMode = DISPLAY_SOLID_TEXTURED;

    if (key == 'q')
        lightMode = LIGHT_OFF;
    if (key == 'e')
        lightMode = LIGHT_ON;

    if (key == 'a')
        projectionMode = PROJECTION_PERSPECTIVE;
    if (key == 'd')
        projectionMode = PROJECTION_AXONOMETRIC;

    if (key == 'l')
        phiLightPos += deltaLightPos;
    else if (key == 'j')
        phiLightPos -= deltaLightPos;

    if (key == 'i')
        tetaLightPos += deltaLightPos;
    else if (key == 'k')
        tetaLightPos -= deltaLightPos;

    if (key == 's')
        lightDistance -= 0.5f;
    else if (key == 'w')
        lightDistance += 0.5f;

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(dimensions, dimensions);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Сагайдак А.Е. АВТ-113. Вариант №14");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(processSpecialKeys);
    glutKeyboardFunc(processRegularKeys);
    glutMainLoop();
    return 0;
}