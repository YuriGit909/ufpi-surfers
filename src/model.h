#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <GL/glut.h>

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct MeshData {
    std::vector<Vertex> vertices;

    float r = 0.8f;
    float g = 0.8f;
    float b = 0.8f;

    float opacity = 1.0f;
    std::string materialName;

    bool hasTexture = false;
    GLuint textureID = 0;
};

class Model {
public:
    Model(const std::string& path);
    void draw();

private:
    std::vector<MeshData> meshes;
};

#endif