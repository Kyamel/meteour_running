#include <GL/gl.h>         // Para OpenGL
#include <GL/glu.h>        // Para funções de utilidade OpenGL
#include <GL/freeglut.h>   // Para FreeGLUT

#define STB_IMAGE_IMPLEMENTATION

#include "ObjLoader.h"
#include "Camera.h"
#include "stb_image.h"

using namespace std;

ObjLoader obj;

GLfloat objX = 0.0, objZ = 0.0;
GLfloat cameraDistance = 5.0; // Distância da câmera em relação ao objeto
GLfloat cameraHeight = 5.0; // Altura da câmera em relação ao objeto

Camera camera(objX - cameraDistance, cameraHeight, objZ - cameraDistance);

GLfloat dx = 0.0; // Translação no eixo X do objeto
GLfloat dz = 0.0; // Translação no eixo Z do objeto

GLuint pisoTexture; // ID da textura do piso

void LoadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Erro ao carregar a textura: " << stbi_failure_reason() << std::endl;
        return;
    }

    // Converte para formato RGBA se necessário
    if (channels == 3) {
        unsigned char* rgbaData = new unsigned char[width * height * 4];
        for (int i = 0; i < width * height; ++i) {
            rgbaData[i * 4 + 0] = data[i * 3 + 0]; // R
            rgbaData[i * 4 + 1] = data[i * 3 + 1]; // G
            rgbaData[i * 4 + 2] = data[i * 3 + 2]; // B
            rgbaData[i * 4 + 3] = 255;             // A
        }

        glGenTextures(1, &pisoTexture);
        glBindTexture(GL_TEXTURE_2D, pisoTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);

        // Configura os parâmetros da textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        delete[] rgbaData; // Liberar os dados RGBA
    } else if (channels == 4) {
        glGenTextures(1, &pisoTexture);
        glBindTexture(GL_TEXTURE_2D, pisoTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Configura os parâmetros da textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cerr << "Formato de imagem não suportado." << std::endl;
    }

    stbi_image_free(data); // Liberar memória do data original
}


void AtualizaCamera() {
    // Calcular a nova posição da câmera
    GLfloat targetX = objX;
    GLfloat targetZ = objZ - cameraDistance;

    camera.posX += (targetX - camera.posX) * 0.1; // Lerp suave para o alvo
    camera.posY = cameraHeight; // Mantém a altura fixa
    camera.posZ += (targetZ - camera.posZ) * 0.1; // Lerp suave para o alvo

    // Limitar a altura da câmera
    if (camera.posY < 1.0) {
        camera.posY = 1.0;
    }

    camera.apply();
}


void Teclado(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
            objZ -= 0.1;
            break;
        case 's':
            objZ += 0.1;
            break;
        case 'a':
            objX -= 0.1;
            break;
        case 'd':
            objX += 0.1;
            break;
        case 'i':
            camera.move(0.0, 0.0, 0.1);
            break;
        case 'k':
            camera.move(0.0, 0.0, -0.1);
            break;
    }
    glutPostRedisplay();
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


    LoadTexture("assets/textura_piso.jpg"); // Carrega a textura do piso
}

void Piso(float escala, float altura) {
    glPushMatrix();
    glTranslatef(0.0, altura, 0.0);
    glScalef(escala, escala, escala);

    glEnable(GL_TEXTURE_2D); // Habilita o uso de texturas
    glBindTexture(GL_TEXTURE_2D, pisoTexture); // Usa a textura carregada

    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0, 0.0, 100.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0, 0.0, 100.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0, 0.0, -100.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(100.0, 0.0, -100.0);
    glEnd();

    glDisable(GL_TEXTURE_2D); // Desabilita o uso de texturas
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa os buffers

    Piso(1.0, -4.0); // Desenha o piso

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // A câmera é aplicada antes de desenhar o objeto
    AtualizaCamera();
    camera.apply(); // Aplique as transformações da câmera

    glColor3f(1.0, 0.0, 1.0);
    glPushMatrix();
    glScalef(2.0, 2.0, 2.0);
    glTranslatef(objX, 0.0, objZ);
    ObjWireFrame();
    glPopMatrix();

    glutSwapBuffers(); // Troca os buffers
    glFlush(); // Garante que todas as operações estão completas
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(1366, 768);
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
