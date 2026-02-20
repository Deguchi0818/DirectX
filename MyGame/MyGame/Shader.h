#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>

struct BoneBuffer {
	DirectX::XMMATRIX mBoneMatrices[128];
};

class Shader {
public:
    bool Load(ID3D11Device* device, const wchar_t* vsFile, const wchar_t* psFile);
    void Bind(ID3D11DeviceContext* context);

	void UpdateBones(ID3D11DeviceContext* context, const std::vector<DirectX::XMMATRIX>& matrices);

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_boneBuffer;
};