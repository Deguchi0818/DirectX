#pragma once
#include "GameObject.h"

struct WeaponData {
    std::string name;
    float damage;
    float attackSpeed;
    Model* model;
    std::string effect;
};

class Weapon : public GameObject {
private:
    WeaponData m_data;
    bool m_isAttacking = false;

public:
    void Initialize(const WeaponData& data);

    void SetAttack(bool active) { m_isAttacking = active; }
    bool IsAttacking() const { return m_isAttacking; }

    virtual void OnTriggerEnter(GameObject* other) override;
};

