#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h>

class Shader {
public:
    Shader();
    ~Shader();

    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;
    static void stop();

    GLuint id() const;
    bool isLoaded() const;

    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;

private:
    GLuint programID;
    bool loaded;

    std::string readFile(const std::string& path) const;
    bool checkShader(GLuint shader, const std::string& label) const;
    bool checkProgram(GLuint program, const std::string& label) const;
};

#endif
