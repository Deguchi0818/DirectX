#pragma once
#include <d3d11.h>
#include <map>
#include <string>
#include "Model.h"

class ResourceManager {
public:
    // モデルをロードして管理下に置く
    bool LoadModel(ID3D11Device* device, const std::string& name, const std::string& filePath);

    // 名前でモデルを取得
    Model* GetModel(const std::string& name);

    // モデルをプログラムで生成して登録する
    void CreateCube(ID3D11Device* device, const std::string& name, float size, DirectX::XMFLOAT4 color);
    void CreatePlane(ID3D11Device* device, const std::string& name, float width, float height, DirectX::XMFLOAT4 color);

private:
    std::map<std::string, Model> m_models; // 名前とモデルをセットで管理
};

