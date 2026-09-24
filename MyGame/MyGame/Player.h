#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Weapon.h"
#include "PlayerState.h"


class Player : public GameObject
{
public:
	Player() = default;

	void Initialize(Model* model);

	void Update(float dt, float camYaw);

	virtual void OnCollisionEnter(std::string myCol, GameObject* other, std::string otherCol) override;
	bool IsGrounded() const { return m_isGrounded; }

	float& GetJumpPower() { return m_jumpPower; }
	float& GetMoveSpeed() { return m_moveSpeed; }
	float GetMoveDirX() const { return m_moveDirX; }
	float GetMoveDirZ() const { return m_moveDirZ; }
	bool m_hitHead = false;

	void ChangeState(PlayerStateBase* next);

	// 状態クラスから呼ぶ切り替え関数
	void ChangeToIdle() { ChangeState(&m_idleState); }
	void ChangeToMove() { ChangeState(&m_moveState); }
	void ChangeToJump() { ChangeState(&m_jumpState); }
	void ChangeToAttack() { ChangeState(&m_attackState); }
	void ChangeToQuickBoost() { ChangeState(&m_quickBoostState); }


	// 状態クラスが遷移判断に使う情報
	bool HasMoveInput() const { return m_hasMoveInput; }
	bool IsAttackInput() const;      // Qキーが押されたか
	bool IsQuickBoostInput() const;  // QuickBoostの入力があるか

	float GetAttackDuration() const
	{
		return pModel ? pModel->GetAnimationDuration("Attack") : 1.0f;
	}

	void ChangeToGroundOrAir()
	{
		if (!m_isGrounded)      ChangeToJump();
		else if (m_hasMoveInput) ChangeToMove();
		else                     ChangeToIdle();
	}

	void SetSpeedLimit(float limit) { m_currentSpeedLimit = limit; }
	void ResetSpeedLimit() { m_currentSpeedLimit = m_moveSpeed; }
	float& GetBoostSpeed() { return m_boostSpeed; }
	float& GetDuration() { return m_duration; }

	void SetLockOnTarget(GameObject* target) { m_lockOnTarget = target; }
	bool IsLockOn() const { return m_isLockOn && m_lockOnTarget != nullptr; }
	GameObject* GetLockOnTarget() const { return m_lockOnTarget; }

private:
	float m_moveSpeed = 20.0f;
	float m_currentSpeedLimit = 20.0f;
	float m_turnSpeed = 10.0f;
	float m_jumpPower = 5.0f;
	float coyoteTimer = 0.0f;
	float coyoteTime = 0.2f;
	bool m_isGrounded = false;

	float m_timer = 0.0f;
	float m_duration = 0.2f;    // 持続時間
	float m_boostSpeed = 40.0f; // この速度で飛ぶ(通常の2倍)

	float m_moveAccel = 60.0f;   // 通常の加速度
	float m_brakeAccel = 100.0f;
	float m_moveDirX = 0.0f;   // カメラ変換後の移動方向(正規化済み)
	float m_moveDirZ = 0.0f;
	float m_inputIntensity = 0.0f;  // 入力の強さ(0〜1)
	bool  m_hasMoveInput = false;   // 入力があるか(len > 0.5f の判定結果)

	void UpdateInput(float camYaw);               // 入力の取得
	void UpdateMove(float dt);      // 移動(加速・ブレーキ・速度制限)
	void UpdateRotation(float dt);  // 向きの制御
	void UpdateJump(float dt);              // ジャンプ

	IdleState m_idleState;
	MoveState m_moveState;
	JumpState m_jumpState;
	AttackState m_attackState;
	QuickBoostState m_quickBoostState;
	PlayerStateBase* m_currentState = nullptr;

	GameObject* m_lockOnTarget = nullptr;
	bool m_isLockOn = false;
};

