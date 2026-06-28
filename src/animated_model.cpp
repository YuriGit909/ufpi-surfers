#include "animated_model.h"

#include <iostream>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/postprocess.h>
#include "external/stb_image.h"

using namespace std;

static Shader animatedShader;
static bool animatedShaderLoaded = false;

static void tryLoadAnimatedShader()
{
    if (animatedShaderLoaded)
        return;

    animatedShaderLoaded = true;

    if (!animatedShader.load(
            "./assets/shaders/animated_model.vert",
            "./assets/shaders/animated_model.frag"))
    {
        cout << "Erro ao carregar shader animado." << endl;
    }
}

AnimatedModel::AnimatedModel(const std::string& path)
{
    loadModel(path);
}

void AnimatedModel::loadModel(const std::string& path)
{
    sceneRef = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs
    );

    if (!sceneRef || !sceneRef->HasMeshes())
    {
        cout << "Erro ao carregar AnimatedModel: " << path << endl;
        return;
    }

    for (unsigned int i = 0; i < sceneRef->mNumMeshes; i++)
    {
        AnimatedMesh mesh = processMesh(sceneRef->mMeshes[i]);
        setupMesh(mesh);
        meshes.push_back(mesh);
    }

    cout << "AnimatedModel carregado: " << path << endl;
    cout << "Meshes: " << meshes.size() << endl;
    cout << "Bones: " << boneCounter << endl;
}

AnimatedMesh AnimatedModel::processMesh(aiMesh* mesh)
{
    AnimatedMesh result;

    if (sceneRef->HasMaterials())
    {
        aiMaterial* material = sceneRef->mMaterials[mesh->mMaterialIndex];

        aiString materialName;
        if (material->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS)
            result.materialName = materialName.C_Str();

        result.textureID = chooseTextureByMaterialName(result.materialName);
        result.hasTexture = result.textureID != 0;

        cout << "Material: " << result.materialName
             << " | textura: " << (result.hasTexture ? "SIM" : "NAO") << endl;
    }

    result.vertices.resize(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        AnimatedVertex vertex;

        vertex.position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        if (mesh->HasNormals())
        {
            vertex.normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }
        else
        {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (mesh->HasTextureCoords(0))
        {
            vertex.texCoord = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }
        else
        {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        result.vertices[i] = vertex;
    }

    extractBoneWeights(mesh, result.vertices);

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            result.indices.push_back(face.mIndices[j]);
    }

    return result;
}

void AnimatedModel::extractBoneWeights(aiMesh* mesh, std::vector<AnimatedVertex>& vertices)
{
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
    {
        string boneName = mesh->mBones[boneIndex]->mName.C_Str();

        int boneID;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            AnimatedBoneInfo info;
            info.id = boneCounter;
            info.offset = convertMatrixToGLM(mesh->mBones[boneIndex]->mOffsetMatrix);

            boneInfoMap[boneName] = info;
            boneID = boneCounter;
            boneCounter++;
        }
        else
        {
            boneID = boneInfoMap[boneName].id;
        }

        aiBone* bone = mesh->mBones[boneIndex];

        for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
        {
            int vertexID = bone->mWeights[weightIndex].mVertexId;
            float weight = bone->mWeights[weightIndex].mWeight;

            if (vertexID >= 0 && vertexID < (int)vertices.size())
                setVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }
}

void AnimatedModel::setVertexBoneData(AnimatedVertex& vertex, int boneID, float weight)
{
    for (int i = 0; i < 4; i++)
    {
        if (vertex.boneIDs[i] == -1)
        {
            vertex.boneIDs[i] = boneID;
            vertex.weights[i] = weight;
            return;
        }
    }
}

void AnimatedModel::setupMesh(AnimatedMesh& mesh)
{
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        mesh.vertices.size() * sizeof(AnimatedVertex),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        mesh.indices.size() * sizeof(unsigned int),
        mesh.indices.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(AnimatedVertex),
        (void*)offsetof(AnimatedVertex, position)
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(AnimatedVertex),
        (void*)offsetof(AnimatedVertex, normal)
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(AnimatedVertex),
        (void*)offsetof(AnimatedVertex, texCoord)
    );

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,
        4,
        GL_INT,
        GL_FALSE,
        sizeof(AnimatedVertex),
        (void*)offsetof(AnimatedVertex, boneIDs)
    );

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(AnimatedVertex),
        (void*)offsetof(AnimatedVertex, weights)
    );

    glBindVertexArray(0);
}

GLuint AnimatedModel::loadTextureFromFile(const std::string& path)
{
    if (!filesystem::exists(path))
    {
        cout << "Textura nao encontrada: " << path << endl;
        return 0;
    }

    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        0
    );

    if (!data)
    {
        cout << "Erro ao carregar textura: " << path << endl;
        return 0;
    }

    GLenum format = GL_RGB;

    if (channels == 4)
        format = GL_RGBA;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 1)
        format = GL_RED;

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);

    return id;
}

GLuint AnimatedModel::chooseTextureByMaterialName(const std::string& materialName)
{
    string base = "./assets/player/student/textures/";

    if (materialName.find("Hair") != string::npos)
        return loadTextureFromFile(base + "Hair.jpg");

    if (materialName.find("Skin") != string::npos)
        return loadTextureFromFile(base + "Wolf3D_Skin_baseColor.jpeg");

    if (materialName.find("Body") != string::npos)
        return loadTextureFromFile(base + "Wolf3D_Skin_baseColor.jpeg");

    if (materialName.find("Top") != string::npos)
        return loadTextureFromFile(base + "Wolf3D_Outfit_Bottom_baseColor.jpeg");

    if (materialName.find("Bottom") != string::npos)
        return loadTextureFromFile(base + "Wolf3D_Outfit_Bottom_baseColor.jpeg");

    if (materialName.find("Footwear") != string::npos ||
        materialName.find("Shoes") != string::npos)
        return loadTextureFromFile(base + "Wolf3D_Outfit_Footwear_baseColor.jpeg");

    return 0;
}

glm::mat4 AnimatedModel::convertMatrixToGLM(const aiMatrix4x4& from)
{
    glm::mat4 to;

    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;

    return to;
}

void AnimatedModel::draw()
{
    for (auto& mesh : meshes)
    {
        glColor3f(1.0f, 1.0f, 1.0f);

        if (mesh.hasTexture)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, mesh.textureID);
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
        }

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void AnimatedModel::draw(const std::vector<glm::mat4>& boneMatrices)
{
    tryLoadAnimatedShader();

    if (!animatedShader.isLoaded())
    {
        draw();
        return;
    }

    glDisable(GL_LIGHTING);

    animatedShader.use();

    for (int i = 0; i < (int)boneMatrices.size() && i < 100; i++)
    {
        string name = "bones[" + to_string(i) + "]";
        GLint location = glGetUniformLocation(animatedShader.id(), name.c_str());

        if (location != -1)
        {
            glUniformMatrix4fv(
                location,
                1,
                GL_FALSE,
                glm::value_ptr(boneMatrices[i])
            );
        }
    }

    for (auto& mesh : meshes)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh.hasTexture ? mesh.textureID : 0);

    glUniform1i(
        glGetUniformLocation(animatedShader.id(), "diffuseMap"),
        0
    );

    glUniform1i(
        glGetUniformLocation(animatedShader.id(), "hasTexture"),
        mesh.hasTexture ? 1 : 0
    );

    glBindVertexArray(mesh.VAO);

    glDrawElements(
        GL_TRIANGLES,
        mesh.indices.size(),
        GL_UNSIGNED_INT,
        0
    );

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

    Shader::stop();

    glEnable(GL_LIGHTING);
}

std::map<std::string, AnimatedBoneInfo>& AnimatedModel::getBoneInfoMap()
{
    return boneInfoMap;
}

int& AnimatedModel::getBoneCount()
{
    return boneCounter;
}