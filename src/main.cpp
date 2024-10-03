#include <iostream>
#include <GL/gl.h>         // Para OpenGL
#include <GL/glu.h>        // Para funções de utilidade OpenGL
#include <GL/freeglut.h>   // Para FreeGLUT
#include <map>

#define STB_IMAGE_IMPLEMENTATION

#include "ObjLoader.h"
#include "Camera.h"
#include "stb_image.h"

using namespace std;

ObjLoader obj;

GLfloat objX = 0.0, objZ = 0.0, objY = 0.0;
GLfloat cameraDistance = 5.0;
GLfloat cameraHeight = 5.0;

GLfloat cameraScale = 1.0;

Camera camera(objX - cameraDistance, cameraHeight, objZ - cameraDistance);

GLuint pisoTexture; // ID da textura do piso

map<unsigned char, bool> keyStates;

// Função para printar o estado das teclas
void PrintKeyStates() {
    cout << "Key States: " << endl;
    for (const auto& keyState : keyStates) {
        cout << keyState.first << ": " << (keyState.second ? "Pressed" : "Released") << endl;
    }
    cout << endl;
}

class Meteour {
public:
    GLfloat posX, posY, posZ;
    GLfloat speed;

    Meteour(GLfloat x, GLfloat y, GLfloat z, GLfloat speed)
        : posX(x), posY(y), posZ(z), speed(speed) {}

    void update() {
        posY -= speed; // Meteoro cai no eixo Y
    }

    bool hasCollided(GLfloat objX, GLfloat objZ, GLfloat objSize) {
        // Simples verificação de colisão com base nas posições X e Z
        return (fabs(posX - objX) < objSize && fabs(posZ - objZ) < objSize && posY <= 0.0);
    }
};

float velocidadeMovimento = 0.1f;  // Controla a velocidade de movimento para frente/trás
float velocidadeRotacao = 1.0f;    // Controla a velocidade de rotação
float velocidadeZoom = 0.1f;

float currentSpeed = 0.1f; // Velocidade atual do movimento para frente
const float maxSpeed = 0.5f; // Velocidade máxima permitida

void AtualizaMovimento() {
   if (keyStates['w']) {
        currentSpeed += 0.001f; // Aumenta a velocidade ao longo do tempo
        currentSpeed = fmin(currentSpeed, maxSpeed); // V max

        objX += sin((objY) * 3.14 / 180) * currentSpeed;
        objZ += cos((objY) * 3.14 / 180) * currentSpeed;
    } else {
        currentSpeed -= 0.005f; // Aumenta a velocidade ao longo do tempo
        currentSpeed = fmax(currentSpeed, 0.0); // V max

        objX += sin((objY) * 3.14 / 180) * currentSpeed;
        objZ += cos((objY) * 3.14 / 180) * currentSpeed;
    }
    // mover para trás
    if (keyStates['s']) {
        objX -= sin((objY) * 3.14 / 180) * velocidadeMovimento;
        objZ -= cos((objY) * 3.14 / 180) * velocidadeMovimento;
    }
    // girar à esquerda
    if (keyStates['a']) {
        objY += velocidadeRotacao;
    }
    // girar à direita
    if (keyStates['d']) {
        objY -= velocidadeRotacao;
    }

    // Zoom com 'i' e 'k'
    if (keyStates['i']) {
        cameraScale += velocidadeZoom;
    }

    if (keyStates['k']) {
        if (cameraScale >= 1.0) {
            cameraScale -= velocidadeZoom;
        }
    }

    glutPostRedisplay();
}


void Teclado(unsigned char key, int x, int y) {
    keyStates[key] = true;  // Marca a tecla como pressionada
    AtualizaMovimento();
}

void TecladoUp(unsigned char key, int x, int y) {
    keyStates[key] = false; // Marca a tecla como solta
}


void TecladoEspecial(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            objY += 5.0;  // Girar o objeto para a esquerda
            break;
        case GLUT_KEY_RIGHT:
            objY -= 5.0;  // Girar o objeto para a direita
            break;
    }
    glutPostRedisplay();
}

void AtualizaCamera() {
    GLfloat rad = (objY * 3.14) / 180; // Converter o ângulo para radianos

    // Atualiza a posição da câmera baseada na posição e rotação do objeto
    camera.posX = objX - cameraDistance * sin(rad) * cameraScale;
    camera.posZ = objZ - cameraDistance * cos(rad) * cameraScale;
    camera.posY = cameraHeight * cameraScale;  // Altura fixa da câmera

    // A câmera olha diretamente para o objeto
    camera.lookAt(objX, 0.0, objZ);


}


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

    Piso(1.0, -4.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    AtualizaCamera();

      // Chama a função que atualiza o movimento e imprime o estado das teclas
    AtualizaMovimento();


    glColor3f(1.0, 0.0, 1.0);
    glPushMatrix();
        glTranslatef(objX, 0.0, objZ);
        glRotatef(objY, 0.0, 1.0, 0.0);
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
        glutKeyboardUpFunc(TecladoUp);
        glutMouseFunc([](int button, int state, int x, int y) { camera.mouseButton(button, state, x, y); });
        glutMotionFunc([](int x, int y) { camera.mouseMotion(x, y); });

    glutMainLoop();

    return 0;
}
