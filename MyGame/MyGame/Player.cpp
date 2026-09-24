#include "Player.h"
#include "Input.h"
#include <Windows.h>
#include <cmath>
#include <string>
#include <vector>

void Player::Initialize(Model* model) 
{
    pModel = model;

    ChangeState(&m_idleState);

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

}

void Player::Update(float dt, float camYaw)
{
     UpdateAnimTimer(dt);

     UpdateInput(camYaw);
     UpdateMove(dt);
     UpdateRotation(dt);
     UpdateJump(dt);

     m_currentState->Update(*this, dt);

     PlayAnimation(m_currentState->GetAnimName());

     m_isGrounded = false;

}

void Player::UpdateInput(float camYaw) 
{
    m_moveDirX = Input::GetAxisX();
    m_moveDirZ = Input::GetAxisZ();

    if (Input::GetKey('W')) m_moveDirZ += 1.0f;
    if (Input::GetKey('S')) m_moveDirZ -= 1.0f;
    if (Input::GetKey('A')) m_moveDirX -= 1.0f;
    if (Input::GetKey('D')) m_moveDirX += 1.0f;

    float len = sqrtf(m_moveDirX * m_moveDirX + m_moveDirZ * m_moveDirZ);
    m_hasMoveInput = (len > 0.5f);
    if (m_hasMoveInput)
    {
        m_inputIntensity = (len > 1.0f) ? 1.0f : len;

        m_moveDirX /= len;
        m_moveDirZ /= len;

        float fwdX = sinf(camYaw);
        float fwdZ = cosf(camYaw);
        float rtX = cosf(camYaw);
        float rtZ = -sinf(camYaw);

        float finalMoveX = (m_moveDirX * rtX) + (m_moveDirZ * fwdX);
        float finalMoveZ = (m_moveDirX * rtZ) + (m_moveDirZ * fwdZ);

        m_moveDirX = finalMoveX;
        m_moveDirZ = finalMoveZ;

    }

    if (Input::GetKeyDown('R'))
    {
        m_isLockOn = !m_isLockOn;
    }
}

void Player::UpdateMove(float dt) 
{
    MyVector3 vel = GetVelocity();

    if (m_hasMoveInput) 
    {
        float dot = vel.x * m_moveDirX + vel.z * m_moveDirZ;
        float accel = (dot < 0.0f) ? m_brakeAccel : m_moveAccel;

        vel.x += m_moveDirX * accel * m_inputIntensity * dt;
        vel.z += m_moveDirZ * accel * m_inputIntensity * dt;


    }

    float velLen = sqrtf(vel.x * vel.x + vel.z * vel.z);

    if (velLen > m_currentSpeedLimit)
    {
        vel.x /= velLen;
        vel.x *= m_currentSpeedLimit;

        vel.z /= velLen;
        vel.z *= m_currentSpeedLimit;
    }

    SetVelocity(vel);
}

void Player::UpdateRotation(float dt) 
{
    float targetYaw;
    
    if (m_isLockOn && m_lockOnTarget != nullptr) 
    {
        MyVector3 myPos = transform.GetWorldPosition();
        MyVector3 targetPos = m_lockOnTarget->transform.GetWorldPosition();

        float dirX = targetPos.x - myPos.x;
        float dirZ = targetPos.z - myPos.z;

        targetYaw = atan2f(dirX, dirZ) * (180.0f / 3.14159265f);
    }

    else if (m_hasMoveInput) 
    {
        targetYaw = atan2f(m_moveDirX, m_moveDirZ) * (180.0f / 3.14159265f);
    }

    else
    {
        return;
    }
    

    float currentYaw = transform.GetRotation().y;
    // モデルが初期状態で手前を向いているため、180度ずらして補正する
    float target = targetYaw - 180.0f;

    float diff = fmodf(target - currentYaw, 360.0f);
    if (diff < -180.0f) diff += 360.0f;
    if (diff > 180.0f) diff -= 360.0f;

    const float turnBlend = 1.0f - expf(-m_turnSpeed * dt);
    float newYaw = currentYaw + diff * turnBlend;
    transform.SetRotation(0.0f, newYaw, 0.0f);
}

void Player::UpdateJump(float dt) 
{
    MyVector3 vel = GetVelocity();

    if (Input::GetKeyDown(VK_SPACE) && (m_isGrounded || coyoteTimer >= 0) ||
        Input::GetButtonDown(XINPUT_GAMEPAD_A) && (m_isGrounded || coyoteTimer >= 0))
    {
        vel.y = m_jumpPower;
        m_isGrounded = false;
        coyoteTimer = -1.0f;
        ChangeToJump();
    }

    if (coyoteTimer >= 0)
    {
        coyoteTimer -= dt;
    }

    SetVelocity(vel);
}

void Player::ChangeState(PlayerStateBase* next) 
{
    if (m_currentState == next) return;

    if (m_currentState) m_currentState->OnExit(*this);

    m_currentState = next;

    m_currentState->OnEnter(*this);
}

bool Player::IsAttackInput() const
{
    return Input::GetKeyDown('Q');
}

bool Player::IsQuickBoostInput() const
{
    return Input::GetKeyDown(VK_SHIFT);
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