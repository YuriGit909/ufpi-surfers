#ifndef ANIMATION_H
#define ANIMATION_H

#include <iostream>
#include <map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "animated_model.h"
#include "bone.h"

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation(const std::string& animationPath, AnimatedModel* model, int animationIndex = 0)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            animationPath,
            aiProcess_Triangulate
        );

        if (!scene || !scene->mRootNode)
        {
            std::cout << "Erro ao carregar animacao: " << animationPath << std::endl;
            return;
        }

        if (!scene->HasAnimations())
        {
            std::cout << "Esse modelo nao tem animacao." << std::endl;
            return;
        }

        if (animationIndex < 0 || animationIndex >= (int)scene->mNumAnimations)
{
    std::cout << "Indice de animacao invalido: " << animationIndex << std::endl;
    return;
}

        aiAnimation* animation = scene->mAnimations[animationIndex];


        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;

        readHierarchyData(m_RootNode, scene->mRootNode);

        m_BoneInfoMap = model->getBoneInfoMap();
        m_BoneCounter = model->getBoneCount();

        readMissingBones(animation);
    }

    Bone* findBone(const std::string& name)
    {
        for (auto& bone : m_Bones)
        {
            if (bone.getBoneName() == name)
                return &bone;
        }

        return nullptr;
    }

    float getTicksPerSecond()
    {
        return m_TicksPerSecond;
    }

    float getDuration()
    {
        return m_Duration;
    }

    const AssimpNodeData& getRootNode()
    {
        return m_RootNode;
    }

    const std::map<std::string, AnimatedBoneInfo>& getBoneIDMap()
    {
        return m_BoneInfoMap;
    }

private:
    glm::mat4 convertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;

        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;

        return to;
    }

    void readMissingBones(const aiAnimation* animation)
    {
        int size = animation->mNumChannels;

        for (int i = 0; i < size; i++)
        {
            aiNodeAnim* channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                AnimatedBoneInfo info;
                info.id = m_BoneCounter;
                info.offset = glm::mat4(1.0f);

                m_BoneInfoMap[boneName] = info;
                m_BoneCounter++;
            }

            m_Bones.push_back(
                Bone(
                    boneName,
                    m_BoneInfoMap[boneName].id,
                    channel
                )
            );
        }
    }

    void readHierarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        dest.name = src->mName.data;
        dest.transformation = convertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            readHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

private:
    float m_Duration = 0.0f;
    float m_TicksPerSecond = 0.0f;

    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;

    std::map<std::string, AnimatedBoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;
};

#endif