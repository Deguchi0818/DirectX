#include "ResourceManager.h"
bool ResourceManager::LoadModel(ID3D11Device* device, const std::string& name, const std::string& filePath) {
    Model newModel;
    if (!newModel.LoadFromFile(device, filePath.c_str(), name.c_str())) {
        return false;
    }
    m_models[name] = newModel;
    return true;
}

Model* ResourceManager::GetModel(const std::string& name) {
    if (m_models.find(name) != m_models.end()) {
        return &m_models[name];
    }
    return nullptr; // Œ©‚Â‚©‚ç‚È‚©‚Á‚½‚çnullptr
}

void ResourceManager::CreateCube(ID3D11Device* device, const std::string& name, float size, DirectX::XMFLOAT4 color) {
    Model cube;
    cube.CreateCube(device, size, MyVector4(color.x, color.y, color.z, color.w));
    m_models[name] = cube;
}

void ResourceManager::CreatePlane(ID3D11Device* device, const std::string& name, float width, float height, DirectX::XMFLOAT4 color) {
    Model plane;
    plane.CreatePlane(device, width, height, MyVector4(color.x, color.y, color.z, color.w));
    m_models[name] = plane;
}