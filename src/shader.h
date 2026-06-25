#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h>
#include <GL/glut.h>

class Shader
{
public:
    GLuint id;

    Shader(const char* vertexSource, const char* fragmentSource);
    void setMat4(const std::string& name, const float* value);

    void use();

    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec3(const std::string& name, float x, float y, float z);
};

#endif