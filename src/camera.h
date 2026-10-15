#ifndef CAMERA_H
#define CAMERA_H

class Camera {
  public:
    void apply(float carX, float carZ, float headingDegrees) const;
    void reshape(int width, int height) const;
};

#endif
