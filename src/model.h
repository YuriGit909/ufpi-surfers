#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <GL/glut.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    float tx, ty, tz;
};

struct MeshData {
    std::vector<Vertex> vertices;

    float r = 0.8f;
    float g = 0.8f;
    float b = 0.8f;
    float opacity = 1.0f;

    std::string materialName;

    bool hasDiffuse = false;
    bool hasNormal = false;
    bool hasSpecular = false;
    bool hasRoughness = false;

    std::string diffusePath;
    std::string normalPath;
    std::string specularPath;
    std::string roughnessPath;

    GLuint diffuseMap = 0;
    GLuint normalMap = 0;
    GLuint specularMap = 0;
    GLuint roughnessMap = 0;
};

class Model {
public:
    Model(const std::string& path);
    void draw();

private:
    Assimp::Importer importer;
    const aiScene* sceneRef = nullptr;
    std::vector<MeshData> meshes;
};

#endif
