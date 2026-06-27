#include <GL/glew.h>
#include <GL/glut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>

#include "model.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#ifndef aiTextureType_BASE_COLOR
#define aiTextureType_BASE_COLOR aiTextureType_DIFFUSE
#endif

#ifndef aiTextureType_NORMAL_CAMERA
#define aiTextureType_NORMAL_CAMERA aiTextureType_NORMALS
#endif

#ifndef aiTextureType_METALNESS
#define aiTextureType_METALNESS aiTextureType_UNKNOWN
#endif

#ifndef aiTextureType_DIFFUSE_ROUGHNESS
#define aiTextureType_DIFFUSE_ROUGHNESS aiTextureType_SHININESS
#endif

using namespace std;

static Shader modelShader;
static bool shaderTriedToLoad = false;

string getDirectory(const string &path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos == string::npos)
        return "";

    return path.substr(0, pos + 1);
}

string getBaseName(const string &path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos == string::npos)
        return path;

    return path.substr(pos + 1);
}

bool fileExists(const string &path)
{
    return filesystem::exists(path) && filesystem::is_regular_file(path);
}

string cleanTextureName(const string &texturePath)
{
    if (texturePath.find(' ') == string::npos)
        return texturePath;

    // Alguns MTL escrevem: map_Bump -bm 1.000000 arquivo.png.
    // Se o Assimp devolver a linha com opcoes, pegamos o ultimo token.
    string token;
    string lastToken;
    stringstream ss(texturePath);

    while (ss >> token)
        lastToken = token;

    return lastToken;
}

string resolveTexturePath(const string &modelDirectory, const string &texturePath)
{
    string cleanedPath = cleanTextureName(texturePath);

    if (cleanedPath.empty() || cleanedPath == ".")
        return "";

    filesystem::path tex(cleanedPath);

    if (tex.is_absolute() && fileExists(cleanedPath))
        return cleanedPath;

    string direct = (filesystem::path(modelDirectory) / tex).lexically_normal().string();
    if (fileExists(direct))
        return direct;

    string fallback = (filesystem::path("./assets/textures") / getBaseName(cleanedPath)).lexically_normal().string();
    if (fileExists(fallback))
        return fallback;

    return direct;
}

GLuint loadTexture(const string &filename)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (!data)
    {
        cout << "Erro ao carregar textura: " << filename << endl;
        return 0;
    }

    GLenum format = GL_RGB;
    GLint internalFormat = GL_RGB;

    if (channels == 1)
    {
        format = GL_RED;
        internalFormat = GL_RED;
    }
    else if (channels == 3)
    {
        format = GL_RGB;
        internalFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        format = GL_RGBA;
        internalFormat = GL_RGBA;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalFormat,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);

    cout << "Textura carregada: " << filename << endl;

    return textureID;
}

GLuint loadEmbeddedTexture(const aiScene *scene, const string &textureName)
{
    if (textureName.empty() || textureName[0] != '*')
        return 0;

    int index = stoi(textureName.substr(1));

    if (index < 0 || index >= (int)scene->mNumTextures)
    {
        cout << "Textura embedded invalida: " << textureName << endl;
        return 0;
    }

    const aiTexture *texture = scene->mTextures[index];

    int width, height, channels;
    unsigned char *data = nullptr;

    if (texture->mHeight == 0)
    {
        data = stbi_load_from_memory(
            reinterpret_cast<unsigned char *>(texture->pcData),
            texture->mWidth,
            &width,
            &height,
            &channels,
            0);
    }
    else
    {
        data = stbi_load_from_memory(
            reinterpret_cast<unsigned char *>(texture->pcData),
            texture->mWidth * texture->mHeight * 4,
            &width,
            &height,
            &channels,
            0);
    }

    if (!data)
    {
        cout << "Erro ao carregar textura embedded: " << textureName << endl;
        return 0;
    }

    GLenum format = GL_RGB;
    GLint internalFormat = GL_RGB;

    if (channels == 1)
    {
        format = GL_RED;
        internalFormat = GL_RED;
    }
    else if (channels == 3)
    {
        format = GL_RGB;
        internalFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        format = GL_RGBA;
        internalFormat = GL_RGBA;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalFormat,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);

    cout << "Textura embedded carregada: " << textureName << endl;

    return textureID;
}

void storeMaterialTexture(
    aiMaterial *material,
    aiTextureType type,
    const string &directory,
    string &textureFile,
    bool &hasTexture,
    const string &label)
{
    aiString texturePath;

    if (material->GetTexture(type, 0, &texturePath) != AI_SUCCESS)
        return;

    string texFile = texturePath.C_Str();

    if (!texFile.empty() && texFile[0] == '*')
    {
        textureFile = texFile;
        hasTexture = true;

        cout << "  " << label << " embedded: " << texFile << endl;
        return;
    }

    string fullPath = resolveTexturePath(directory, texFile);
    if (fullPath.empty())
        return;

    textureFile = fullPath;
    hasTexture = true;

    cout << "  " << label << ": " << fullPath << endl;
}

void ensureTextureLoaded(
    GLuint &textureID,
    bool &hasTexture,
    const string &textureFile,
    const aiScene *scene)
{
    if (!hasTexture || textureID != 0)
        return;

    if (!textureFile.empty() && textureFile[0] == '*')
        textureID = loadEmbeddedTexture(scene, textureFile);
    else
        textureID = loadTexture(textureFile);

    if (textureID == 0)
        hasTexture = false;
}

void tryLoadShader()
{
    if (shaderTriedToLoad)
        return;

    shaderTriedToLoad = true;

    if (!modelShader.load("./assets/shaders/model.vert", "./assets/shaders/model.frag"))
        cout << "Shader nao carregou. O modelo vai usar modo fixo do OpenGL." << endl;
}

Model::Model(const std::string &path)
{
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace);

    if (!scene || !scene->HasMeshes())
    {
        cout << "Erro ao carregar modelo: " << path << endl;
        return;
    }

    sceneRef = scene;

    string directory = getDirectory(path);

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh *mesh = scene->mMeshes[m];

        MeshData data;

        if (scene->HasMaterials())
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            aiString materialName;
            if (material->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS)
                data.materialName = materialName.C_Str();

            aiColor3D color;
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
            {
                data.r = color.r;
                data.g = color.g;
                data.b = color.b;
            }

            float opacity = 1.0f;
            if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
                data.opacity = opacity;

            cout << "Material " << mesh->mMaterialIndex << " (" << data.materialName << ")" << endl;



            for (int t = aiTextureType_NONE; t <= aiTextureType_UNKNOWN; t++)
            {
                unsigned int n = material->GetTextureCount((aiTextureType)t);

                if (n > 0)
                    cout << "Tipo " << t << " -> " << n << " textura(s)" << endl;
            }

            // GLB / glTF - PBR
            storeMaterialTexture(material, aiTextureType_BASE_COLOR, directory, data.diffusePath, data.hasDiffuse, "baseColor");
            storeMaterialTexture(material, aiTextureType_NORMAL_CAMERA, directory, data.normalPath, data.hasNormal, "normal");
            storeMaterialTexture(material, aiTextureType_METALNESS, directory, data.specularPath, data.hasSpecular, "metallic");
            storeMaterialTexture(material, aiTextureType_DIFFUSE_ROUGHNESS, directory, data.roughnessPath, data.hasRoughness, "roughness");

            // OBJ / MTL - compatibilidade antiga
            if (!data.hasDiffuse)
                storeMaterialTexture(material, aiTextureType_DIFFUSE, directory, data.diffusePath, data.hasDiffuse, "diffuse/map_Kd");

            if (!data.hasNormal)
                storeMaterialTexture(material, aiTextureType_HEIGHT, directory, data.normalPath, data.hasNormal, "normal/map_Bump");

            if (!data.hasNormal)
                storeMaterialTexture(material, aiTextureType_NORMALS, directory, data.normalPath, data.hasNormal, "normal");

            if (!data.hasSpecular)
                storeMaterialTexture(material, aiTextureType_SPECULAR, directory, data.specularPath, data.hasSpecular, "specular/map_Ks");

            
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                unsigned int index = face.mIndices[j];

                aiVector3D pos = mesh->mVertices[index];

                float nx = 0.0f;
                float ny = 1.0f;
                float nz = 0.0f;

                if (mesh->HasNormals())
                {
                    aiVector3D normal = mesh->mNormals[index];
                    nx = normal.x;
                    ny = normal.y;
                    nz = normal.z;
                }

                float u = 0.0f;
                float v = 0.0f;

                int uvChannel = 0;

                if (data.materialName.find("Bark") != string::npos ||
                    data.materialName.find("Leaf") != string::npos)
                {
                    if (mesh->HasTextureCoords(1))
                        uvChannel = 1;
                }

                if (mesh->HasTextureCoords(uvChannel))
                {
                    aiVector3D tex = mesh->mTextureCoords[uvChannel][index];
                    u = tex.x;
                    v = tex.y;
                }

                if (index < 5 &&
                    (data.materialName == "Bark.001" || data.materialName == "Leaf.001"))
                {
                    cout << data.materialName
                        << " UV = " << u << ", " << v << endl;
                }

                float tx = 1.0f;
                float ty = 0.0f;
                float tz = 0.0f;

                if (mesh->HasTangentsAndBitangents())
                {
                    aiVector3D tangent = mesh->mTangents[index];
                    tx = tangent.x;
                    ty = tangent.y;
                    tz = tangent.z;
                }

                data.vertices.push_back({pos.x, pos.y, pos.z,
                                         nx, ny, nz,
                                         u, v,
                                         tx, ty, tz});
            }
        }

        meshes.push_back(data);
    }

    cout << "Modelo carregado com materiais, texturas multiplas e tangentes: " << path << endl;
}

void Model::draw()
{
    tryLoadShader();

    bool usingShader = modelShader.isLoaded();

    if (usingShader)
    {
        glDisable(GL_LIGHTING);
        modelShader.use();
        modelShader.setInt("diffuseMap", 0);
        modelShader.setInt("normalMap", 1);
        modelShader.setInt("specularMap", 2);
        modelShader.setInt("roughnessMap", 3);
        modelShader.setVec3("lightPosition", 0.0f, 20.0f, 10.0f);
        modelShader.setVec3("lightColor", 0.9f, 0.9f, 0.9f);
        modelShader.setVec3("ambientColor", 0.45f, 0.45f, 0.45f);
    }

    GLint tangentLocation = -1;
    if (usingShader)
        tangentLocation = glGetAttribLocation(modelShader.id(), "tangent");

    for (auto &mesh : meshes)
    {
        ensureTextureLoaded(mesh.diffuseMap, mesh.hasDiffuse, mesh.diffusePath, sceneRef);
        ensureTextureLoaded(mesh.normalMap, mesh.hasNormal, mesh.normalPath, sceneRef);
        ensureTextureLoaded(mesh.specularMap, mesh.hasSpecular, mesh.specularPath, sceneRef);
        ensureTextureLoaded(mesh.roughnessMap, mesh.hasRoughness, mesh.roughnessPath, sceneRef);
        if (usingShader)
        {
            modelShader.setVec4("materialColor", mesh.r, mesh.g, mesh.b, mesh.opacity);
            modelShader.setInt("hasDiffuse", mesh.hasDiffuse ? 1 : 0);
            modelShader.setInt("hasNormal", mesh.hasNormal ? 1 : 0);
            modelShader.setInt("hasSpecular", mesh.hasSpecular ? 1 : 0);
            modelShader.setInt("hasRoughness", mesh.hasRoughness ? 1 : 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.hasDiffuse ? mesh.diffuseMap : 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mesh.hasNormal ? mesh.normalMap : 0);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, mesh.hasSpecular ? mesh.specularMap : 0);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, mesh.hasRoughness ? mesh.roughnessMap : 0);

            glActiveTexture(GL_TEXTURE0);
        }
        else
        {
            if (mesh.hasDiffuse)
            {
                cout << "Desenhando com textura: " << mesh.materialName
                     << " -> " << mesh.diffusePath
                     << " id=" << mesh.diffuseMap << endl;

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, mesh.diffuseMap);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                glColor4f(1.0f, 1.0f, 1.0f, mesh.opacity);
            }
            else
            {
                glDisable(GL_TEXTURE_2D);
                glColor4f(mesh.r, mesh.g, mesh.b, mesh.opacity);
            }
        }

        glBegin(GL_TRIANGLES);

        for (auto &v : mesh.vertices)
        {
            if (usingShader && tangentLocation != -1)
                glVertexAttrib3f(tangentLocation, v.tx, v.ty, v.tz);

            glNormal3f(v.nx, v.ny, v.nz);
            glTexCoord2f(v.u, v.v);
            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }

    if (usingShader)
    {
        Shader::stop();
        glEnable(GL_LIGHTING);
    }

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
