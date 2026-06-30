#ifndef ANIMATED_MODEL_H
#define ANIMATED_MODEL_H

#include "shader.h"

#include <vector>
#include <string>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

struct AnimatedVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    int boneIDs[4] = {-1, -1, -1, -1};
    float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct AnimatedBoneInfo
{
    int id;
    glm::mat4 offset;
};

struct AnimatedMesh
{
    std::vector<AnimatedVertex> vertices;
    std::vector<unsigned int> indices;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    GLuint textureID = 0;
    bool hasTexture = false;

    std::string materialName;
};

class AnimatedModel
{
public:
    AnimatedModel(
    const std::string& path,
    const std::string& textures = "./assets/player/student/textures/"
);

    void draw();
    void draw(const std::vector<glm::mat4>& boneMatrices);

    std::map<std::string, AnimatedBoneInfo>& getBoneInfoMap();
    int& getBoneCount();

private:
    void loadModel(const std::string& path);
    AnimatedMesh processMesh(aiMesh* mesh);

    void extractBoneWeights(aiMesh* mesh, std::vector<AnimatedVertex>& vertices);
    void setupMesh(AnimatedMesh& mesh);

    void setVertexBoneData(AnimatedVertex& vertex, int boneID, float weight);

    GLuint loadTextureFromFile(const std::string& path);
    GLuint chooseTextureByMaterialName(const std::string& materialName);

    glm::mat4 convertMatrixToGLM(const aiMatrix4x4& from);

private:
    Assimp::Importer importer;
    const aiScene* sceneRef = nullptr;
    std::string textureFolder;
    std::vector<AnimatedMesh> meshes;

    std::map<std::string, AnimatedBoneInfo> boneInfoMap;
    int boneCounter = 0;
};

#endif