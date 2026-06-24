#include <GL/glut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <filesystem>
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

using namespace std;

string getDirectory(const string& path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos == string::npos)
        return "";

    return path.substr(0, pos + 1);
}

GLuint loadTexture(const string& filename)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (!data)
    {
        cout << "Erro ao carregar textura: " << filename << endl;
        return 0;
    }

    GLenum format = GL_RGB;

    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);

    cout << "Textura carregada: " << filename << endl;

    return textureID;
}

Model::Model(const std::string& path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals
    );

    if (!scene || !scene->HasMeshes())
    {
        cout << "Erro ao carregar modelo: " << path << endl;
        return;
    }

    string directory = getDirectory(path);

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh* mesh = scene->mMeshes[m];

        MeshData data;

        if (scene->HasMaterials())
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            aiColor3D color;
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
            {
                data.r = color.r;
                data.g = color.g;
                data.b = color.b;
            }

            float opacity = 1.0f;
            if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
            {
                data.opacity = opacity;
            }

            aiString texturePath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
            {
                string texFile = texturePath.C_Str();

                string fullPath = directory + texFile;

                data.textureID = loadTexture(fullPath);

                if (data.textureID != 0)
                {
                    data.hasTexture = true;
                }
            printf(
            "Material %d -> textura %u\n",
            mesh->mMaterialIndex,
            data.textureID
            );
            }
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

                if (mesh->HasTextureCoords(0))
                {
                    aiVector3D tex = mesh->mTextureCoords[0][index];
                    u = tex.x;
                    v = tex.y;
                }

                data.vertices.push_back({
                    pos.x, pos.y, pos.z,
                    nx, ny, nz,
                    u, v
                });
            }
        }

        meshes.push_back(data);
    }

    cout << "Modelo carregado com materiais e texturas!" << endl;
}

void Model::draw()
{
    for (auto &mesh : meshes)
    {
        if (mesh.hasTexture)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, mesh.textureID);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glColor4f(1.0f, 1.0f, 1.0f, mesh.opacity);
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
            glColor4f(mesh.r, mesh.g, mesh.b, mesh.opacity);
        }

        glBegin(GL_TRIANGLES);

        for (auto &v : mesh.vertices)
        {
            glNormal3f(v.nx, v.ny, v.nz);
            glTexCoord2f(v.u, v.v);
            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
}