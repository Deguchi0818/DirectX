#pragma once
#include "GameObject.h"

struct WeaponData {
    std::string name;
    float damage;
    float attackSpeed;
    Model* model;
    std::string effect;

    DirectX::XMFLOAT3 localRotation = { 0.0f, 0.0f, 0.0f };
};

class Weapon : public GameObject {
private:
    WeaponData m_data;
    bool m_isAttacking = false;

    std::vector<DirectX::XMVECTOR> m_originalOffsets;

public:
    void Initialize(const WeaponData& data);

    void SetAttack(bool active) { m_isAttacking = active; }
    bool IsAttacking() const { return m_isAttacking; }

    virtual void OnTriggerEnter(GameObject* other) override;

    void FollowToBone(const DirectX::XMMATRIX& handMatrix);
};

