#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <vector>
#include <glm/glm.hpp>

#include "animation.h"

class Animator
{
public:
    Animator(Animation* animation)
    {
        m_CurrentTime = 0.0f;
        m_CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(100);

        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    bool animationFinished()
{
    if (!m_CurrentAnimation)
        return true;

    return !m_Loop && m_CurrentTime >= m_CurrentAnimation->getDuration();
}

    void playAnimation(Animation* animation, bool loop = true)
{
    if (!animation)
        return;

    m_CurrentAnimation = animation;
    m_CurrentTime = 0.0f;
    m_Loop = loop;
}
    void setSpeed(float speed)
    {
        m_Speed = speed;
    }
    
    void updateAnimation(float dt)
    {
        if (!m_CurrentAnimation)
            return;

        float ticksPerSecond = m_CurrentAnimation->getTicksPerSecond();

        if (ticksPerSecond == 0.0f)
            ticksPerSecond = 25.0f;

        m_CurrentTime += ticksPerSecond * dt * m_Speed;

        if (m_Loop)
{
    m_CurrentTime = fmod(
        m_CurrentTime,
        m_CurrentAnimation->getDuration()
    );
}
else
{
    if (m_CurrentTime > m_CurrentAnimation->getDuration())
        m_CurrentTime = m_CurrentAnimation->getDuration();
}

        calculateBoneTransform(
            &m_CurrentAnimation->getRootNode(),
            glm::mat4(1.0f)
        );
    }

    std::vector<glm::mat4> getFinalBoneMatrices()
    {
        return m_FinalBoneMatrices;
    }

private:
    void calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* bone = m_CurrentAnimation->findBone(nodeName);

        if (bone)
        {
            bone->update(m_CurrentTime);
            nodeTransform = bone->getLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();

        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;

            if (index < 100)
                m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
        {
            calculateBoneTransform(
                &node->children[i],
                globalTransformation
            );
        }
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    bool m_Loop = true;

    Animation* m_CurrentAnimation = nullptr;
    float m_CurrentTime = 0.0f;
    float m_Speed = 1.0f;
};

#endif