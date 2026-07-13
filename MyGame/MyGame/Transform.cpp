#include "Transform.h"

Transform::Transform()
    : m_position(0, 0, 0), m_rotation(0, 0, 0), m_scale(1, 1, 1) {
    UpdateMatrix();
}

void Transform::UpdateMatrix() 
{
    // 「度」から「ラジアン」に変換する定数
    const float toRadian = 3.14159265f / 180.0f;

    MyMatrix4x4 matS = MyMatrix4x4::CreateScale(m_scale.x, m_scale.y, m_scale.z);

    MyMatrix4x4 matRX = MyMatrix4x4::CreateRotationX(m_rotation.x * toRadian);
    MyMatrix4x4 matRY = MyMatrix4x4::CreateRotationY(m_rotation.y * toRadian);
    MyMatrix4x4 matRZ = MyMatrix4x4::CreateRotationZ(m_rotation.z * toRadian);

    MyMatrix4x4 matT = MyMatrix4x4::CreateTranslation(m_position.x, m_position.y, m_position.z);

    MyMatrix4x4 matR = MyMatrix4x4::Multiply(matRZ, matRX);
    matR = MyMatrix4x4::Multiply(matR, matRY);

    m_worldMatrix = MyMatrix4x4::Multiply(matS, matR);
    m_worldMatrix = MyMatrix4x4::Multiply(m_worldMatrix, matT);
}