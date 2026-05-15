#pragma once
#include "Transform.h"
#include "Common.h"
#include "Mesh.h"
#include "Model.h"
#include "Collider.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

class GameObject
{
public:
	Transform transform;
    Model* pModel;

    bool m_isTrigger = false;
    bool isStatic = false;

    float m_mass = 1.0f;          // 質量 
    float m_restitution = 0.0f;   // 反発係数 (0.0: 跳ねない ～ 1.0: 完全に跳ねる)
    float m_friction = 0.5f;      // 摩擦係数 (0.0: ツルツル ～ 1.0: ザラザラ)
    bool m_useGravity = true;     // 重力の影響を受けるか

    bool m_showCollider = false;

    MyVector3 velocity = { 0.0f, 0.0f, 0.0f };
    virtual void OnCollisionEnter(std::string myColName, GameObject* other, std::string otherColName) {}
    virtual void OnTriggerEnter(GameObject* other) {}

    MyVector3 GetVelocity() const { return velocity; }
    void SetVelocity(const MyVector3& v) { velocity = v; }

    void Draw(ID3D11DeviceContext* context, Shader* shader, ID3D11Buffer* cb, const MyMatrix4x4& view, const MyMatrix4x4& proj)
    {
        if (!pModel) return; // メッシュがない場合は何もしない

        // 行列の計算
        transform.UpdateMatrix();
        MyMatrix4x4 worldMat = transform.GetWorldMatrix();
        MyMatrix4x4 viewMat = view;
        MyMatrix4x4 projMat = proj;

        DirectX::XMMATRIX world = *(DirectX::XMMATRIX*)&worldMat;
        DirectX::XMMATRIX xmView = *(DirectX::XMMATRIX*)&viewMat;
        DirectX::XMMATRIX xmProj = *(DirectX::XMMATRIX*)&projMat;

        DirectX::XMMATRIX wvp = world * xmView * xmProj;
        // 定数バッファの更新
        ConstantBufferData cbData;
        cbData.wvp = wvp;

        D3D11_MAPPED_SUBRESOURCE ms;
        if (SUCCEEDED(context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
        {
            memcpy(ms.pData, &cbData, sizeof(cbData));
            context->Unmap(cb, 0);
        }

        ID3D11Buffer* pBuffer = cb;
        // シェーダーに定数バッファをセット
        context->VSSetConstantBuffers(0, 1, &pBuffer);

        pModel->Draw(context, shader);
    }

    AABB GetAABB() const;

    std::vector<ColliderComponent> m_colliders;

    const std::vector<ColliderComponent>& GetColliders() const { return m_colliders; }

    ColliderComponent& AddCollider(std::string name, ColliderType type, MyVector3 offset, MyVector3 scale, bool isTrigger = false)
    {
        ColliderComponent col;
        col.name = name;
        col.type = type;
        col.offset = offset;
        col.scale = scale;
        col.isTrigger = isTrigger;

        m_colliders.push_back(col);

        return m_colliders.back();
    }
};

