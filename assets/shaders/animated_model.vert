#version 330 compatibility

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aBoneIDs;
layout(location = 4) in vec4 aWeights;

uniform mat4 bones[100];

out vec3 fragPos;
out vec3 fragNormal;
out vec2 texCoord;

void main()
{
    mat4 skinMatrix =
        aWeights.x * bones[int(aBoneIDs.x)] +
        aWeights.y * bones[int(aBoneIDs.y)] +
        aWeights.z * bones[int(aBoneIDs.z)] +
        aWeights.w * bones[int(aBoneIDs.w)];

    float totalWeight = aWeights.x + aWeights.y + aWeights.z + aWeights.w;

    if (totalWeight <= 0.0)
        skinMatrix = mat4(1.0);

    vec4 skinnedPosition = skinMatrix * vec4(aPos, 1.0);
    vec3 skinnedNormal = mat3(skinMatrix) * aNormal;

    gl_Position = gl_ModelViewProjectionMatrix * skinnedPosition;

    fragPos = vec3(gl_ModelViewMatrix * skinnedPosition);
    fragNormal = normalize(gl_NormalMatrix * skinnedNormal);
    texCoord = aTexCoord;
}