#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>

struct Vertex 
{
	float x, y, z;
	float r, g, b, a;
};

struct ConstantBuffer 
{
	DirectX::XMMATRIX worldViewProj; // 3つの行列を掛け合わせたもの
};

struct ConstantBufferData {
	DirectX::XMMATRIX wvp; // World * View * Projection
};

class GameInstance
{
public:
	GameInstance();
	~GameInstance();

	bool Initialize(HWND hWnd, int width, int height);
	void Render();
	void Finalize();

private:
	Microsoft::WRL::ComPtr<ID3D11Device>           m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;

	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_vertexBuffer; // 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D11InputLayout>      m_inputLayout;  // 入力レイアウト

	Microsoft::WRL::ComPtr<ID3D11VertexShader>     m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>      m_pixelShader;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
};

