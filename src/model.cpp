#include <GL/glut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

Model::Model(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals
    );

    if (!scene || !scene->HasMeshes()) {
        std::cout << "Erro ao carregar modelo: " << path << std::endl;
        return;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        MeshData data;

        if (scene->HasMaterials()) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            aiColor3D color;
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
                data.r = color.r;
                data.g = color.g;
                data.b = color.b;
            }

            float opacity = 1.0f;
            if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
                data.opacity = opacity;
            }
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                unsigned int index = face.mIndices[j];

                aiVector3D pos = mesh->mVertices[index];

                float nx = 0.0f;
                float ny = 1.0f;
                float nz = 0.0f;

                if (mesh->HasNormals()) {
                    aiVector3D normal = mesh->mNormals[index];
                    nx = normal.x;
                    ny = normal.y;
                    nz = normal.z;
                }

                float u = 0.0f;
                float v = 0.0f;

                if (mesh->HasTextureCoords(0)) {
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

    std::cout << "Modelo carregado com materiais, normais e opacidade!" << std::endl;
}

void Model::draw() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& mesh : meshes) {
        glColor4f(mesh.r, mesh.g, mesh.b, mesh.opacity);

        glDisable(GL_TEXTURE_2D);

        glBegin(GL_TRIANGLES);

        for (auto& v : mesh.vertices) {
            glNormal3f(v.nx, v.ny, v.nz);
            glTexCoord2f(v.u, v.v);
            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }

    glDisable(GL_BLEND);
}