cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(MeteourRunning CXX)

# Definir compiladores (opcional, use ao chamar cmake se preferir)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# Ativar compilação com C++17 ou mais recente
set(CMAKE_CXX_STANDARD 17)

# Definir o caminho do MSYS2
set(MSYS2_PATH "C:/msys64/mingw64")

# Incluir os diretórios onde o FreeGLUT e OpenGL estão instalados
include_directories("${MSYS2_PATH}/include")
include_directories(${CMAKE_SOURCE_DIR}/include)

# Linkar bibliotecas do MSYS2
link_directories("${MSYS2_PATH}/lib")

# Adicionar os arquivos de código fonte
set(SOURCES
    src/main.cpp
    src/ObjLoader.cpp
)

# Encontrar e linkar as bibliotecas necessárias
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
    message(FATAL_ERROR "OpenGL not found!")
endif()

find_package(GLUT REQUIRED)
if(NOT GLUT_FOUND)
    message(FATAL_ERROR "GLUT not found!")
endif()

# Adicionar o executável
set(EXECUTABLE_NAME "MeteourRunning")
add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} OpenGL::GL GLUT::GLUT)

# Opcional: Adicionar uma regra de instalação
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
