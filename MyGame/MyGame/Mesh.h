#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Common.h"

class Mesh
{
public:
	void Create(ID3D11Device* device, Vertex* vertices, int vCount, unsigned short* indices, int iCount);

	void Draw(ID3D11DeviceContext* context);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	int m_indexCount;
};

