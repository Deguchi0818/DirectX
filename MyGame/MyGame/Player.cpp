#include "Player.h"
#include "Input.h"
#include <Windows.h>
#include <cmath>
#include <string>
#include <vector>

void Player::Initialize(Model* model) 
{
    pModel = model;
    transform.SetScale(0.01f, 0.01f, 0.01f);
    transform.SetRotation(0.0f, 0.0f, 0.0f);
    transform.SetPosition(0, 0.5f, 0); // ‰ŠúˆÊ’u

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

    float len = sqrtf(moveX * moveX + moveZ * moveZ);
    if (len > 0.0f)
    {
        m_state = PlayerState::Run;
        // “ü—Í‚Ì‹­‚³‚ð•Û‘¶
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
        transform.SetRotation(0.0f, targetYaw - 180.0f, 0.0f);

        //float targetYaw = atan2f(finalMoveX, finalMoveZ) * (180.0f / 3.14159265f);
        //transform.SetRotation(90.0f, targetYaw + -90.0f, 0.0f);

        transform.UpdateMatrix();
    }
    else
    {
        m_state = PlayerState::Idle;
        //vel.x = 0.0f;
        //vel.z = 0.0f;
    }

    if (coyoteTimer >= 0) 
    {
        coyoteTimer -= dt;
    }

    if (Input::GetKey(VK_SPACE) && (m_isGrounded || coyoteTimer >= 0) || 
        Input::GetButtonDown(XINPUT_GAMEPAD_A) && (m_isGrounded || coyoteTimer >= 0))
    {
        if(m_isGrounded == false)
        m_state = PlayerState::Jump;
        vel.y = m_jumpPower;
        m_isGrounded = false;
        coyoteTimer = -1.0f;
    }

    SetVelocity(vel);

    m_isGrounded = false;
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