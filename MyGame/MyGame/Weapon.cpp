#include "Weapon.h"

void Weapon::Initialize(const WeaponData& data) {
    m_data = data;
    pModel = data.model; 
    m_isTrigger = true;
}

void Weapon::OnTriggerEnter(GameObject* other) {
    if (m_isAttacking) 
    {

    }
}