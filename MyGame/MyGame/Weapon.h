#pragma once
#include "GameObject.h"

struct WeaponData {
    std::string name;
    float damage;
    float attackSpeed;
    Model* model;       // その武器の見た目
    std::string effect; // その武器特有のパーティクル名など
};

class Weapon : public GameObject {
private:
    WeaponData m_data;
    bool m_isAttacking = false; // 今、攻撃中かどうか

public:
    // 武器を初期化する
    void Initialize(const WeaponData& data);

    // 攻撃状態を切り替える
    void SetAttack(bool active) { m_isAttacking = active; }
    bool IsAttacking() const { return m_isAttacking; }

    // GameObjectのOnTriggerEnterをオーバーライドして攻撃判定を行う
    virtual void OnTriggerEnter(GameObject* other) override;
};

