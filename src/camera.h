#ifndef CAMERA_H
#define CAMERA_H

#include <GL/freeglut.h>

class Camera {
public:
    float posX, posY, posZ; // Posição da câmera
    float rotX, rotY;      // Ângulos de rotação
    int lastMouseX, lastMouseY; // Posição anterior do mouse
    int mouseDown = 0; // Estado do botão do mouse

    Camera(float startX, float startY, float startZ)
        : posX(startX), posY(startY), posZ(startZ), rotX(0), rotY(0) {}

    void move(float dx, float dy, float dz) {
        posX += dx;
        posY += dy;
        posZ += dz;
    }

    void rotate(float dRotX, float dRotY) {
        rotX += dRotX;
        rotY += dRotY;
    }

    void apply() {
        glRotatef(rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotY, 0.0f, 1.0f, 0.0f);
        glTranslatef(-posX, -posY, -posZ);
    }

    void mouseMotion(int x, int y) {
        if (mouseDown) {
            // Calcular o movimento do mouse
            int dx = x - lastMouseX;
            int dy = y - lastMouseY;

            // Atualizar os ângulos de rotação da câmera
            rotate(dy * 0.1f, dx * 0.1f);

            // Redesenhar a cena
            glutPostRedisplay();
        }

        // Atualizar a posição do mouse
        lastMouseX = x;
        lastMouseY = y;
    }

    void mouseButton(int button, int state, int x, int y) {
        if (button == GLUT_LEFT_BUTTON) {
            if (state == GLUT_DOWN) {
                mouseDown = 1;
                lastMouseX = x;
                lastMouseY = y;
            } else if (state == GLUT_UP) {
                mouseDown = 0;
            }
        }
    }

    void lookAt(float targetX, float targetY, float targetZ) {
        gluLookAt(
            posX, posY, posZ,  // Posição da câmera
            targetX, targetY, targetZ,  // Ponto que a câmera está olhando
            0.0, 1.0, 0.0   // Vetor "up" para orientar a câmera (normalmente o eixo Y é "up")
        );
    }

    void moveCamera(float x, float y, float z, float tx, float ty, float tz){
        // Subir a câmera modificando o valor de 'y'
        gluLookAt(
            x, y, z,  // Posição da câmera (a altura é controlada por 'y')
            tx, ty, tz,  // Ponto que a câmera está olhando
            0.0, 1.0, 0.0   // Vetor "up", mantendo o eixo Y como "up"
        );
    }
};

#endif
