#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <GL/glut.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <map>
#include <glm/glm.hpp>

struct BoneInfo
{
    int id;
    glm::mat4 offset;
};

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    float tx, ty, tz;

    int boneIDs[4] = {-1, -1, -1, -1};
    float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
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

    std::map<std::string, BoneInfo>& getBoneInfoMap()
    {
        return boneInfoMap;
    }

    int& getBoneCount()
    {
        return boneCounter;
    }

private:
    Assimp::Importer importer;
    const aiScene* sceneRef = nullptr;

    std::vector<MeshData> meshes;

    std::map<std::string, BoneInfo> boneInfoMap;
    int boneCounter = 0;
};

#endif
