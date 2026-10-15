#include <GL/freeglut.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "Audio.h"
#include "ObjLoader.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace {
constexpr float pi = 3.14159265f;
constexpr float groundHalfSize = 200.0f;
constexpr float forwardSpeed = 38.0f;
constexpr float reverseSpeed = 18.0f;
constexpr int maxHealth = 100;
constexpr int meteorCount = 12;
constexpr float carHalfWidth = 3.4f;
constexpr float carHalfLength = 8.0f;
constexpr float carHeight = 5.3f;

struct Meteor {
    float x, y, z, radius, speed, spin;
};

struct Impact {
    float x, z, age;
    bool car;
};

std::vector<Meteor> meteors;
std::vector<Impact> impacts;
std::mt19937 randomEngine(std::random_device{}());
ObjLoader car;
Audio audio;
Camera camera;
GLuint groundTexture = 0, skyTexture = 0, meteorTexture = 0;
GLUquadric* sphere = nullptr;
std::filesystem::path assetRoot;
std::array<bool, 256> keys{};
int width = 1280, height = 720, health = maxHealth, hits = 0;
float heading = 0, velocity = 0, survival = 0, invulnerability = 0, flash = 0;
int lastTick = 0;
bool gameOver = false;

float randomFloat(float low, float high) {
    return std::uniform_real_distribution<float>(low, high)(randomEngine);
}

GLuint loadTexture(const std::filesystem::path& path, bool repeat = false) {
    int w, h, channels;
    stbi_uc* pixels = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
    if (!pixels) {
        std::cerr << "Falha ao carregar " << path << ": " << stbi_failure_reason() << '\n';
        return 0;
    }
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    stbi_image_free(pixels);
    return id;
}

GLuint createMeteorTexture() {
    constexpr int size = 128;
    std::array<unsigned char, size * size * 3> pixels{};
    // Tiled, deterministic mottled rock; no external meteor asset required.
    std::mt19937 rng(67);
    std::array<float, 16 * 16> noise{};
    for (float& n : noise)
        n = std::uniform_real_distribution<float>(0, 1)(rng);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) {
            const int coarse = noise[(y / 8) * 16 + x / 8] * 75;
            const int fine = static_cast<int>(rng() % 38);
            const int i = (y * size + x) * 3;
            pixels[i] = static_cast<unsigned char>(85 + coarse + fine);
            pixels[i + 1] = static_cast<unsigned char>(57 + coarse / 2 + fine / 2);
            pixels[i + 2] = static_cast<unsigned char>(43 + coarse / 3 + fine / 3);
        }
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return id;
}

void resetMeteor(Meteor& m, bool initial = false) {
    m.radius = randomFloat(1.6f, 3.4f);
    const float meteorLimit = groundHalfSize - m.radius;
    m.x = std::clamp(car.x + randomFloat(-42, 42), -meteorLimit, meteorLimit);
    m.z = std::clamp(car.z + randomFloat(-42, 42), -meteorLimit, meteorLimit);
    m.y = randomFloat(initial ? 28.0f : 90.0f, initial ? 105.0f : 145.0f);
    m.speed = randomFloat(24.0f, 40.0f);
    m.spin = randomFloat(0, 360);
}

bool touchesCar(const Meteor& m) {
    // Transform the meteor into car space, then test a sphere against the car box.
    const float angle = heading * pi / 180.0f;
    const float dx = m.x - car.x, dz = m.z - car.z;
    const float localX = dx * std::cos(angle) - dz * std::sin(angle);
    const float localZ = dx * std::sin(angle) + dz * std::cos(angle);
    const float nearestX = std::clamp(localX, -carHalfWidth, carHalfWidth);
    const float nearestY = std::clamp(m.y, 0.0f, carHeight);
    const float nearestZ = std::clamp(localZ, -carHalfLength, carHalfLength);
    const float distanceSquared = (localX - nearestX) * (localX - nearestX) +
                                  (m.y - nearestY) * (m.y - nearestY) +
                                  (localZ - nearestZ) * (localZ - nearestZ);
    return distanceSquared <= m.radius * m.radius;
}

void restart() {
    car.x = car.z = 0;
    heading = velocity = survival = invulnerability = flash = 0;
    health = maxHealth;
    hits = 0;
    gameOver = false;
    impacts.clear();
    for (auto& m : meteors)
        resetMeteor(m, true);

    lastTick = glutGet(GLUT_ELAPSED_TIME);
}

void update(float dt) {
    flash = std::max(0.0f, flash - dt * 2.4f);
    for (auto& impact : impacts)
        impact.age += dt;
    impacts.erase(std::remove_if(impacts.begin(), impacts.end(),
                                 [](const Impact& i) { return i.age >= 0.75f; }),
                  impacts.end());

    if (gameOver) {
        audio.update(0, 0, dt, false);
        return;
    }

    survival += dt;
    invulnerability = std::max(0.0f, invulnerability - dt);

    const float drive = (keys['w'] ? 1.0f : 0.0f) - (keys['s'] ? 1.0f : 0.0f);
    const float target = drive * (drive < 0 ? reverseSpeed : forwardSpeed);
    velocity += (target - velocity) * std::min(1.0f, dt * (drive ? 4.0f : 2.5f));
    const float steering = (keys['a'] ? 1.0f : 0.0f) - (keys['d'] ? 1.0f : 0.0f);
    heading += steering * 110.0f * dt;

    const float angle = heading * pi / 180.0f;
    // Keep the rotated car on the same ground quad that drawGround renders.
    const float halfX =
        std::abs(std::cos(angle)) * carHalfWidth + std::abs(std::sin(angle)) * carHalfLength;
    const float halfZ =
        std::abs(std::sin(angle)) * carHalfWidth + std::abs(std::cos(angle)) * carHalfLength;
    const float xLimit = groundHalfSize - halfX;
    const float zLimit = groundHalfSize - halfZ;
    car.x = std::clamp(car.x + std::sin(angle) * velocity * dt, -xLimit, xLimit);
    car.z = std::clamp(car.z + std::cos(angle) * velocity * dt, -zLimit, zLimit);

    for (auto& m : meteors) {
        m.y -= m.speed * dt;
        m.spin += 32.0f * dt;
        if (touchesCar(m)) {
            impacts.push_back({m.x, m.z, 0, true});
            audio.carImpact();
            if (invulnerability == 0) {
                health = std::max(0, health - 20);
                ++hits;
                flash = 0.7f;
                invulnerability = 1.0f;
                if (health == 0) {
                    gameOver = true;
                    audio.gameOver();
                }
            }
            resetMeteor(m);
        } else if (m.y <= m.radius) {
            impacts.push_back({m.x, m.z, 0, false});
            audio.groundImpact(std::hypot(m.x - car.x, m.z - car.z));
            resetMeteor(m);
        }
    }

    float meteorIntensity = 0;
    for (const auto& m : meteors) {
        const float horizontal = std::hypot(m.x - car.x, m.z - car.z);
        const float nearness = std::max(0.0f, 1.0f - horizontal / 40.0f);
        const float descent = std::max(0.0f, 1.0f - m.y / 100.0f);
        meteorIntensity = std::max(meteorIntensity, nearness * descent);
    }

    audio.update(std::abs(velocity) / forwardSpeed, meteorIntensity, dt, !gameOver);
}

void drawGround() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glColor3f(0.76f, 0.79f, 0.72f);
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-groundHalfSize, -0.04f, -groundHalfSize);
    glTexCoord2f(40, 0);
    glVertex3f(groundHalfSize, -0.04f, -groundHalfSize);
    glTexCoord2f(40, 40);
    glVertex3f(groundHalfSize, -0.04f, groundHalfSize);
    glTexCoord2f(0, 40);
    glVertex3f(-groundHalfSize, -0.04f, groundHalfSize);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawSky() {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTexture);
    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(car.x, 0, car.z);
    gluQuadricOrientation(sphere, GLU_INSIDE);
    gluSphere(sphere, 230, 32, 20);
    gluQuadricOrientation(sphere, GLU_OUTSIDE);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void drawMeteors() {
    for (const auto& m : meteors) {
        glPushMatrix();
        glTranslatef(m.x, m.y, m.z);
        glRotatef(m.spin, 0.4f, 1, 0.2f);
        glColor3f(1, 0.83f, 0.64f);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, meteorTexture);
        gluSphere(sphere, m.radius, 12, 10);
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& m : meteors) {
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1, 0.42f, 0.1f, 0.55f);
        glVertex3f(m.x, m.y + m.radius * 0.5f, m.z);
        glColor4f(1, 0.2f, 0.02f, 0);
        for (int i = 0; i <= 12; ++i) {
            const float a = i * 2 * pi / 12;
            glVertex3f(m.x + std::cos(a) * m.radius * 0.8f, m.y + m.radius * 4.5f,
                       m.z + std::sin(a) * m.radius * 0.8f);
        }
        glEnd();
    }

    for (const auto& impact : impacts) {
        const float t = impact.age / 0.75f;
        const float radius = (impact.car ? 2.0f : 0.8f) + t * 8.0f;
        glLineWidth(3.0f);
        glColor4f(1, impact.car ? 0.15f : 0.55f, 0.05f, 1 - t);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 32; ++i) {
            const float a = i * 2 * pi / 32;
            glVertex3f(impact.x + std::cos(a) * radius, 0.08f, impact.z + std::sin(a) * radius);
        }
        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void rectangle(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void label(float x, float y, const std::string& value, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (unsigned char c : value)
        glutBitmapCharacter(font, c);
}

void drawHud() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, height, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.06f, 0.09f, 0.13f, 0.78f);
    rectangle(16, 16, 316, 84);

    glColor3f(1, 1, 1);
    char text[100];
    std::snprintf(text, sizeof(text), "VIDA  %d / %d", health, maxHealth);
    label(30, 43, text);
    glColor3f(0.25f, 0.28f, 0.30f);
    rectangle(30, 54, 286, 20);
    glColor3f(health > 40 ? 0.25f : 0.9f, health > 40 ? 0.78f : 0.26f, 0.23f);
    rectangle(30, 54, 286.0f * health / maxHealth, 20);

    glColor3f(1, 1, 1);
    std::snprintf(text, sizeof(text), "Tempo: %.0fs    Impactos: %d", survival, hits);
    label(30, 94, text, GLUT_BITMAP_HELVETICA_12);
    label(22, height - 24, "W/S mover  |  A/D virar  |  R reiniciar  |  M som  |  Q sair",
          GLUT_BITMAP_HELVETICA_12);
    if (audio.isMuted())
        label(width - 105, 30, "SOM MUDO", GLUT_BITMAP_HELVETICA_12);

    if (flash > 0) {
        glColor4f(1, 0.08f, 0.02f, flash * 0.32f);
        rectangle(0, 0, width, height);
    }

    if (gameOver) {
        glColor4f(0.02f, 0.03f, 0.07f, 0.78f);
        rectangle(0, 0, width, height);
        glColor3f(1, 0.38f, 0.2f);
        label(width / 2.0f - 84, height / 2.0f - 24, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(1, 1, 1);
        std::snprintf(text, sizeof(text), "Sobreviveu %.0f segundos", survival);
        label(width / 2.0f - 100, height / 2.0f + 12, text);
        label(width / 2.0f - 111, height / 2.0f + 46, "Pressione R para jogar novamente");
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.apply(car.x, car.z, heading);

    GLfloat sun[] = {-60, 90, 40, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, sun);

    drawSky();
    drawGround();
    drawMeteors();

    if (invulnerability == 0 || static_cast<int>(invulnerability * 12) % 2 == 0) {
        glPushMatrix();
        glTranslatef(car.x, 0, car.z);
        glRotatef(heading, 0, 1, 0);
        glScalef(4, 4, 4);
        car.draw();
        glPopMatrix();
    }

    drawHud();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    width = std::max(1, w);
    height = std::max(1, h);
    camera.reshape(width, height);
}

void keyDown(unsigned char key, int, int) {
    if (key == 27 || key == 'q' || key == 'Q')
        std::exit(0);
    if (key == 'r' || key == 'R')
        restart();
    if (key == 'm' || key == 'M')
        audio.toggleMute();

    keys[static_cast<unsigned char>(std::tolower(key))] = true;
}

void keyUp(unsigned char key, int, int) {
    keys[static_cast<unsigned char>(std::tolower(key))] = false;
}

void tick(int) {
    const int now = glutGet(GLUT_ELAPSED_TIME);
    const float dt = std::clamp((now - lastTick) / 1000.0f, 0.0f, 0.05f);
    lastTick = now;
    update(dt);
    glutPostRedisplay();
    glutTimerFunc(16, tick, 0);
}
} // namespace

int main(int argc, char** argv) {
    assetRoot = std::filesystem::absolute(argv[0]).parent_path();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutCreateWindow("Meteour Running");

    glClearColor(0.31f, 0.49f, 0.7f, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    GLfloat ambient[] = {0.36f, 0.37f, 0.4f, 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    groundTexture = loadTexture(assetRoot / "assets/textura_piso.jpg", true);
    skyTexture = loadTexture(assetRoot / "assets/skybox.jpg", true);
    meteorTexture = createMeteorTexture();
    sphere = gluNewQuadric();
    gluQuadricTexture(sphere, GL_TRUE);
    if (!car.load((assetRoot / "LowPolyCars.obj").string()))
        return 1;

    car.setTexture(loadTexture(assetRoot / "assets/Car Texture 1.png"));
    audio.init(assetRoot);

    meteors.resize(meteorCount);
    restart();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutIgnoreKeyRepeat(1);
    glutTimerFunc(16, tick, 0);

    glutMainLoop();
    gluDeleteQuadric(sphere);
    return 0;
}
