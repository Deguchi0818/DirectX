#pragma once
#include "Mesh.h"
#include "Common.h"
#include <string>
#include <vector>

class Shader;

struct BoneInfo {
    std::string name;
    int parentIndex = -1;
    DirectX::XMMATRIX offset;
};

class Model {
public:
    struct Subset {
        unsigned int indexCount;
        unsigned int startIndex;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
    };

    // 立方体モデルを作成
    void CreateCube(ID3D11Device* device, float size, const MyVector4& color);

    // 平面モデルを作成
    void CreatePlane(ID3D11Device* device, float width, float depth, const MyVector4& color);

    bool LoadFromFile(ID3D11Device* device, const std::string& filename);

    void SetToonTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> toon) { m_toonTexture = toon; }

    // メッシュを描画する
    void Draw(ID3D11DeviceContext* context, Shader* shader);

    // ボーンのオフセット行列を保存する配列
    std::vector<BoneInfo> m_bones;
private:
    Mesh m_mesh;

    std::vector<Subset> m_subsets;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_toonTexture;
};