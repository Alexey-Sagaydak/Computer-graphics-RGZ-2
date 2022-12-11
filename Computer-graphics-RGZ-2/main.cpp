#include <iostream>
#include <glut.h>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int dimensions = 400;
const float PI = 3.1415;
float delta = 0.05;
float teta = 1;
float phi = 0.1;
float r = 15;

GLfloat camX, camY, camZ = 0;
GLfloat lookX, lookY, lookZ = 0;

float t = 0;

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

    void calcCylinderCoord(float radius, float height, float h, float l) {
        float modR = getModifiedRadius(radius, h, l);
        l = convertDegToRad(l);
        this->x = modR * sin(l);
        this->y = modR * cos(l);
        this->z = height * h;
    }

    void calcVaseCoord(float radius, float height, float h, float l) {
        float modR = radius * (1 - 0.3f * sin(2 * h * PI));
        l = convertDegToRad(l);
        this->x = modR * sin(l);
        this->y = modR * cos(l);
        this->z = height * h;
    }
};

void drawWireCylinder(float radius, float height) {
    Point p;

    glPolygonMode(GL_FRONT, GL_LINE);
    for (int l = 0; l <= 360; l += 5) {
        glBegin(GL_LINE_STRIP);
        for (float h = -0.5; h <= 0.5; h += 0.1) {
            p.calcCylinderCoord(radius, height, h, l);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }

    for (float h = -0.5; h <= 0.5; h += 0.1) {
        glBegin(GL_LINE_STRIP);
        for (int l = 0; l <= 360; l += 5) {
            p.calcCylinderCoord(radius, height, h, l);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }
}

void drawSolidCylinder(float radius, float height, GLuint texture) {
    Point p;

    glEnable(GL_TEXTURE_2D);

    int texCoord[][2] = {
        {0,1},
        {1,1},
        {1,0},
        {0,0}
    };

    glBindTexture(GL_TEXTURE_2D, texture);

    glPolygonMode(GL_FRONT, GL_FILL);
    for (int l = 0; l <= 360; l += 5) {
        glBegin(GL_QUAD_STRIP);
        for (float h = -0.5; h <= 0.5; h += 0.01) {
            int k = fabs(h) * 10;

            if (k % 2 != 0)
                glTexCoord2d(texCoord[0][0], texCoord[0][1]);
            else
                glTexCoord2d(texCoord[1][0], texCoord[1][1]);

            p.calcCylinderCoord(radius, height, h, l);
            glColor3f(1, 1, 1);
            glVertex3f(p.x, p.y, p.z);

            if (k % 2 != 0)
                glTexCoord2d(texCoord[2][0], texCoord[2][1]);
            else
                glTexCoord2d(texCoord[3][0], texCoord[3][1]);

            p.calcCylinderCoord(radius, height, h, l + PI * 2);
            glColor3f(1, 1, 1);
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

    data = stbi_load(path, &width, &height, &nrChannels, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        width, height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, data);

    return texture;
}

void cameraFunc() {
    camX = r * sin(teta) * cos(phi);
    camY = r * sin(teta) * sin(phi);
    camZ = r * cos(teta);

    gluLookAt(camX, camY, camZ, 0, lookY, lookZ, 0, 0, 1);
    //gluLookAt(0, 0, 0, camX, camY, camZ, 0, 0, 1);
}

void drawWireVase(float radius, float height) {
    Point p;

    glPolygonMode(GL_FRONT, GL_LINE);
    for (int l = 0; l <= 360; l += 5) {
        glBegin(GL_LINE_STRIP);
        for (float h = -0.5; h <= 0.5; h += 0.1) {
            p.calcVaseCoord(radius, height, h, l);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }

    for (float h = -0.5; h <= 0.5; h += 0.1) {
        glBegin(GL_LINE_STRIP);
        for (int l = 0; l <= 360; l += 5) {
            p.calcVaseCoord(radius, height, h, l);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }
}

void drawSolidVase(float radius, float height) {
    Point p;

    glPolygonMode(GL_FRONT, GL_FILL);
    for (int l = 0; l <= 360; l += 5) {
        glBegin(GL_QUAD_STRIP);
        for (float h = -0.5; h <= 0.5; h += 0.1) {
            p.calcVaseCoord(radius, height, h, l);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);


            p.calcVaseCoord(radius, height, h, l + 2 * PI);
            glColor3f(h, 0, 1 - h);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();
    }


}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    cameraFunc();

    char fname[] = "floppa.bmp";
    GLuint tex = createTexture(fname);

    glColor3f(0.0f, 1.0f, 0.0f);

    glViewport(0, 0, 200, 400);
    drawWireVase(5, 8);
    glViewport(200, 0, 200, 400);
    drawSolidVase(5, 8);

    glutSwapBuffers();
}

void lighting() {

    GLfloat ambient[] = { 0.7, 0.0, 0.4, 1.0 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specular[] = { 0.0, 0.0, 1.0, 1.0 };
    GLfloat position[] = { 2.0, 3.0, 5.0, 1.0 };

    GLfloat Mambient[] = { 0.25, 0.25, 0.5, 1.0 };
    GLfloat Mdiffuse[] = { 0.1,0.3, 0.4, 1.0 };
    GLfloat Mspecular[] = { 0.1,0.0, 0.2, 1.0 };

    glMaterialfv(GL_FRONT, GL_SPECULAR, Mambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, Mdiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, Mspecular);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void init(void)
{
    lighting();
    glShadeModel(GL_FLAT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    //проекция
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

void reshape(int width, int height)
{
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glShadeModel(GL_SMOOTH);
}

void processSpecialKeys(int key, int x, int y) {

    if (key == GLUT_KEY_RIGHT)
        phi += delta;
    else if (key == GLUT_KEY_LEFT)
        phi -= delta;

    if (key == GLUT_KEY_UP) {
        teta += delta;
    }
    else if (key == GLUT_KEY_DOWN) {
        teta -= delta;
    }

    if (key == GLUT_KEY_PAGE_UP) {
        r -= 0.5f;
    }
    else if (key == GLUT_KEY_PAGE_DOWN) {
        r += 0.5f;
    }

    glutPostRedisplay();
}

void processRegularKeys(unsigned char key, int x, int y) {
    if (key == 'w') {
        lookZ += 1.0f;
    }
    if (key == 's') {
        lookZ -= 1.0f;
    }
    if (key == 'a') {
        lookY -= 1.0f;
    }
    if (key == 'd') {
        lookY += 1.0f;
    }

    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(dimensions, dimensions);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Расчётно-графическое задание.");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(processSpecialKeys);
    glutKeyboardFunc(processRegularKeys);

    glutMainLoop();
    return 0;
}
