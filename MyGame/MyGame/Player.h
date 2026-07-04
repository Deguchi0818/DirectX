#pragma once
#include "GameObject.h"
#include "Camera.h"

enum class PlayerState
{
	Idle = 0,
	Run = 1,
	Jump = 2
};

class Player : public GameObject
{
public:
	Player() = default;

	void Initialize(Model* model);

	void Update(float dt, float camYaw);

	virtual void OnCollisionEnter(std::string myCol, GameObject* other, std::string otherCol) override;
	bool IsGrounded() const { return m_isGrounded; }

	float& GetJumpPower(){ return m_jumpPower; }
	float& GetMoveSpeed() { return m_moveSpeed; }
	std::string GetCurrentAnimName() const {
		return m_stateAnimMap.at(m_state);
	}
	bool m_hitHead = false;


private:
	float m_moveSpeed = 5.0f;
	float m_turnSpeed = 10.0f;
	float m_jumpPower = 5.0f;
	float coyoteTimer = 0.0f;
	float coyoteTime = 0.2f;
	bool m_isGrounded = false;
	PlayerState m_state = PlayerState::Idle;
	std::map<PlayerState, std::string> m_stateAnimMap;

};

