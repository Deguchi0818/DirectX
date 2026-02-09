#include "Model.h"
#include "GeometryGenerator.h"
#include <vector>

void Model::CreateCube(ID3D11Device* device, float size, const MyVector4& color) {
    std::vector<Vertex> v;
    std::vector<unsigned short> i;
    GeometryGenerator::CreateCube(size, color, v, i);
    m_mesh.Create(device, v.data(), (int)v.size(), i.data(), (int)i.size());
}

void Model::CreatePlane(ID3D11Device* device, float width, float depth, const MyVector4& color) {
    std::vector<Vertex> v;
    std::vector<unsigned short> i;
    GeometryGenerator::CreatePlane(width, depth, color, v, i);
    m_mesh.Create(device, v.data(), (int)v.size(), i.data(), (int)i.size());
}

void Model::Draw(ID3D11DeviceContext* context) {
    m_mesh.Draw(context);
}