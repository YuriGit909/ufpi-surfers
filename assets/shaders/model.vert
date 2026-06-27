#version 120

attribute vec3 tangent;

varying vec3 fragPos;
varying vec3 fragNormal;
varying vec2 texCoord;
varying mat3 TBN;

void main()
{
    vec4 worldPos = gl_ModelViewMatrix * gl_Vertex;
    fragPos = worldPos.xyz;

    vec3 N = normalize(gl_NormalMatrix * gl_Normal);
    vec3 T = normalize(gl_NormalMatrix * tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    fragNormal = N;
    TBN = mat3(T, B, N);
    texCoord = gl_MultiTexCoord0.st;

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
