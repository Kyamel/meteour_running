#include "ObjLoader.h"

// Método para carregar o arquivo OBJ
bool ObjLoader::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        processLine(line);
    }

    file.close();
    return true;
}

// Método para renderizar o objeto
void ObjLoader::render() const {
    for (const auto& face : faces) {
        glBegin(GL_POLYGON);
        for (const auto& vertexIndex : face) {
            glVertex3fv(&vertices[vertexIndex[0]][0]);
        }
        glEnd();
    }
}

// Método auxiliar para processar linhas do arquivo
void ObjLoader::processLine(const std::string& line) {
    std::istringstream iss(line);
    std::string prefix;

    if (line.empty() || line[0] == '#') {
        return; // Ignora comentários e linhas vazias
    }

    iss >> prefix;
    if (prefix == "v") { // Vértices
        std::vector<GLfloat> vertex(3);
        iss >> vertex[0] >> vertex[1] >> vertex[2];
        vertices.push_back(vertex);
    } else if (prefix == "vn") { // Normais
        std::vector<GLfloat> normal(3);
        iss >> normal[0] >> normal[1] >> normal[2];
        normais.push_back(normal);
    } else if (prefix == "vt") { // Texturas
        std::vector<GLfloat> texture(2);
        iss >> texture[0] >> texture[1];
        texturas.push_back(texture);
    } else if (prefix == "f") { // Faces
        std::vector<std::vector<GLint>> face;
        std::string vertexData;
        while (iss >> vertexData) {
            std::istringstream vertexStream(vertexData);
            std::string index;
            std::vector<GLint> indices;

            while (std::getline(vertexStream, index, '/')) {
                indices.push_back(std::stoi(index) - 1); // Ajuste de índice
            }

            face.push_back(indices);
        }
        faces.push_back(face);
    }
}
