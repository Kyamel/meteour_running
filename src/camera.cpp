#include "camera.h"

#include <GL/freeglut.h>
#include <cmath>

namespace {
constexpr float pi = 3.14159265f;
constexpr float chaseDistance = 23.0f;
constexpr float chaseHeight = 15.0f;
constexpr float targetHeight = 2.0f;
} // namespace

void Camera::apply(float carX, float carZ, float headingDegrees) const {
    const float angle = headingDegrees * pi / 180.0f;
    gluLookAt(carX - std::sin(angle) * chaseDistance, chaseHeight,
              carZ - std::cos(angle) * chaseDistance, carX, targetHeight, carZ, 0, 1, 0);
}

void Camera::reshape(int width, int height) const {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65, static_cast<double>(width) / height, 0.5, 500);
    glMatrixMode(GL_MODELVIEW);
}
