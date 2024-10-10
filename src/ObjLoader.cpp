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
void ObjLoader::draw() const {
    // Ativar a textura
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, this->texture);

    for (const auto& face : faces) {
        glBegin(GL_POLYGON); // ou GL_TRIANGLES se necessário
        glColor3f(1.0f, 1.0f, 1.0f);
        glScalef(4.0f, 4.0f, 4.0f);

        for (const auto& vertexIndex : face) {
            // Aplica a normal, se disponível
            if (vertexIndex.size() > 2 && vertexIndex[2] >= 0) {
                glNormal3fv(&normais[vertexIndex[2]][0]);
            }
            // Aplica a coordenada de textura, se disponível
            if (vertexIndex.size() > 1 && vertexIndex[1] >= 0) {
                glTexCoord2fv(&texturas[vertexIndex[1]][0]);
            }
            // Renderiza o vértice
            glVertex3fv(&vertices[vertexIndex[0]][0]);
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D); // Desativa a textura após renderizar
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

void ObjLoader::setTexture(GLuint texture){
    this->texture = texture;
}
