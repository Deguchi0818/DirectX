#include "Player.h"
#include "Input.h"
#include <Windows.h>
#include <cmath>

void Player::Initialize(Model* model) 
{
    m_object.pModel = model;
    m_object.transform.SetScale(1.0f, 1.5f, 1.0f);
    m_object.transform.SetPosition(0, 0.7f, 0); // ‰ŠúˆÊ’u

    m_object.transform.UpdateMatrix();
}

void Player::Update(float dt, float camYaw)
{
    float moveX = 0.0f;
    float moveZ = 0.0f;

    if (Input::GetKey('W')) moveZ += 1.0f;
    if (Input::GetKey('S')) moveZ -= 1.0f;
    if (Input::GetKey('A')) moveX -= 1.0f;
    if (Input::GetKey('D')) moveX += 1.0f;

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

        MyVector3 pos = m_object.transform.GetPosition();
        pos.x += finalMoveX * m_moveSpeed * dt;
        pos.z += finalMoveZ * m_moveSpeed * dt;
        m_object.transform.SetPosition(pos.x, pos.y, pos.z);

        float targetYaw = atan2f(finalMoveX, finalMoveZ);
        m_object.transform.SetRotation(0, targetYaw, 0);

        m_object.transform.UpdateMatrix();
    }
}
void Player::Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer,
    const MyMatrix4x4& view, const MyMatrix4x4& projection)
{
    m_object.Draw(context, constantBuffer, view, projection);
}