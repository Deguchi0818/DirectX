#include "GameInstance.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

GameInstance::GameInstance() {};
GameInstance::~GameInstance() { Finalize(); }

bool GameInstance::Initialize(HWND hWnd, int width, int height) 
{
    // デバイスとスワップチェーンの作成
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 一般的なRGBA
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1; // アンチエイリアスなし
    sd.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevels, 1, D3D11_SDK_VERSION, &sd,
        &m_swapChain, &m_device, nullptr, &m_context
    );
    if (FAILED(hr)) return false;

    // レンダーターゲットビュー（キャンバス）の作成
    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTarget);

    // 描画範囲（ビューポート）の設定
    D3D11_VIEWPORT vp = { 0, 0, (float)width, (float)height, 0.0f, 1.0f };
    m_context->RSSetViewports(1, &vp);

    Vertex vertices[] =
    {
        {  0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f }, // 赤
        {  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f }, // 緑
        { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f }, // 青
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    m_device->CreateBuffer(&bd, &initData, &m_vertexBuffer);

    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

    };

    m_device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

    D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBufferData); // 構造体のサイズ
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // 「定数バッファ」として指定

    m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    return true;
}

void GameInstance::Render()
{
    // 画面をクリアする色
    float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };

    m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), nullptr);

    // 指定した色でキャンバスを塗りつぶす
    m_context->ClearRenderTargetView(m_renderTarget.Get(), clearColor);

    static float angle = 0.0f;
    angle += 0.01f; // 回転角を更新

    // ワールド行列（回転）
    DirectX::XMMATRIX world = DirectX::XMMatrixRotationY(angle);
    // ビュー行列（カメラの位置）
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);
    DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, at, up);
    // プロジェクション行列（遠近感）
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, 1280.0f / 720.0f, 0.1f, 100.0f);

    // 3つを掛け合わせる（DirectXMathでは右から掛ける点に注意）
    ConstantBufferData cbData;
    cbData.wvp = DirectX::XMMatrixTranspose(world * view * proj); // HLSL用に転置する(CPU と GPU で行列の並びが違う)

    // GPUへデータを転送
    m_context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cbData, 0, 0);

    // 定数バッファをシェーダーにセット
    m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    m_context->IASetInputLayout(m_inputLayout.Get()); // レイアウトをセット
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0); // 頂点シェーダーをセット
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0); // ピクセルシェーダーをセット

    // どの頂点バッファを使うかセット
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    // 三角形として描く設定
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // 描画実行
    m_context->Draw(3, 0);

    // 描画結果を画面に反映（スワップ）
    m_swapChain->Present(1, 0);
}

void GameInstance::Finalize() 
{

}