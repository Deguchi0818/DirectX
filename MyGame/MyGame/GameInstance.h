#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>
#include <vector>
#include "GameObject.h"


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

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_indexBuffer;

	std::vector<GameObject> m_gameObjects;

	Mesh m_cubeMesh;
	Mesh m_planeMesh;

	std::vector<Vertex> v;
	std::vector<unsigned short> i;
};

