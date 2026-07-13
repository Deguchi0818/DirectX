#include "Weapon.h"

void Weapon::Initialize(const WeaponData& data) {
    m_data = data;
    pModel = data.model; 
    m_isTrigger = true;
}

void Weapon::OnTriggerEnter(GameObject* other) {
    // 攻撃中かつ、敵（Enemyクラスなど）に当たった場合
    if (m_isAttacking) {
        // 例: other->TakeDamage(m_data.damage); 
        // 攻撃が当たった時の演出などをここに追加できる
    }
}