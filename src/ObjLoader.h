#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <vector>
#include <string>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>

class ObjLoader {
public:
    // Armazenamento das informações do objeto
    std::vector<std::vector<GLfloat>> vertices;    // Vértices
    std::vector<std::vector<GLfloat>> normais;      // Normais
    std::vector<std::vector<GLfloat>> texturas;     // Texturas
    std::vector<std::vector<std::vector<GLint>>> faces; // Faces

    // Método para carregar o arquivo OBJ
    bool load(const std::string& filename);

    // Método para renderizar o objeto (exemplo simples)
    void render() const;

private:
    // Método auxiliar para processar linhas do arquivo
    void processLine(const std::string& line);
};

#endif // OBJ_DATA_H
