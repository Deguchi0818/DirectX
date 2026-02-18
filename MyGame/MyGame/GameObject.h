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

    MyVector3 velocity = { 0.0f, 0.0f, 0.0f };
    virtual void OnCollisionEnter(std::string myColName, GameObject* other, std::string otherColName) {}
    virtual void OnTriggerEnter(GameObject* other) {}

    MyVector3 GetVelocity() const { return velocity; }
    void SetVelocity(const MyVector3& v) { velocity = v; }

    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* cb, const MyMatrix4x4& view, const MyMatrix4x4& proj) 
    {
        if (!pModel) return; // メッシュがない場合は何もしない

        // 行列の計算
        transform.UpdateMatrix();
        MyMatrix4x4 wvp = MyMatrix4x4::Multiply(transform.GetWorldMatrix(), view);
        wvp = MyMatrix4x4::Multiply(wvp, proj);

        // 定数バッファの更新
        ConstantBufferData cbData;
        MyMatrix4x4 finalMat = wvp.Transpose();
        memcpy(&cbData.wvp, &finalMat, sizeof(MyMatrix4x4));
        context->UpdateSubresource(cb, 0, nullptr, &cbData, 0, 0);

        // シェーダーに定数バッファをセット
        context->VSSetConstantBuffers(0, 1, &cb);

        pModel->Draw(context);
    }

    AABB GetAABB() const;

    std::vector<ColliderComponent> m_colliders;

    const std::vector<ColliderComponent>& GetColliders() const { return m_colliders; }

    void AddCollider(std::string name, ColliderType type, MyVector3 offset, MyVector3 scale, float radius = 0.0f, bool isTrigger = false)
    {
        m_colliders.push_back({ name, type, offset, scale, radius, isTrigger });
    }
};

