#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>
#include <vector>

class Graphics {
public:
    Graphics() = default;

    // 初期化（ウィンドウハンドルとサイズを渡す）
    bool Initialize(HWND hWnd, int width, int height);

    // フレーム開始処理（画面クリアなど）
    void BeginScene(float r, float g, float b, float a);

    // フレーム終了処理（画面反映）
    void EndScene();

    // デバイスやコンテキストを取得する関数（他のクラスが使うため）
    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11Device>           m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTarget;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

};