#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Common.h"

class Mesh
{
public:
	void Create(ID3D11Device* device, Vertex* vArray, int vCount, unsigned int* iArray, int iCount);


	void BindBuffers(ID3D11DeviceContext* context);

	void Draw(ID3D11DeviceContext* context);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	int m_vertexCount;
	int m_indexCount;
};

