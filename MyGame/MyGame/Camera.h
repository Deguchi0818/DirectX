#pragma once
#include "Transform.h"
#include "MyMatrix4x4.h"
#include <numbers>
class Camera
{
public:
	Camera() = default;

	void Update(float dt);

	void UpdateTPS(const MyVector3& targetPos);

	void Rotate(float dx, float dy);

	[[nodiscard]] Transform& GetTransform() { return m_transform; }
	[[nodiscard]] MyMatrix4x4 GetViewMatrix() const;
	[[nodiscard]] MyMatrix4x4 GetProjectionMatrix(float aspect) const;

	float GetYaw() { return m_yaw; }
	float GetPitch() { return m_pitch; }

	float& GetSensitivity() { return sensitivity; }
	float& GetRightStickSensitivity() { return rightStickSensitivity; }

private:
	Transform m_transform;
	float m_fov{ std::numbers::pi_v<float> / 4.0f };	// フィールド・オブ・ビュー

	float m_yaw = 0.0f;
	float m_pitch = 0.0f;

	float sensitivity = 0.0005f;
	float rightStickSensitivity = 0.01f;	// 右スティックの感度
};

