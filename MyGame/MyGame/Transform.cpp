#include "Transform.h"

Transform::Transform()
    : m_position(0, 0, 0), m_rotation(0, 0, 0), m_scale(1, 1, 1) {
    UpdateMatrix();
}

void Transform::UpdateMatrix() 
{
    // --------------------------------------------------------
    // 各成分(スケール・回転・移動)の独立した行列を生成
    // --------------------------------------------------------
    const float toRadian = 3.14159265f / 180.0f;

    MyMatrix4x4 matS = MyMatrix4x4::CreateScale(m_scale.x, m_scale.y, m_scale.z);

    MyMatrix4x4 matRX = MyMatrix4x4::CreateRotationX(m_rotation.x * toRadian);
    MyMatrix4x4 matRY = MyMatrix4x4::CreateRotationY(m_rotation.y * toRadian);
    MyMatrix4x4 matRZ = MyMatrix4x4::CreateRotationZ(m_rotation.z * toRadian);

    MyMatrix4x4 matT = MyMatrix4x4::CreateTranslation(m_position.x, m_position.y, m_position.z);

    // --------------------------------------------------------
    // 回転行列の合成 (Z -> X -> Y の順序)
    // --------------------------------------------------------
    MyMatrix4x4 matR = MyMatrix4x4::Multiply(matRZ, matRX);
    matR = MyMatrix4x4::Multiply(matR, matRY);

    // --------------------------------------------------------
    // SRTによる最終ワールド行列の合成
    // --------------------------------------------------------
    m_worldMatrix = MyMatrix4x4::Multiply(matS, matR);
    m_worldMatrix = MyMatrix4x4::Multiply(m_worldMatrix, matT);
}