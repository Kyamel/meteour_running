#include <iostream>
#include <GL/gl.h>
#include <GL/freeglut.h>
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

class Piso {

    private:
        float escala = 1.0;
        float altura = 0.0;
        GLuint texturePiso;

    public:
        Piso(float escala, float altura){

        };


        void draw(float size) const {
            glPushMatrix();
            glTranslatef(0.0, altura, 0.0);
            glScalef(escala, escala, escala);

            glEnable(GL_TEXTURE_2D); // Habilita o uso de texturas
            glBindTexture(GL_TEXTURE_2D, this->texturePiso); // Usa a textura carregada

            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(size, 0.0, size);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, 0.0, size);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, 0.0, -size);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(size, 0.0, -size);
            glEnd();
            glDisable(GL_TEXTURE_2D); // Desabilita o uso de texturas
            glPopMatrix();
        }

        void setTexture(GLuint texturePiso){
            this->texturePiso = texturePiso;
        }
};

class Meteour {
public:
    GLfloat posX, posY, posZ;
    GLfloat speed;
    double radius;
    bool hasCollidedBefore = false;
    float cooldown;

    Meteour(GLfloat x, GLfloat y, GLfloat z, GLfloat speed, double radius)
        : posX(x), posY(y), posZ(z), speed(speed), radius(radius) {}

    void update() {
        if (posY > -50.0f){
            posY -= speed; // Meteoro cai no eixo Y
        }
        else{
            GLfloat distanceY = 200 + (rand() % 151); // Varía entre 150 e 200
            posY = distanceY;
            // Gera uma distância aleatória entre -100 e 100
            GLfloat distanceX = (static_cast<float>(rand()) / RAND_MAX * 200.0f) - 100.0f;
            GLfloat distanceZ = (static_cast<float>(rand()) / RAND_MAX * 200.0f) - 100.0f;

            GLfloat meteorX = distanceX; // Agora usando distanceX
            GLfloat meteorZ = distanceZ; // Agora usando distanceZ
        }
    }

    bool hasCollided(GLfloat objX, GLfloat objZ, GLfloat objSize) {
        // Simples verificação de colisão com base nas posições X e Z
        return (fabs(posX - objX) < objSize && fabs(posZ - objZ) < objSize && posY <= objSize);
    }

    void draw(){
        glPushMatrix();
        glColor3f(0.6f, 0.3f, 0.0f);  // Marrom
        glTranslated(posX, posY, posZ);
        glutSolidSphere(radius, 10, 10);
        glPopMatrix();
    }
};


class Skybox {
public:
    GLuint textures[6]; // Textures for the six faces of the skybox

    void setTexture(const GLuint texture[6]) {
        for (int i = 0; i < 6; ++i) {
            this->textures[i] = texture[i];
        }
    }

    void draw(float size, Camera camera) {
        glEnable(GL_TEXTURE_2D);  // Enable 2D textures
        glDisable(GL_DEPTH_TEST); // Disable depth testing to ensure skybox is drawn behind everything

        glPushMatrix();
        glLoadIdentity();
        glTranslatef(camera.posX, camera.posY, camera.posZ);

            // Right face
            glBindTexture(GL_TEXTURE_2D, this->textures[0]);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); glVertex3f( size, -size, -size);
                glTexCoord2f(1.0, 0.0); glVertex3f( size, -size,  size);
                glTexCoord2f(1.0, 1.0); glVertex3f( size,  size,  size);
                glTexCoord2f(0.0, 1.0); glVertex3f( size,  size, -size);
            glEnd();

            // Left face
            glBindTexture(GL_TEXTURE_2D, this->textures[1]);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size,  size);
                glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size, -size);
                glTexCoord2f(1.0, 1.0); glVertex3f(-size,  size, -size);
                glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size,  size);
            glEnd();

            // Top face
            glBindTexture(GL_TEXTURE_2D, this->textures[2]);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); glVertex3f(-size,  size, -size);
                glTexCoord2f(1.0, 0.0); glVertex3f( size,  size, -size);
                glTexCoord2f(1.0, 1.0); glVertex3f( size,  size,  size);
                glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size,  size);
            glEnd();

            // Bottom face
            glBindTexture(GL_TEXTURE_2D, this->textures[3]);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size, -size);
                glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size,  size);
                glTexCoord2f(1.0, 1.0); glVertex3f( size, -size,  size);
                glTexCoord2f(0.0, 1.0); glVertex3f( size, -size, -size);
            glEnd();

            // Front face
            glBindTexture(GL_TEXTURE_2D, this->textures[4]);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size,  size);
                glTexCoord2f(1.0, 0.0); glVertex3f( size, -size,  size);
                glTexCoord2f(1.0, 1.0); glVertex3f( size,  size,  size);
                glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size,  size);
            glEnd();

            // Back face
            glBindTexture(GL_TEXTURE_2D, this->textures[5]);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0); glVertex3f( size, -size, -size);
                glTexCoord2f(1.0, 0.0); glVertex3f(-size, -size, -size);
                glTexCoord2f(1.0, 1.0); glVertex3f(-size,  size, -size);
                glTexCoord2f(0.0, 1.0); glVertex3f( size,  size, -size);
            glEnd();

        glPopMatrix();

        glEnable(GL_DEPTH_TEST); // Re-enable depth testing
        glDisable(GL_TEXTURE_2D); // Disable textures

    }
};

void drawLighting() {
    glEnable(GL_LIGHTING); // Ativa a iluminação
    glEnable(GL_LIGHT0);   // Ativa a fonte de luz 0

    // Propriedades da luz
    GLfloat light_position[] = { 0.0f, 10.0f, 10.0f, 1.0f };// Posição da luz
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };   // Cor difusa da luz
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Cor especular da luz

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);      // Define a posição da luz
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);        // Define a cor difusa da luz
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);      // Define a cor especular da luz

    // Propriedades do material
    GLfloat mat_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };     // Cor difusa do material
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };    // Cor especular do material
    GLfloat mat_shininess[] = { 50.0f };                    // Brilho do material

    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);        // Define a cor difusa do material
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);      // Define a cor especular do material
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);    // Define o brilho do material
}

void glInit(void) {
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

// ----------------------------------------------------------------------------
// Constantes
// ----------------------------------------------------------------------------

const float xLimit = 100.0f;
const float zLimit = 100.0f;
const float yLimit = 100.0f;
const int screenWidth = 1280;
const int screenHeight = 720;
const int TARGET_FPS = 60;
const float FRAME_TIME = 1.0f / TARGET_FPS;  // Tempo de cada frame (segundos)

// ----------------------------------------------------------------------------
// Variáveis globais
// ----------------------------------------------------------------------------

Meteour met = Meteour(0.0f, 100.0f, 0.0f, 0.5f, 2.0);
Skybox skybox = Skybox();
std::vector<Meteour> meteous;
FPSManager fpsManager;
Piso piso = Piso(1.0, -0.0);

ObjLoader obj;
GLfloat cameraDistance = 5.0;
GLfloat cameraHeight = 5.0;
GLfloat cameraScale = 1.0;
Camera camera(obj.x - cameraDistance, cameraHeight, obj.z - cameraDistance);

map<unsigned char, bool> keyStates;
int mouseX = 0, mouseY = 0;
int hits = 0;

// ----------------------------------------------------------------------------
// Gerenciar Eventos e Desenhar
// ----------------------------------------------------------------------------

void Desenha();
void Teclado(unsigned char key, int x, int y);
void TecladoUp(unsigned char key, int x, int y);
void AtualizaMovimento(void);
void AtualizaCamera(int mouseX, int mouseY);
void idle(int);

// Meteoros
void spawnMeteors(int count);
void updateAndDrawMeteors();

// ----------------------------------------------------------------------------
// Função principal
// ----------------------------------------------------------------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Meteour Running");

    GLuint texturePiso, textureObj;
    GLuint skyboxTextures[6];

    TestureHelper::LoadTexture("assets/textura_piso.jpg", texturePiso);
    TestureHelper::LoadTexture("assets/Car Texture 1.png", textureObj);

    TestureHelper::LoadTexture("assets/skybox.jpg", skyboxTextures[0]);   // Direita
    TestureHelper::LoadTexture("assets/skybox.jpg", skyboxTextures[1]);   // Esquerda
    TestureHelper::LoadTexture("assets/skybox.jpg", skyboxTextures[2]);   // Cima
    TestureHelper::LoadTexture("assets/skybox.jpg", skyboxTextures[3]);   // Baixo
    TestureHelper::LoadTexture("assets/skybox.jpg", skyboxTextures[4]);   // Frente
    TestureHelper::LoadTexture("assets/skybox.jpg", skyboxTextures[5]);   // Trás

    skybox.setTexture(skyboxTextures);
    spawnMeteors(10);
    Inicializa();
    obj.load("LowPolyCars.obj");
    obj.setTexture(textureObj);
    piso.setTexture(texturePiso);

        glutDisplayFunc(Desenha);
        glutKeyboardFunc(Teclado);
        glutKeyboardUpFunc(TecladoUp);
        // glutReshapeFunc(reshape);
        glutMotionFunc([](int x, int y) { camera.mouseMotion(x, y); });
        glutMouseFunc([](int button, int state, int x, int y) { camera.mouseButton(button, state, x, y); });
        glutTimerFunc(1000/60, idle, 0);
        glutMainLoop();

    return 0;
}

// ----------------------------------------------------------------------------
// Gerenciar Eventos e Desenhar
// ----------------------------------------------------------------------------

void idle(int){
    glutPostRedisplay();
    glutTimerFunc(1000/60, idle, 0);
}

void Desenha() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa os buffers
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    drawLighting();
    skybox.draw(300.0f, camera);
    piso.draw(200.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    AtualizaCamera(mouseX, mouseY);
    AtualizaMovimento();
    updateAndDrawMeteors();

    glColor3f(1.0, 0.0, 1.0);
    glPushMatrix();
        glTranslatef(obj.x, 0.0, obj.z);
        glRotatef(obj.y, 0.0, 1.0, 0.0);
        obj.draw();
    glPopMatrix();

    glutSwapBuffers();
    //glFlush(); // Garante que todas as operações estão completas

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);  // Definir projeção ortogonal (ajustar ao tamanho da janela)

    glMatrixMode(GL_MODELVIEW);  // Mudar para a matriz de modelo/visualização
    glPushMatrix();
    glLoadIdentity();

    fpsManager.calculateFPS();
    fpsManager.displayFPS(10, 740);

    glutPostRedisplay();
}

void Teclado(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') {
        exit(0);
    }
    if (key == 'c' || key == 'C') {
        camera.isSideView = !camera.isSideView;
    }
    if (key == 'f' || key == 'F') {
        camera.freeCamera = !camera.freeCamera;
    }
    keyStates[key] = true;  // Marca a tecla como pressionada
    //AtualizaMovimento();
}

void AtualizaMovimento(void) {
   if (keyStates['w']) {
        obj.currentSpeed += 0.0001f; // Aumenta a velocidade ao longo do tempo
        obj.currentSpeed = fmin(obj.currentSpeed, obj.maxSpeed); // V max

        // Calcula as novas posições
        float newX = obj.x + sin((obj.y) * 3.14 / 180) * obj.currentSpeed;
        float newZ = obj.z + cos((obj.y) * 3.14 / 180) * obj.currentSpeed;

        // Verifica se as novas posições estão dentro dos limites
        if (newX > xLimit) {
            obj.x = xLimit;
        } else if (newX < -xLimit) {
            obj.x = -xLimit;
        } else {
            obj.x = newX;
        }

        if (newZ > zLimit) {
            obj.z = zLimit;
        } else if (newZ < -zLimit) {
            obj.z = -zLimit;
        } else {
            obj.z = newZ;
        }


    } else {
        obj.currentSpeed -= 0.0001f; // Aumenta a velocidade ao longo do tempo
        obj.currentSpeed = fmax(obj.currentSpeed, 0.0); // V max

        float newX = obj.x + sin((obj.y) * 3.14 / 180) * obj.currentSpeed;
        float newZ = obj.z + cos((obj.y) * 3.14 / 180) * obj.currentSpeed;

        // Verifica se as novas posições estão dentro dos limites
        if (newX > xLimit) {
            obj.x = xLimit;
        } else if (newX < -xLimit) {
            obj.x = -xLimit;
        } else {
            obj.x = newX;
        }

        if (newZ > zLimit) {
            obj.z = zLimit;
        } else if (newZ < -zLimit) {
            obj.z = -zLimit;
        } else {
            obj.z = newZ;
        }

    }
    // mover para trás
    if (keyStates['s']) {
        obj.x -= sin((obj.y) * 3.14 / 180) * obj.velocidadeMovimento;
        obj.z -= cos((obj.y) * 3.14 / 180) * obj.velocidadeMovimento;
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
        if (cameraScale >= 20.0) {
            cameraScale = 20.0;
        }
    }

    if (keyStates['k']) {
        if (cameraScale >= 1.0) {
            cameraScale -= obj.velocidadeZoom;
        }
    }
}

void TecladoUp(unsigned char key, int x, int y) {
    keyStates[key] = false; // Marca a tecla como solta
}

void AtualizaCamera(int mouseX, int mouseY) {
    if (camera.freeCamera){
        return;
    } else{
        if (!camera.isSideView) {
            GLfloat rad = (obj.y * 3.14) / 180; // Converter o ângulo para radianos

            // Atualiza a posição da câmera baseada na posição e rotação do objeto
            camera.posX = obj.x - cameraDistance * sin(rad) * cameraScale;
            camera.posZ = obj.z - cameraDistance * cos(rad) * cameraScale;
            camera.posY = cameraHeight * cameraScale;  // Altura fixa da câmera

            // A câmera em cima do objeto
            camera.lookAt(obj.x, 0.0, obj.z);
        }
        if (camera.isSideView) {
            camera.lookAt(obj.x - 10.0f, 0.0, obj.z); // Câmera global focada no objeto
        }
    }

}

// ------------------------------------------------------------------
// Meteroros
// ------------------------------------------------------------------


// Função para gerar meteoros em posições aleatórias ao redor do jogador
void spawnMeteors(int count) {
    for (int i = 0; i < count; ++i) {
        // Gera uma distância aleatória entre -100 e 100
        GLfloat distanceX = (static_cast<float>(rand()) / RAND_MAX * 200.0f) - 100.0f;
        GLfloat distanceZ = (static_cast<float>(rand()) / RAND_MAX * 200.0f) - 100.0f;

        GLfloat meteorX = obj.x + distanceX;
        GLfloat meteorZ = obj.z + distanceZ;
        GLfloat meteorSpeed = 0.01f;
        double meteorRadius = 1.0 + static_cast<double>(rand()) / RAND_MAX * 2.0;

        meteous.push_back(Meteour(meteorX, 300.0f, meteorZ, meteorSpeed, meteorRadius * 8.0f)); // Meteoro começa no Y = 10
    }
}

// Função para desenhar uma esfera (ou qualquer forma que você queira usar como efeito visual)
void drawCollisionEffect(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(1.0f, 0.0f, 0.0f); // Cor vermelha para o efeito de colisão
    glutSolidSphere(0.5f, 20, 20); // Desenha uma esfera sólida
    glPopMatrix();
}

// Função que desenha texto na tela
void renderBitmapString(float x, float y, void *font, const char *string) {
    glRasterPos2f(x, y); // Define a posição onde o texto será desenhado
    for (const char *c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c); // Desenha cada caractere da string
    }
}

// Função que atualiza e desenha os meteoros
void updateAndDrawMeteors() {

    for (int i = 0; i < meteous.size(); ++i) {
        meteous[i].update();

        // Verificar se o meteoro colidiu com o objeto do jogador
        if (meteous[i].hasCollided(obj.x, obj.z, 5.0f) && !meteous[i].hasCollidedBefore) {
            printf("Colisao!\n");
            drawCollisionEffect(obj.x, obj.y, obj.z);
            hits++;
            meteous[i].hasCollidedBefore = true;
        } else if (!meteous[i].hasCollided(obj.x, obj.z, 5.0f) ) {
            meteous[i].hasCollidedBefore = false;
        }
        meteous[i].draw();
    }
    // Desenhar o valor de hits na tela
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Hits: %d", hits);
    GLfloat rad = (obj.y * 3.14) / 180; // Converter o ângulo para radianos
    renderBitmapString(-0.9f, 0.9f, GLUT_BITMAP_HELVETICA_18, buffer);
}