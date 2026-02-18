#pragma once
#include "GameObject.h"
#include "Camera.h"

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

private:
	float m_moveSpeed = 5.0f;
	float m_turnSpeed = 10.0f;
	float m_jumpPower = 5.0f;
	float coyoteTimer = 0.0f;
	float coyoteTime = 0.2f;
	bool m_isGrounded = false;
};

