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
    DirectX::XMVECTOR scale, rot, trans;
    DirectX::XMMatrixDecompose(&scale, &rot, &trans, handMatrix);
    DirectX::XMFLOAT3 handPos;
    DirectX::XMStoreFloat3(&handPos, trans);
    transform.SetPosition(handPos.x, handPos.y, handPos.z);

    DirectX::XMMATRIX handRotMat = DirectX::XMMatrixRotationQuaternion(rot);
    DirectX::XMMATRIX localRot = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(m_data.localRotation.x),
        DirectX::XMConvertToRadians(m_data.localRotation.y),
        DirectX::XMConvertToRadians(m_data.localRotation.z)
    );
    DirectX::XMMATRIX combinedRot = localRot * handRotMat;

    for (size_t i = 0; i < m_colliders.size(); ++i) {
        DirectX::XMVECTOR baseOffset = m_originalOffsets[i];

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