#pragma once
#include "Transform.h"
#include "Common.h"
#include "Mesh.h"
#include "Model.h"
#include "Collider.h"
#include "MyMatrix4x4.h"
#include "BoneAttachment.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

class GameObject
{
protected:
    std::string m_currentAnim = "Idle";
    float m_animTimer = 0.0f;

public:
    // 継承したクラスを基底ポインタ経由で破棄できるようにする
    virtual ~GameObject() = default;

	Transform transform;
    Model* pModel = nullptr;

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

    void Draw(ID3D11DeviceContext* context, Shader* shader, ID3D11Buffer* cb,
        const MyMatrix4x4& view, const MyMatrix4x4& proj)
    {
        if (!pModel) return;

        DirectX::XMMATRIX world  = ToXM(transform.GetWorldMatrix());
        DirectX::XMMATRIX xmView = ToXM(view);
        DirectX::XMMATRIX xmProj = ToXM(proj);

        DirectX::XMMATRIX wvp = world * xmView * xmProj;

        ConstantBufferData cbData;
        cbData.wvp = wvp;

        D3D11_MAPPED_SUBRESOURCE ms;
        if (SUCCEEDED(context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
        {
            memcpy(ms.pData, &cbData, sizeof(cbData));
            context->Unmap(cb, 0);
        }

        ID3D11Buffer* pBuffer = cb;
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

    void PlayAnimation(const std::string& animName, float startTime = 0.0f) {
        // 今と違うアニメーションを指示された時だけ、名前を書き換えてタイマーを0に戻す
        if (m_currentAnim != animName) {
            m_currentAnim = animName;
            m_animTimer = startTime;
        }
    }

    std::string GetCurrentAnimName() const { return m_currentAnim; }
    float GetAnimTimer() const { return m_animTimer; }

    // タイマーを進める関数
    void UpdateAnimTimer(float dt) { m_animTimer += dt; }


    // --------------------------------------------------------
    // ボーンへの装着
    //   自分のモデルのボーンに、別のオブジェクトを取り付ける。
    //   武器・盾・帽子など、すべて同じ仕組みで扱える。
    //   ボーンを持たないオブジェクトに対しては何も起きない。
    // --------------------------------------------------------
    void AttachToBone(GameObject* target, const std::string& boneName)
    {
        if (!pModel) return;          // モデルが無い = ボーンも無い
        if (!target) return;

        int index = pModel->GetBoneIndex(boneName);
        if (index < 0) return;        // そのボーンが存在しない

        auto attachment = std::make_unique<BoneAttachment>();
        attachment->Attach(target, index);
        m_attachments.push_back(std::move(attachment));
    }

    // 取り付けたものの姿勢を更新する(Update時に呼ぶ)
    void UpdateAttachments(const std::vector<DirectX::XMMATRIX>& boneWorlds)
    {
        for (auto& attachment : m_attachments)
        {
            attachment->Update(transform.GetWorldMatrix(), boneWorlds);
        }
    }

    // 取り付けたものを描画する(Render時に呼ぶ)
    void DrawAttachments(ID3D11DeviceContext* context, Shader* shader, ID3D11Buffer* cb,
        const MyMatrix4x4& view, const MyMatrix4x4& proj)
    {
        for (auto& attachment : m_attachments)
        {
            GameObject* target = attachment->GetTarget();
            if (target) target->Draw(context, shader, cb, view, proj);
        }
    }

protected:
    // 取り付けたもの。BoneAttachment は内部に Transform を持ち、
    // そのアドレスを子の親として登録しているため、
    // 再配置でアドレスが変わらないよう unique_ptr で保持する。
    std::vector<std::unique_ptr<BoneAttachment>> m_attachments;
};

