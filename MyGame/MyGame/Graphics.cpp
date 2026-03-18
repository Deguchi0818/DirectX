#include "Graphics.h"

using Microsoft::WRL::ComPtr;

bool Graphics::Initialize(HWND hWnd, int width, int height) {
    // デバイスとスワップチェーンの作成
    DXGI_SWAP_CHAIN_DESC sd{
        .BufferDesc = {.Width = (UINT)width, .Height = (UINT)height, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
        .SampleDesc = {.Count = 1 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 1,
        .OutputWindow = hWnd,
        .Windowed = TRUE
    };

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevels, 1, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, nullptr, &m_context);
    if (FAILED(hr)) return false;

    // 深度バッファとレンダーターゲットの作成
    D3D11_TEXTURE2D_DESC depthDesc{
        .Width = (UINT)width, .Height = (UINT)height, .MipLevels = 1, .ArraySize = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT, .SampleDesc = {.Count = 1 },
        .Usage = D3D11_USAGE_DEFAULT, .BindFlags = D3D11_BIND_DEPTH_STENCIL
    };
    ComPtr<ID3D11Texture2D> depthBuffer;
    m_device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer);
    m_device->CreateDepthStencilView(depthBuffer.Get(), nullptr, &m_depthStencilView);

    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTarget);

    // ビューポートの設定
    D3D11_VIEWPORT vp{ 0, 0, (float)width, (float)height, 0.0f, 1.0f };
    m_context->RSSetViewports(1, &vp);

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable = TRUE;                            // 奥行き判定を有効にする
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;   // 奥行きを書き込む
    dsd.DepthFunc = D3D11_COMPARISON_LESS;             // 手前にあるものを描画する

    hr = m_device->CreateDepthStencilState(&dsd, &m_depthStencilState);
    if (FAILED(hr)) return false;

    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    //rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.CullMode = D3D11_CULL_BACK;

    // 修正: メンバ変数 m_rasterizerState に直接作成して保存する
    m_device->CreateRasterizerState(&rd, &m_rasterizerState);
    m_context->RSSetState(m_rasterizerState.Get());

    return true;
}

void Graphics::BeginScene(float r, float g, float b, float a) {
    float clearColor[] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_renderTarget.Get(), clearColor);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    m_context->RSSetState(m_rasterizerState.Get());

    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
}

void Graphics::EndScene() {
    m_swapChain->Present(1, 0);
}