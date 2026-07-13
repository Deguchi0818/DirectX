#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Weapon.h"


enum class PlayerState
{
	Idle = 0,
	Run = 1,
	Jump = 2
};

class Player : public GameObject
{
	Weapon* m_equippedWeapon = nullptr;
public:
	Player() = default;

	void Initialize(Model* model);

	void Update(float dt, float camYaw);

	virtual void OnCollisionEnter(std::string myCol, GameObject* other, std::string otherCol) override;
	bool IsGrounded() const { return m_isGrounded; }

	float& GetJumpPower() { return m_jumpPower; }
	float& GetMoveSpeed() { return m_moveSpeed; }
	bool m_hitHead = false;

	void EquipWeapon(Weapon* weapon, const std::string& boneName) {
		m_equippedWeapon = weapon;
		if (pModel) {
			m_handBoneIndex = pModel->GetBoneIndex(boneName);
		}
	}

	void DrawWeapon(ID3D11DeviceContext* context, Shader* shader, ID3D11Buffer* cb, const MyMatrix4x4& view, const MyMatrix4x4& proj, const std::vector<DirectX::XMMATRIX>& worldMatrices);

private:
	float m_moveSpeed = 5.0f;
	float m_turnSpeed = 10.0f;
	float m_jumpPower = 5.0f;
	float coyoteTimer = 0.0f;
	float coyoteTime = 0.2f;
	bool m_isGrounded = false;
	PlayerState m_state = PlayerState::Idle;
	std::map<PlayerState, std::string> m_stateAnimMap;
	//GameObject* m_equippedWeapon = nullptr;
	int m_handBoneIndex = -1;

};

