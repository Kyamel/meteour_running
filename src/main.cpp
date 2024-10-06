#include <iostream>
#include <GL/gl.h>         // Para OpenGL
#include <GL/glu.h>        // Para funções de utilidade OpenGL
#include <GL/freeglut.h>   // Para FreeGLUT
#include <map>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION

#include "ObjLoader.h"
#include "camera.h"
#include "stb_image.h"

using namespace std;

#define WINDOW_WIDTH 1366
#define WINDOW_HEIGHT 768

ObjLoader obj;

GLfloat cameraDistance = 5.0;
GLfloat cameraHeight = 5.0;

GLfloat cameraScale = 1.0;

Camera camera(obj.x - cameraDistance, cameraHeight, obj.z - cameraDistance);

class TestureHelper {
public:
   static void LoadTexture(const char* filename, GLuint& textureID) {
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

            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);

            // Configura os parâmetros da textura
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            delete[] rgbaData; // Liberar os dados RGBA
        } else if (channels == 4) {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

            // Configura os parâmetros da textura
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            std::cerr << "Formato de imagem não suportado." << std::endl;
        }

        stbi_image_free(data); // Liberar memória do data original
   }
};

class FPSManager {
private:
    float fps;
    int frameCount;
    float previousTime;

public:
    FPSManager() : fps(0.0f), frameCount(0), previousTime(0.0f) {}

    // Função para calcular FPS
    void calculateFPS() {
        frameCount++;
        float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;  // Tempo em segundos
        float deltaTime = currentTime - previousTime;

        if (deltaTime > 1.0f) {
            fps = frameCount / deltaTime;  // Calcula o FPS
            frameCount = 0;  // Reseta a contagem de quadros
            previousTime = currentTime;  // Atualiza o tempo anterior
        }
    }

    // Função para desenhar texto na tela
    void drawText(const char *text, float x, float y) {

        glRasterPos2f(x, y);

        while (*text) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
            text++;
        }
    }

    // Função para exibir o FPS na tela
    void displayFPS(float x, float y) {
        char fpsText[32];
        snprintf(fpsText, sizeof(fpsText), "FPS: %.2f", fps);

        float padding = 5.0f;
        float textWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)fpsText);
        float textHeight = 18.0f; // Altura da fonte usada (HELVETICA_18)

        // Desenhar o fundo (um retângulo branco)
        glColor3f(0.0f, 0.0f, 0.0f); // Cor do fundo: branco
        glBegin(GL_QUADS);
            glVertex2f(x - padding, y - padding);
            glVertex2f(x + textWidth + padding, y - padding);
            glVertex2f(x + textWidth + padding, y + textHeight + padding);
            glVertex2f(x - padding, y + textHeight + padding);
        glEnd();

        // Desenhar o texto por cima do fundo
        glColor3f(1.0f, 1.0f, 1.0f);
        glDisable(GL_DEPTH_TEST);
        drawText(fpsText, x, y + textHeight / 2);
        glEnable(GL_DEPTH_TEST);
    }
};

FPSManager fpsManager;


map<unsigned char, bool> keyStates;

void AtualizaMovimento() {
   if (keyStates['w']) {
        obj.currentSpeed += 0.002f; // Aumenta a velocidade ao longo do tempo
        obj.currentSpeed = fmin(obj.currentSpeed, obj.maxSpeed); // V max

        obj.x += sin((obj.y) * 3.14 / 180) * obj.currentSpeed;
        obj.z += cos((obj.y) * 3.14 / 180) * obj.currentSpeed;
    } else {
        obj.currentSpeed -= 0.01f; // Aumenta a velocidade ao longo do tempo
        obj.currentSpeed = fmax(obj.currentSpeed, 0.0); // V max

        obj.x += sin((obj.y) * 3.14 / 180) * obj.currentSpeed;
        obj.z += cos((obj.y) * 3.14 / 180) * obj.currentSpeed;
    }
    // mover para trás
    if (keyStates['s']) {
        obj.x -= sin((obj.y) * 3.14 / 180) * obj.velocidadeMovimento;
        obj.y -= cos((obj.y) * 3.14 / 180) * obj.velocidadeMovimento;
    }
    // girar à esquerda
    if (keyStates['a']) {
        obj.y += obj.velocidadeRotacao;
    }
    // girar à direita
    if (keyStates['d']) {
        obj.y -= obj.velocidadeRotacao;
    }

    // Zoom com 'i' e 'k'
    if (keyStates['i']) {
        cameraScale += obj.velocidadeZoom;
    }

    if (keyStates['k']) {
        if (cameraScale >= 1.0) {
            cameraScale -= obj.velocidadeZoom;
        }
    }
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
            obj.y += 5.0;  // Girar o objeto para a esquerda
            break;
        case GLUT_KEY_RIGHT:
            obj.y -= 5.0;  // Girar o objeto para a direita
            break;
    }

}

void AtualizaCamera() {
    GLfloat rad = (obj.y * 3.14) / 180; // Converter o ângulo para radianos

    // Atualiza a posição da câmera baseada na posição e rotação do objeto
    camera.posX = obj.x - cameraDistance * sin(rad) * cameraScale;
    camera.posZ = obj.z - cameraDistance * cos(rad) * cameraScale;
    camera.posY = cameraHeight * cameraScale;  // Altura fixa da câmera

    // A câmera olha diretamente para o objeto
    camera.lookAt(obj.x, 0.0, obj.z); // ATENÇÂO< TIRAR O OBJY SE DER ERRADO
}




void Inicializa(void) {
    GLint texturePiso, textureObj;

    glClearColor(1.0, 1.0, 1.0, 0.0); // fundo branco
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65, 1, 0.5, 500);
}

class Piso{

    private:
        float escala = 1.0;
        float altura = 0.0;
        GLuint texturePiso;

    public:
        Piso(float escala, float altura){

        };

        void render() const {
            glPushMatrix();
            glTranslatef(0.0, altura, 0.0);
            glScalef(escala, escala, escala);

            glEnable(GL_TEXTURE_2D); // Habilita o uso de texturas
            glBindTexture(GL_TEXTURE_2D, this->texturePiso); // Usa a textura carregada

            glBegin(GL_QUADS);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0, 0.0, 100.0);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0, 0.0, 100.0);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0, 0.0, -100.0);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(100.0, 0.0, -100.0);
            glEnd();

            glDisable(GL_TEXTURE_2D); // Desabilita o uso de texturas
            glPopMatrix();
        }

        void setTexture(GLuint texturePiso){
            this->texturePiso = texturePiso;
        }
};

auto piso = Piso(1.0, -4.0);

const int TARGET_FPS = 60;
const float FRAME_TIME = 1.0f / TARGET_FPS;  // Tempo de cada frame (segundos)

class Meteour {
public:
    GLfloat posX, posY, posZ;
    GLfloat speed;
    double radius;

    Meteour(GLfloat x, GLfloat y, GLfloat z, GLfloat speed, double radius)
        : posX(x), posY(y), posZ(z), speed(speed), radius(radius) {}

    void update() {
        if (posY > 0){
            posY -= speed; // Meteoro cai no eixo Y
        }
        else{
            posY = 100.0f;
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14;
            float distance = 10.0f + static_cast<float>(rand()) / RAND_MAX * 50.0f;
            posX = obj.x + distance * cos(angle);
            posY = obj.z + distance * sin(angle);
        }
    }

    bool hasCollided(GLfloat objX, GLfloat objZ, GLfloat objSize) {
        // Simples verificação de colisão com base nas posições X e Z
        return (fabs(posX - objX) < objSize && fabs(posZ - objZ) < objSize && posY <= 0.0);
    }

    void draw(){
        glPushMatrix();
        glColor3f(0.6f, 0.3f, 0.0f);  // Marrom
        glTranslated(posX, posY, posZ);
        glutSolidSphere(radius, 10, 10);
        glPopMatrix();
    }
};

// Vetor que armazenará os meteoros
std::vector<Meteour> meteous;

// Função para gerar meteoros em posições aleatórias ao redor do jogador
void spawnMeteors(int count) {
    for (int i = 0; i < count; ++i) {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14;
        float distance = 10.0f + static_cast<float>(rand()) / RAND_MAX * 50.0f;

        GLfloat meteorX = obj.x + distance * cos(angle);
        GLfloat meteorZ = obj.z + distance * sin(angle);
        GLfloat meteorSpeed = 0.001f + static_cast<float>(rand()) / RAND_MAX;
        double meteorRadius = 1.0 + static_cast<double>(rand()) / RAND_MAX * 2.0; // Raio entre 1 e 3

        meteous.push_back(Meteour(meteorX, 100.0f, meteorZ, meteorSpeed, meteorRadius)); // Meteoro começa no Y = 10
    }
}

// Função que atualiza e desenha os meteoros
void updateAndDrawMeteors() {
    for (int i = 0; i < meteous.size(); ++i) {
        meteous[i].update();

        // Verificar se o meteoro colidiu com o objeto do jogador
        if (meteous[i].hasCollided(obj.x, obj.z, 5.0f)) {
            printf("Colisão detectada com o meteoro!\n");
            meteous.erase(meteous.begin() + i); // Remove o meteoro se colidir
            --i;
            continue;
        }
        meteous[i].draw();
    }
}

Meteour met = Meteour(0.0f, 100.0f, 0.0f, 0.5f, 2.0);

void Desenha() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa os buffers
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();              // Salva a matriz de projeção atual
    glLoadIdentity();            // Reseta a projeção
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);  // Definir projeção ortogonal (ajustar ao tamanho da janela)

    glMatrixMode(GL_MODELVIEW);  // Mudar para a matriz de modelo/visualização
    glPushMatrix();              // Salva a matriz de visualização
    glLoadIdentity();            // Reseta a visualização

    fpsManager.calculateFPS();
    fpsManager.displayFPS(10, 740);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    piso.render();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    AtualizaCamera();
    AtualizaMovimento();

    for(int i = 0; i < 10; i++){
        meteous[i].update();
        if(meteous[i].hasCollided(obj.x, obj.z, 5.0f)){
            printf("colisao");
        }
        meteous[i].draw();
    }

    glColor3f(1.0, 0.0, 1.0);
    glPushMatrix();
        glTranslatef(obj.x, 0.0, obj.z);
        glRotatef(obj.y, 0.0, 1.0, 0.0);
        obj.render();
    glPopMatrix();


    glutPostRedisplay();
    glutSwapBuffers(); // Troca os buffers
    glFlush(); // Garante que todas as operações estão completas
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Meteour Running");

    GLuint texturePiso, textureObj;
    TestureHelper::LoadTexture("assets/textura_piso.jpg", texturePiso);
    TestureHelper::LoadTexture("assets/textura_piso.jpg", textureObj);

    spawnMeteors(10);

    std::cout << textureObj << std::endl;
    std::cout << texturePiso << std::endl;


    Inicializa();
    obj.load("bonde.obj");
    obj.setTexture(textureObj);
    piso.setTexture(texturePiso);


        glutDisplayFunc(Desenha);
        glutKeyboardFunc(Teclado);
        glutKeyboardUpFunc(TecladoUp);
        glutMouseFunc([](int button, int state, int x, int y) { camera.mouseButton(button, state, x, y); });
        glutMotionFunc([](int x, int y) { camera.mouseMotion(x, y); });

    glutMainLoop();

    return 0;
}
