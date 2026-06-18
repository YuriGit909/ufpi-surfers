#include "model.h"
#include <GL/glut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

Model::Model(const std::string& path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate | aiProcess_FlipUVs
    );

    if (!scene || !scene->HasMeshes()) {
        std::cout << "Erro ao carregar modelo: " << path << std::endl;
        return;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        MeshData data;
        data.r = 0.8f;
        data.g = 0.8f;
        data.b = 0.8f;

        if (scene->HasMaterials()) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            aiColor3D color;
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
                data.r = color.r;
                data.g = color.g;
                data.b = color.b;
            }
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                unsigned int index = face.mIndices[j];
                aiVector3D v = mesh->mVertices[index];

                data.vertices.push_back({v.x, v.y, v.z});
            }
        }

        meshes.push_back(data);
    }

    std::cout << "Modelo carregado com materiais!" << std::endl;
}

void Model::draw() {
    for (auto& mesh : meshes) {
        glColor3f(mesh.r, mesh.g, mesh.b);

        glBegin(GL_TRIANGLES);
        for (auto& v : mesh.vertices) {
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
}