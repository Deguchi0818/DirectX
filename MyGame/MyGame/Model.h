#pragma once
#include "Common.h"
#include "Mesh.h"
#include <d3d11.h>
#include <vector>
#include <string>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Shader;

struct BoneInfo {
    std::string name;
    int parentIndex = -1;
    DirectX::XMMATRIX offset;
};

// 1つのキーフレーム（位置、回転、スケールのいずれか）
template <typename T>
struct AnimKeyFrame {
    float time;
    T value;
};

struct BoneAnimation {
    std::string boneName;
    std::vector<AnimKeyFrame<MyVector3>> positionKeys;
    std::vector<AnimKeyFrame<DirectX::XMVECTOR>> rotationKeys; // クォータニオン
    std::vector<AnimKeyFrame<MyVector3>> scaleKeys;
};

struct AnimationClip {
    std::string name;
    float duration;            // アニメーションの長さ（ティック数）
    float ticksPerSecond;      // 1秒あたりのティック数（速度）
    std::map<std::string, BoneAnimation> channels; // ボーン名で検索できる辞書
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

    // アニメーションデータを保存する配列
    std::vector<AnimationClip> m_animations;
private:
    Mesh m_mesh;

    std::vector<Subset> m_subsets;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_toonTexture;

    void ExtractAnimations(const aiScene* scene);
};