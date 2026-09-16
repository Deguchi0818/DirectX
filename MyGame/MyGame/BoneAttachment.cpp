#include "BoneAttachment.h"
#include "GameObject.h"
#include "MyMatrix4x4.h"

void BoneAttachment::Attach(GameObject* target, int boneIndex)
{
    m_target = target;
    m_boneIndex = boneIndex;

    if (m_target != nullptr)
    {
        // 取り付けた対象の親をボーンノードにする
        m_target->transform.SetParent(&m_boneTransform);
    }
}

void BoneAttachment::Update(const MyMatrix4x4& ownerWorld,
    const std::vector<DirectX::XMMATRIX>& boneWorlds)
{
    if (!IsValid()) return;
    if (m_boneIndex >= (int)boneWorlds.size()) return;

    // ボーンのワールド行列 = モデル空間のボーン行列 * モデル自身のワールド行列
    DirectX::XMMATRIX owner = ToXM(ownerWorld);
    DirectX::XMMATRIX boneWorld = boneWorlds[m_boneIndex] * owner;

    // ボーンノードに流し込む
    m_boneTransform.SetWorldMatrixDirect(FromXM(boneWorld));

    // 親が確定したので、対象の行列を作る (world = 対象のlocal * ボーンのworld)
    m_target->transform.UpdateMatrix();
}