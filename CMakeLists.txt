cmake_minimum_required(VERSION 3.16)
project(MeteourRunning VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(MeteourRunning src/main.cpp src/ObjLoader.cpp)
target_link_libraries(MeteourRunning PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)

# Keep assets next to the executable, including with multi-config generators.
add_custom_command(TARGET MeteourRunning POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:MeteourRunning>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/LowPolyCars.obj" "$<TARGET_FILE_DIR:MeteourRunning>/LowPolyCars.obj"
)
install(TARGETS MeteourRunning RUNTIME DESTINATION bin)
install(DIRECTORY assets DESTINATION bin)
install(FILES LowPolyCars.obj DESTINATION bin)
