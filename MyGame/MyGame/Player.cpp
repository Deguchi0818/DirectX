#include "Player.h"
#include "Input.h"
#include <Windows.h>
#include <cmath>
#include <string>
#include <vector>

void Player::Initialize(Model* model) 
{
    pModel = model;

    m_stateAnimMap[PlayerState::Idle] = "Idle";
    m_stateAnimMap[PlayerState::Run] = "Running";
    m_stateAnimMap[PlayerState::Jump] = "Jump";
    m_stateAnimMap[PlayerState::Attack] = "Attack";

    transform.SetScale(0.01f, 0.01f, 0.01f);
    transform.SetRotation(0.0f, 0.0f, 0.0f);
    transform.SetPosition(0, 0.5f, 0); // 初期位置

   // AddCollider("foot", ColliderType::AABB, { 0, 0.0f, 0 }, { 10.0f, 1.0f, 10.0f });
    auto& bodyCol = AddCollider("body", ColliderType::Capsule, { 0, 90.0f, 0 }, { 100.0f, 160.0f, 100.0f });
    bodyCol.radius = 20.0f;
    bodyCol.height = 130.0f;
    bodyCol.isTrigger = false;
    auto& headCol = AddCollider("head", ColliderType::Sphere, { 0, 19.0f, 0 }, { 10.0f, 10.0f, 10.0f });
    headCol.radius = 2.0f;

    m_restitution = 0.0f;
    m_friction = 5.0f;

    m_showCollider = true;

    transform.UpdateMatrix();
}

void Player::Update(float dt, float camYaw)
{
    float moveX = 0.0f;
    float moveZ = 0.0f;

    moveX = Input::GetAxisX();
    moveZ = Input::GetAxisZ();

    if (Input::GetKey('W')) moveZ += 1.0f;
    if (Input::GetKey('S')) moveZ -= 1.0f;
    if (Input::GetKey('A')) moveX -= 1.0f;
    if (Input::GetKey('D')) moveX += 1.0f;

    MyVector3 vel = GetVelocity();

    UpdateAnimTimer(dt);

    float len = sqrtf(moveX * moveX + moveZ * moveZ);
    if (len > 0.5f)
    {
        if (m_isGrounded) m_state = PlayerState::Run;
        // 入力の強さを保存
        float inputIntensity = (len > 1.0f) ? 1.0f : len;

        moveX /= len;
        moveZ /= len;

        float fwdX = sinf(camYaw);
        float fwdZ = cosf(camYaw);
        float rtX = cosf(camYaw);
        float rtZ = -sinf(camYaw);

        float finalMoveX = (moveX * rtX) + (moveZ * fwdX);
        float finalMoveZ = (moveX * rtZ) + (moveZ * fwdZ);

        vel.x = finalMoveX * m_moveSpeed * inputIntensity;
        vel.z = finalMoveZ * m_moveSpeed * inputIntensity;

        float targetYaw = atan2f(finalMoveX, finalMoveZ) * (180.0f / 3.14159265f);
        //transform.SetRotation(0.0f, targetYaw - 180.0f, 0.0f);

        //float targetYaw = atan2f(finalMoveX, finalMoveZ) * (180.0f / 3.14159265f);
        //transform.SetRotation(90.0f, targetYaw + -90.0f, 0.0f);

		float currentYaw = transform.GetRotation().y;
		float target = targetYaw - 180.0f;

		float diff = fmodf(target - currentYaw, 360.0f);
		if (diff < -180.0f) diff += 360.0f;
		if (diff > 180.0f) diff -= 360.0f;

        const float turnBlend = 1.0f - expf(-m_turnSpeed * dt);
        float newYaw = currentYaw + diff * turnBlend;
        transform.SetRotation(0.0f, newYaw, 0.0f);

        transform.UpdateMatrix();
    }
    else
    {
        if (m_isGrounded && m_state != PlayerState::Attack) m_state = PlayerState::Idle;
        //vel.x = 0.0f;
        //vel.z = 0.0f;
    }

    if (coyoteTimer >= 0) 
    {
        coyoteTimer -= dt;
    }

    if (Input::GetKeyDown(VK_SPACE) && (m_isGrounded || coyoteTimer >= 0) || 
        Input::GetButtonDown(XINPUT_GAMEPAD_A) && (m_isGrounded || coyoteTimer >= 0))
    {
        m_state = PlayerState::Jump;
        vel.y = m_jumpPower;
        m_isGrounded = false;
        coyoteTimer = -1.0f;
    }

    if (Input::GetKeyDown('Q') && m_isGrounded && m_state != PlayerState::Attack)
    {
        m_state = PlayerState::Attack;
		m_attackTimer = 0.0f;

    }

    else if(m_state == PlayerState::Attack)
    {
        m_attackTimer += dt;

		float animDuration = pModel->GetAnimationDuration(m_stateAnimMap[m_state]);

        if (m_attackTimer >= animDuration) // 攻撃アニメーションの長さに応じて調整
        {
            m_state = (m_isGrounded && len > 0.5f)
                ? PlayerState::Run
                : PlayerState::Idle;
        }
	}

    SetVelocity(vel);

    m_isGrounded = false;

    if (m_stateAnimMap.count(m_state) > 0) 
    {
        if (m_state == PlayerState::Jump) 
        {
            // ジャンプの時だけ、しゃがみを飛ばしてから再生する
            PlayAnimation(m_stateAnimMap[m_state], 0.7f);
        }
        else 
        {
            // 走る・待機などは今まで通り再生
            PlayAnimation(m_stateAnimMap[m_state], 0.0f);
        }
    }
}

void Player::OnCollisionEnter(std::string myCol, GameObject* other, std::string otherCol)
{
    if (otherCol == "floor_main")
    {
        m_isGrounded = true;
        coyoteTimer = coyoteTime;
    }
    if (myCol == "head") 
    {
        m_hitHead = true;
    }
}

void Player::DrawWeapon(ID3D11DeviceContext* context, Shader* shader, ID3D11Buffer* cb, const MyMatrix4x4& view, const MyMatrix4x4& proj, const std::vector<DirectX::XMMATRIX>& worldMatrices)
{
    // 手首ボーンが未設定、または武器を装備していない場合は処理をスキップ
    if (!m_equippedWeapon || m_handBoneIndex == -1) return;

    // --------------------------------------------------------
    // プレイヤーのワールド行列を取得・変換
    // --------------------------------------------------------
    transform.UpdateMatrix();
    MyMatrix4x4 pWorldMat = transform.GetWorldMatrix();
    DirectX::XMMATRIX playerWorld = *(DirectX::XMMATRIX*)&pWorldMat; 

    // --------------------------------------------------------
    // 手首ボーンの最終ワールド行列を算出
    // --------------------------------------------------------
    DirectX::XMMATRIX handMatrix = worldMatrices[m_handBoneIndex] * playerWorld;

    // --------------------------------------------------------
    // 武器の姿勢更新と描画
    // --------------------------------------------------------
    m_equippedWeapon->FollowToBone(handMatrix);
    m_equippedWeapon->Draw(context, shader, cb, view, proj, &handMatrix);
}