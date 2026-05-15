#pragma once
#include "Common.h"
#include "Collider.h"
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

struct DebugVertex 
{
	MyVector3 pos;
	MyVector4 color;
};

class DebugRenderer
{
public:
	bool Initialize(ID3D11Device* device);
	void Finalize();

	void AddLine(const MyVector3& p1, const MyVector3& p2, const MyVector4& color);
	void AddAABB(const AABB& aabb, const MyVector4& color);
	void AddSphere(const Sphere& sphere, const MyVector4& color);
	void AddCapsule(const Capsule& capsule, const MyVector4& color);

	void Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& viewProj);

private:
	std::vector<DebugVertex> m_vertices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};

