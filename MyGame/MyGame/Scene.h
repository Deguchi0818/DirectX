#pragma once
#include <vector>
#include <memory>

#include <DirectXMath.h>
#include "GameObject.h"
#include "Player.h"
#include "Weapon.h"
#include "Coin.h"

class ResourceManager;
class PhysicsEngine;

class Scene
{
public:
    // シーンを構築し、物理エンジンに登録する
    void Create(ResourceManager& resources, PhysicsEngine& physics);

    void UpdateAnimation();
    void UpdateTransforms();

    void Draw(ID3D11DeviceContext* context, Shader* shader, ID3D11Buffer* cb,
        const MyMatrix4x4& view, const MyMatrix4x4& proj);

    const std::vector<DirectX::XMMATRIX>& GetSkinMatrices() const { return m_skinMatrices; }

    // 外から参照するための取得関数
    Player& GetPlayer() { return m_player; }
    Weapon& GetSword() { return m_sword; }
    GameObject* GetTarget() const { return m_target; }

    std::vector<std::unique_ptr<GameObject>>& GetGameObjects() { return m_gameObjects; }
    std::vector<std::unique_ptr<GameObject>>& GetTerrain() { return m_terrain; }

private:

    std::vector<DirectX::XMMATRIX> m_boneWorlds;    // ボーンのワールド行列
    std::vector<DirectX::XMMATRIX> m_skinMatrices;  // シェーダーへ送るスキニング行列

    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    std::vector<std::unique_ptr<GameObject>> m_terrain;

    Player m_player;
    Weapon m_sword;
    GameObject* m_target = nullptr;   // 所有しない。参照のみ
};