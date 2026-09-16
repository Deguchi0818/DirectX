#include "Weapon.h"

void Weapon::Initialize(const WeaponData& data) {
    m_data = data;
    pModel = data.model; 
    m_isTrigger = true;


	auto& col = AddCollider("sword_base", ColliderType::Sphere, MyVector3(0.0f, 0.0f, -600.0f), MyVector3(1.0f, 1.0f, 1.0f), true);
	col.radius = 200.0f; 
    auto& col2 = AddCollider("sword_mid", ColliderType::Sphere, MyVector3(0.0f, 0.0f, -1000.0f), MyVector3(1.0f, 1.0f, 1.0f), true);
    col2.radius = 200.0f;
    auto& col3 = AddCollider("sword_tip", ColliderType::Sphere, MyVector3(0.0f, 0.0f, -1400.0f), MyVector3(1.0f, 1.0f, 1.0f), true);
    col3.radius = 200.0f;

}


void Weapon::OnTriggerEnter(GameObject* other) {
    if (m_isAttacking) 
    {

    }
}