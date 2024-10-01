#include <GL/glut.h>
#include "ObjLoader.h"
#include "Camera.h"

using namespace std;

ObjLoader obj;

GLfloat objX = 0.0, objZ = 0.0;
GLfloat cameraDistance = 5.0; // Distância da câmera em relação ao objeto
GLfloat cameraHeight = 3.0; // Altura da câmera em relação ao objeto

Camera camera(objX, cameraHeight, objZ - cameraDistance);

GLfloat dx = 0.0; // Translação no eixo X do objeto
GLfloat dz = 0.0; // Translação no eixo Z do objeto

void AtualizaCamera() {
    // A câmera segue o objeto a uma distância fixa e altura definida
    camera.posX = objX;  // Câmera segue a posição X do objeto
    camera.posY = cameraHeight; // Mantém a altura fixa
    camera.posZ = objZ - cameraDistance; // Mantém a distância fixa no eixo Z

    // Reorienta a câmera para olhar para o objeto
    camera.apply();
}

void Teclado(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
            dz -= 0.1; // Move o objeto para frente
            break;
        case 's':
            dz += 0.1; // Move o objeto para trás
            break;
        case 'a':
            dx -= 0.1; // Move o objeto para a esquerda
            break;
        case 'd':
            dx += 0.1; // Move o objeto para a direita
            break;
        case 'i':
            camera.move(0.0, 0.0, 0.1); // Move a câmera para frente
            break;
        case 'k':
            camera.move(0.0, 0.0, -0.1); // Move a câmera para trás
            break;
    }
    AtualizaCamera();
    glutPostRedisplay(); // Requisita redesenho da cena
}

void Inicializa(void) {
    glClearColor(1.0, 1.0, 1.0, 0.0); // fundo branco
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65, 1, 0.5, 500);
}

void Piso(float escala, float altura) {
    glColor3f(0.0, 1.0, 0.0);
    glPushMatrix();
    glTranslatef(0.0, altura, 0.0);
    glScalef(escala, escala, escala);
    glBegin(GL_QUADS);
    glVertex3f(100.0, 0.0, 100.0);
    glVertex3f(-100.0, 0.0, 100.0);
    glVertex3f(-100.0, 0.0, -100.0);
    glVertex3f(100.0, 0.0, -100.0);
    glEnd();
    glPopMatrix();
}

void ObjWireFrame(void) {
    glPushMatrix();
    for (unsigned int j = 0; j < (obj.faces).size(); ++j) {
        glBegin(GL_LINE_LOOP);
        for (unsigned int i = 0; i < (obj.faces[j]).size(); ++i) {
            GLfloat nor[3] = {(obj.normais[obj.faces[j][i][2]][0]), (obj.normais[obj.faces[j][i][2]][1]), (obj.normais[obj.faces[j][i][2]][2])};
            glNormal3fv(nor);
            GLfloat vert[3] = {(obj.vertices[obj.faces[j][i][0]][0]), (obj.vertices[obj.faces[j][i][0]][1]), (obj.vertices[obj.faces[j][i][0]][2])};
            glVertex3fv(vert);
        }
        glEnd();
    }
    glPopMatrix();
}

void Desenha(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Piso(1.0, -4.0); // Desenha o piso

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.apply(); // Aplica as transformações da câmera

    glColor3f(1.0, 0.0, 1.0);
    glPushMatrix();
    glScalef(2.0, 2.0, 2.0);
    glTranslatef(dx, 0.0, dz); // Translação do objeto em X e Z
    ObjWireFrame();
    glPopMatrix();

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Meteour Running");

    Inicializa();
    obj.load("bonde.obj"); // Carrega o objeto

    glutDisplayFunc(Desenha);
    glutKeyboardFunc(Teclado);
    glutMouseFunc([](int button, int state, int x, int y) { camera.mouseButton(button, state, x, y); });
    glutMotionFunc([](int x, int y) { camera.mouseMotion(x, y); });

    glutMainLoop();
    return 0;
}
