#include "prb_model.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "external/stb_image.h"

using namespace std;

const char* pbrVertexShader = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

uniform mat4 modelView;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 FragPos;
out mat3 TBN;

void main()
{
    vec3 N = normalize(mat3(modelView) * aNormal);
    vec3 T = normalize(mat3(modelView) * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    TBN = mat3(T, B, N);

    vec4 viewPos = modelView * vec4(aPos, 1.0);
    FragPos = viewPos.xyz;

    TexCoord = aTexCoord;

    gl_Position = projection * viewPos;
}
)";

const char* pbrFragmentShader = R"(
#version 330 core

in vec2 TexCoord;
in vec3 FragPos;
in mat3 TBN;

out vec4 FragColor;

uniform sampler2D baseColorMap;
uniform sampler2D normalMap;
uniform sampler2D metallicRoughnessMap;

uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;

    vec3 normalTex = texture(normalMap, TexCoord).rgb;
    normalTex = normalize(normalTex * 2.0 - 1.0);
    vec3 normal = normalize(TBN * normalTex);

    vec3 mr = texture(metallicRoughnessMap, TexCoord).rgb;
    float roughness = mr.g;
    float metallic = mr.b;

    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    float specStrength = mix(0.15, 0.8, metallic);
    float shininess = mix(64.0, 8.0, roughness);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient = albedo * 0.35;
    vec3 diffuse = albedo * diff * lightColor;
    vec3 specular = vec3(spec) * specStrength;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
)";

GLuint PBRModel::loadTexture(const string& path)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data)
    {
        cout << "Erro ao carregar textura PBR: " << path << endl;
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

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);

    cout << "Textura PBR carregada: " << path << endl;

    return textureID;
}

PBRModel::PBRModel(
    const string& modelPath,
    const string& baseColorPath,
    const string& normalPath,
    const string& metallicRoughnessPath
)
{
    shader = new Shader(pbrVertexShader, pbrFragmentShader);

    baseColorTexture = loadTexture(baseColorPath);
    normalTexture = loadTexture(normalPath);
    metallicRoughnessTexture = loadTexture(metallicRoughnessPath);

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        modelPath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace
    );

    if (!scene || !scene->HasMeshes())
    {
        cout << "Erro ao carregar PBRModel: " << modelPath << endl;
        return;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                unsigned int index = face.mIndices[j];

                aiVector3D pos = mesh->mVertices[index];

                aiVector3D normal(0, 1, 0);
                if (mesh->HasNormals())
                    normal = mesh->mNormals[index];

                aiVector3D tex(0, 0, 0);
                if (mesh->HasTextureCoords(0))
                    tex = mesh->mTextureCoords[0][index];

                aiVector3D tangent(1, 0, 0);
                if (mesh->HasTangentsAndBitangents())
                    tangent = mesh->mTangents[index];

                PBRVertex v;

                v.x = pos.x;
                v.y = pos.y;
                v.z = pos.z;

                v.nx = normal.x;
                v.ny = normal.y;
                v.nz = normal.z;

                v.u = tex.x;
                v.v = tex.y;

                v.tx = tangent.x;
                v.ty = tangent.y;
                v.tz = tangent.z;

                vertices.push_back(v);
            }
        }
    }

    setupMesh();

    cout << "PBRModel carregado: " << modelPath << endl;
}

void PBRModel::setupMesh()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(PBRVertex),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PBRVertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PBRVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PBRVertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PBRVertex), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

void PBRModel::draw()
{
    if (!shader)
        return;

    GLfloat modelView[16];
    GLfloat projection[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);

    shader->use();

    shader->setMat4("modelView", modelView);
    shader->setMat4("projection", projection);

    shader->setVec3("lightPos", 0.0f, 20.0f, 10.0f);
    shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseColorTexture);
    shader->setInt("baseColorMap", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    shader->setInt("normalMap", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallicRoughnessTexture);
    shader->setInt("metallicRoughnessMap", 2);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);

    glUseProgram(0);

    glActiveTexture(GL_TEXTURE0);
}