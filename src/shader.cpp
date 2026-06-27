#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Shader::Shader()
{
    programID = 0;
    loaded = false;
}

Shader::~Shader()
{
    if (programID != 0)
        glDeleteProgram(programID);
}

std::string Shader::readFile(const std::string& path) const
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cout << "Erro ao abrir shader: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Shader::checkShader(GLuint shader, const std::string& label) const
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_TRUE)
        return true;

    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    std::vector<char> log(logLength + 1);
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());

    std::cout << "Erro ao compilar " << label << ":\n" << log.data() << std::endl;
    return false;
}

bool Shader::checkProgram(GLuint program, const std::string& label) const
{
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_TRUE)
        return true;

    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

    std::vector<char> log(logLength + 1);
    glGetProgramInfoLog(program, logLength, nullptr, log.data());

    std::cout << "Erro ao linkar " << label << ":\n" << log.data() << std::endl;
    return false;
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty())
        return false;

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    if (!checkShader(vertexShader, vertexPath))
    {
        glDeleteShader(vertexShader);
        return false;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    if (!checkShader(fragmentShader, fragmentPath))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    loaded = checkProgram(programID, "shader program");

    if (!loaded)
    {
        glDeleteProgram(programID);
        programID = 0;
    }

    return loaded;
}

void Shader::use() const
{
    if (loaded)
        glUseProgram(programID);
}

void Shader::stop()
{
    glUseProgram(0);
}

GLuint Shader::id() const
{
    return programID;
}

bool Shader::isLoaded() const
{
    return loaded;
}

void Shader::setInt(const std::string& name, int value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location != -1)
        glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location != -1)
        glUniform1f(location, value);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location != -1)
        glUniform3f(location, x, y, z);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location != -1)
        glUniform4f(location, x, y, z, w);
}
