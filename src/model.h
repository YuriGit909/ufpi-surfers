#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

struct Vertex {
    float x, y, z;
};

struct MeshData {
    std::vector<Vertex> vertices;
    float r, g, b;
};

class Model {
public:
    Model(const std::string& path);
    void draw();

private:
    std::vector<MeshData> meshes;
};

#endif