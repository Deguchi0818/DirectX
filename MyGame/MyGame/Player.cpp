#include "Player.h"
#include "Input.h"
#include <Windows.h>
#include <cmath>
#include <string>
#include <vector>

void Player::Initialize(Model* model) 
{
    pModel = model;
    transform.SetScale(1.0f, 1.5f, 1.0f);
    transform.SetPosition(0, 0.5f, 0); // ‰ŠúˆÊ’u

    AddCollider("foot", ColliderType::AABB, { 0, 0.05f, 0 }, { 0.6f, 0.1f, 0.6f });
    AddCollider("body", ColliderType::AABB, {0, 0.3f, 0}, {0.8f, 1.0f, 0.8f});
    AddCollider("head", ColliderType::AABB, { 0, 1.3f, 0 }, { 0.4f, 0.4f, 0.4f });

    transform.UpdateMatrix();
}

void Player::Update(float dt, float camYaw)
{
    float moveX = 0.0f;
    float moveZ = 0.0f;

    if (Input::GetKey('W')) moveZ += 1.0f;
    if (Input::GetKey('S')) moveZ -= 1.0f;
    if (Input::GetKey('A')) moveX -= 1.0f;
    if (Input::GetKey('D')) moveX += 1.0f;

    MyVector3 vel = GetVelocity();

    float len = sqrtf(moveX * moveX + moveZ * moveZ);
    if (len > 0.0f)
    {
        moveX /= len;
        moveZ /= len;

        float fwdX = sinf(camYaw);
        float fwdZ = cosf(camYaw);
        float rtX = cosf(camYaw);
        float rtZ = -sinf(camYaw);

        float finalMoveX = (moveX * rtX) + (moveZ * fwdX);
        float finalMoveZ = (moveX * rtZ) + (moveZ * fwdZ);

        vel.x = finalMoveX * m_moveSpeed;
        vel.z = finalMoveZ * m_moveSpeed;

        float targetYaw = atan2f(finalMoveX, finalMoveZ);
        transform.SetRotation(0, targetYaw, 0);

        transform.UpdateMatrix();
    }
    else
    {
        vel.x = 0.0f;
        vel.z = 0.0f;
    }

    if (coyoteTimer >= 0) 
    {
        coyoteTimer -= dt;
    }

    if (Input::GetKey(VK_SPACE) && (m_isGrounded || coyoteTimer >= 0))
    {
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
}