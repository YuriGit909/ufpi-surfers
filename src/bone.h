#ifndef BONE_H
#define BONE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        m_NumPositions = channel->mNumPositionKeys;

        for (int i = 0; i < m_NumPositions; i++)
        {
            aiVector3D aiPosition = channel->mPositionKeys[i].mValue;
            float timeStamp = channel->mPositionKeys[i].mTime;

            KeyPosition data;
            data.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
            data.timeStamp = timeStamp;

            m_Positions.push_back(data);
        }

        m_NumRotations = channel->mNumRotationKeys;

        for (int i = 0; i < m_NumRotations; i++)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[i].mValue;
            float timeStamp = channel->mRotationKeys[i].mTime;

            KeyRotation data;
            data.orientation = glm::quat(
                aiOrientation.w,
                aiOrientation.x,
                aiOrientation.y,
                aiOrientation.z
            );
            data.timeStamp = timeStamp;

            m_Rotations.push_back(data);
        }

        m_NumScalings = channel->mNumScalingKeys;

        for (int i = 0; i < m_NumScalings; i++)
        {
            aiVector3D scale = channel->mScalingKeys[i].mValue;
            float timeStamp = channel->mScalingKeys[i].mTime;

            KeyScale data;
            data.scale = glm::vec3(scale.x, scale.y, scale.z);
            data.timeStamp = timeStamp;

            m_Scales.push_back(data);
        }
    }

    void update(float animationTime)
    {
        glm::mat4 translation = interpolatePosition(animationTime);
        glm::mat4 rotation = interpolateRotation(animationTime);
        glm::mat4 scale = interpolateScaling(animationTime);

        m_LocalTransform = translation * rotation * scale;
    }

    glm::mat4 getLocalTransform()
    {
        return m_LocalTransform;
    }

    std::string getBoneName() const
    {
        return m_Name;
    }

    int getBoneID()
    {
        return m_ID;
    }

private:
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;

        if (framesDiff == 0.0f)
            return 0.0f;

        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    int getPositionIndex(float animationTime)
    {
        for (int index = 0; index < m_NumPositions - 1; index++)
        {
            if (animationTime < m_Positions[index + 1].timeStamp)
                return index;
        }

        return 0;
    }

    int getRotationIndex(float animationTime)
    {
        for (int index = 0; index < m_NumRotations - 1; index++)
        {
            if (animationTime < m_Rotations[index + 1].timeStamp)
                return index;
        }

        return 0;
    }

    int getScaleIndex(float animationTime)
    {
        for (int index = 0; index < m_NumScalings - 1; index++)
        {
            if (animationTime < m_Scales[index + 1].timeStamp)
                return index;
        }

        return 0;
    }

    glm::mat4 interpolatePosition(float animationTime)
    {
        if (m_NumPositions == 1)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int p0Index = getPositionIndex(animationTime);
        int p1Index = p0Index + 1;

        float scaleFactor = getScaleFactor(
            m_Positions[p0Index].timeStamp,
            m_Positions[p1Index].timeStamp,
            animationTime
        );

        glm::vec3 finalPosition = glm::mix(
            m_Positions[p0Index].position,
            m_Positions[p1Index].position,
            scaleFactor
        );

        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    glm::mat4 interpolateRotation(float animationTime)
    {
        if (m_NumRotations == 1)
        {
            glm::quat rotation = glm::normalize(m_Rotations[0].orientation);
            return glm::toMat4(rotation);
        }

        int p0Index = getRotationIndex(animationTime);
        int p1Index = p0Index + 1;

        float scaleFactor = getScaleFactor(
            m_Rotations[p0Index].timeStamp,
            m_Rotations[p1Index].timeStamp,
            animationTime
        );

        glm::quat finalRotation = glm::slerp(
            m_Rotations[p0Index].orientation,
            m_Rotations[p1Index].orientation,
            scaleFactor
        );

        finalRotation = glm::normalize(finalRotation);

        return glm::toMat4(finalRotation);
    }

    glm::mat4 interpolateScaling(float animationTime)
    {
        if (m_NumScalings == 1)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int p0Index = getScaleIndex(animationTime);
        int p1Index = p0Index + 1;

        float scaleFactor = getScaleFactor(
            m_Scales[p0Index].timeStamp,
            m_Scales[p1Index].timeStamp,
            animationTime
        );

        glm::vec3 finalScale = glm::mix(
            m_Scales[p0Index].scale,
            m_Scales[p1Index].scale,
            scaleFactor
        );

        return glm::scale(glm::mat4(1.0f), finalScale);
    }

private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;

    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;

    std::string m_Name;
    int m_ID;
};

#endif