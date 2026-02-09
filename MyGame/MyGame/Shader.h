#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>

class Shader {
public:
    bool Load(ID3D11Device* device, const wchar_t* vsFile, const wchar_t* psFile);
    void Bind(ID3D11DeviceContext* context);

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pixelShader;
};