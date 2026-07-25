#include "Weapon.h"

void Weapon::Initialize(const WeaponData& data) {
    m_data = data;
    pModel = data.model; 
    m_isTrigger = true;

    m_originalOffsets.clear();

	auto& col = AddCollider("sword_base", ColliderType::Sphere, MyVector3(0.0f, 0.0f, 6.0f), MyVector3(1.0f, 1.0f, 1.0f), true);
	col.radius = 2.0f; 
    m_originalOffsets.push_back(DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 0.0f));
    auto& col2 = AddCollider("sword_mid", ColliderType::Sphere, MyVector3(0.0f, 0.0f, 10.0f), MyVector3(1.0f, 1.0f, 1.0f), true);
    col2.radius = 2.0f;
    m_originalOffsets.push_back(DirectX::XMVectorSet(0.0f, 0.0f, 10.0f, 0.0f));
    auto& col3 = AddCollider("sword_tip", ColliderType::Sphere, MyVector3(0.0f, 0.0f, 14.0f), MyVector3(1.0f, 1.0f, 1.0f), true);
    col3.radius = 2.0f;
    m_originalOffsets.push_back(DirectX::XMVectorSet(0.0f, 0.0f, 14.0f, 0.0f));

}

void Weapon::FollowToBone(const DirectX::XMMATRIX& handMatrix) {
    // --------------------------------------------------------
    // 手首ボーンの行列を分解し、武器の位置を同期
    // --------------------------------------------------------
    DirectX::XMVECTOR scale, rot, trans;                           
    DirectX::XMMatrixDecompose(&scale, &rot, &trans, handMatrix);

    DirectX::XMFLOAT3 handPos;
    DirectX::XMStoreFloat3(&handPos, trans);
    transform.SetPosition(handPos.x, handPos.y, handPos.z);

    // --------------------------------------------------------
    // 手首の回転と武器固有の持ち方を合成
    // --------------------------------------------------------
    DirectX::XMMATRIX handRotMat = DirectX::XMMatrixRotationQuaternion(rot);

    // 武器種(データ)ごとの「持ち方の角度」を手首の回転に合成する
    DirectX::XMMATRIX localRot = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(m_data.localRotation.x),
        DirectX::XMConvertToRadians(m_data.localRotation.y),
        DirectX::XMConvertToRadians(m_data.localRotation.z)
    );

    // --------------------------------------------------------
    // 刃に配置した各コライダーを武器の向きに合わせて更新
    // --------------------------------------------------------
    DirectX::XMMATRIX combinedRot = localRot * handRotMat;
    for (size_t i = 0; i < m_colliders.size(); ++i) {
        DirectX::XMVECTOR baseOffset = m_originalOffsets[i];

        // 武器の傾きに合わせて各コライダーのオフセットを回転させ、刃に沿った判定を維持する
        DirectX::XMVECTOR transformedOffset = DirectX::XMVector3Transform(baseOffset, combinedRot);

        DirectX::XMFLOAT3 c;
        DirectX::XMStoreFloat3(&c, transformedOffset);

        m_colliders[i].offset = MyVector3(c.x, c.y, c.z);
    }
}

void Weapon::OnTriggerEnter(GameObject* other) {
    if (m_isAttacking) 
    {

    }
}