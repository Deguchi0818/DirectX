#include "Transform.h"

Transform::Transform()
    : m_localPosition{ 0.0f, 0.0f, 0.0f }
    , m_localRotation{ 0.0f, 0.0f, 0.0f }
    , m_localScale{ 1.0f, 1.0f, 1.0f }
{
    UpdateMatrix();
}

void Transform::UpdateMatrix() 
{
    // --------------------------------------------------------
    // 各成分(スケール・回転・移動)の独立した行列を生成
    // --------------------------------------------------------
    const float toRadian = 3.14159265f / 180.0f;

    MyMatrix4x4 matS = MyMatrix4x4::CreateScale(m_localScale.x, m_localScale.y, m_localScale.z);

    MyMatrix4x4 matRX = MyMatrix4x4::CreateRotationX(m_localRotation.x * toRadian);
    MyMatrix4x4 matRY = MyMatrix4x4::CreateRotationY(m_localRotation.y * toRadian);
    MyMatrix4x4 matRZ = MyMatrix4x4::CreateRotationZ(m_localRotation.z * toRadian);

    MyMatrix4x4 matT = MyMatrix4x4::CreateTranslation(m_localPosition.x, m_localPosition.y, m_localPosition.z);

    // --------------------------------------------------------
    // 回転行列の合成 (Z -> X -> Y の順序)
    // --------------------------------------------------------
    MyMatrix4x4 matR = MyMatrix4x4::Multiply(matRZ, matRX);
    matR = MyMatrix4x4::Multiply(matR, matRY);

    // --------------------------------------------------------
    // SRTによる最終ワールド行列の合成
    // --------------------------------------------------------
    m_localMatrix = MyMatrix4x4::Multiply(matS, matR);
    m_localMatrix = MyMatrix4x4::Multiply(m_localMatrix, matT);

    if (m_parent != nullptr)
    {
        m_worldMatrix = MyMatrix4x4::Multiply(m_localMatrix, m_parent->GetWorldMatrix());
    }
    else
    {
        m_worldMatrix = m_localMatrix;
    }

}

MyVector3 Transform::GetWorldPosition() const
{
    return { m_worldMatrix.m[3][0], m_worldMatrix.m[3][1], m_worldMatrix.m[3][2] };
}

MyVector3 Transform::GetWorldScale() const
{
    auto rowLength = [](const float r[4]) -> float
        {
            return sqrtf(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
        };

    return {
        rowLength(m_worldMatrix.m[0]),
        rowLength(m_worldMatrix.m[1]),
        rowLength(m_worldMatrix.m[2])
    };
}