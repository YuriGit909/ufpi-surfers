#ifndef PBR_MODEL_H
#define PBR_MODEL_H

#include <string>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include "shader.h"

struct PBRVertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    float tx, ty, tz;
};

class PBRModel
{
public:
    PBRModel(
        const std::string& modelPath,
        const std::string& baseColorPath,
        const std::string& normalPath,
        const std::string& metallicRoughnessPath
    );

    void draw();

private:
    std::vector<PBRVertex> vertices;

    GLuint vao = 0;
    GLuint vbo = 0;

    GLuint baseColorTexture = 0;
    GLuint normalTexture = 0;
    GLuint metallicRoughnessTexture = 0;

    Shader* shader = nullptr;

    GLuint loadTexture(const std::string& path);
    void setupMesh();
};

#endif