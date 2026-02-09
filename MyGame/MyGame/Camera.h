#pragma once
#include "Transform.h"
#include "MyMatrix4x4.h"
#include <numbers>
class Camera
{
public:
	Camera() = default;

	void Update();

	[[nodiscard]] Transform& GetTransform() { return m_transform; }

	[[nodiscard]] MyMatrix4x4 GetViewMatrix() const;
	[[nodiscard]] MyMatrix4x4 GetProjectionMatrix(float aspect) const;

private:
	Transform m_transform;
	float m_fov{ std::numbers::pi_v<float> / 4.0f };	// フィールド・オブ・ビュー
};

