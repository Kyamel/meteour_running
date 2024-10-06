#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <vector>
#include <string>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/freeglut.h>

class ObjLoader {
public:
    bool load(const std::string& filename);
    void render() const;
    void setTexture(GLuint texture); // Declaração do método setTexture

    GLfloat x = 0.0, z = 0.0, y = 0.0;
    float velocidadeMovimento = 0.2f;
    float velocidadeRotacao = 1.0f;
    float velocidadeZoom = 0.1f;

    float currentSpeed = 0.1f;
    const float maxSpeed = 0.4f;

private:
    void processLine(const std::string& line);
    
    std::vector<std::vector<GLfloat>> vertices;
    std::vector<std::vector<GLfloat>> normais;
    std::vector<std::vector<GLfloat>> texturas;
    std::vector<std::vector<std::vector<GLint>>> faces;

    GLuint texture;


};


#endif // OBJ_DATA_H
