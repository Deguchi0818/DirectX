#include "GameInstance.h"
#include "MyMatrix4x4.h"
#include "Transform.h"
#include "Common.h"
#include "Mesh.h"
#include "GeometryGenerator.h"

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


    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 奥行き24bit、ステンシル8bit
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> depthBuffer;
    m_device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer);
    m_device->CreateDepthStencilView(depthBuffer.Get(), nullptr, &m_depthStencilView);

    // レンダーターゲットビュー（キャンバス）の作成
    ComPtr<ID3D11Texture2D> backBuffer;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTarget);

    // 描画範囲（ビューポート）の設定
    D3D11_VIEWPORT vp = { 0, 0, (float)width, (float)height, 0.0f, 1.0f };
    m_context->RSSetViewports(1, &vp);

    // 立方体 (サイズ 1.0, 青色) を生成
    GeometryGenerator::CreateCube(1.0f, { 0.0f, 0.5f, 1.0f, 1.0f }, v, i);
    m_cubeMesh.Create(m_device.Get(), v.data(), (int)v.size(), i.data(), (int)i.size());

    // 灰色の地面 (幅 10, 奥行 10)
    GeometryGenerator::CreatePlane(10.0f, 10.0f, { 0.3f, 0.3f, 0.3f, 1.0f }, v, i);
    m_planeMesh.Create(m_device.Get(), v.data(), (int)v.size(), i.data(), (int)i.size());


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

    //for (int i = 0; i < 1; i++) {
    //    GameObject obj;
    //    //obj.transform.SetPosition(i * 0.2f - 1.0f, 0, 0); // 横に並べる
    //    m_gameObjects.push_back(obj);
    //}

    m_gameObjects.clear();

    GameObject ground;
    ground.pMesh = &m_planeMesh;
    ground.transform.SetPosition(0, 0, 0);
    m_gameObjects.push_back(ground);

    // 2. 立方体をいくつか置く
    for (int i = 0; i < 3; i++) {
        GameObject cube;
        cube.pMesh = &m_cubeMesh;
        cube.transform.SetPosition(i * 1.5f - 1.5f, 0.5f, 0); // 地面より少し上に置く
        m_gameObjects.push_back(cube);
    }

    return true;


}

void GameInstance::Render()
{
    // 画面をクリアする色
    float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };

    m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());

    // 指定した色でキャンバスを塗りつぶす
    m_context->ClearRenderTargetView(m_renderTarget.Get(), clearColor);

    MyMatrix4x4 matCamT = MyMatrix4x4::CreateTranslation(0.0f, -1.0f, 5.0f);
    MyMatrix4x4 matCamR = MyMatrix4x4::CreateRotationX(-0.2f);

    MyMatrix4x4 view = MyMatrix4x4::Multiply(matCamT, matCamR);
    float aspect = 1280.0f / 720.0f;
    MyMatrix4x4 proj = MyMatrix4x4::CreatePerspective(0.785f, aspect, 0.1f, 100.0f);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    m_context->IASetInputLayout(m_inputLayout.Get()); // レイアウトをセット
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0); // 頂点シェーダーをセット
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0); // ピクセルシェーダーをセット

    // 三角形として描く設定
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    static float angle = 0;
    angle += 0.01f;

   for (auto& obj : m_gameObjects) 
    {
        // 立方体（pMeshがm_cubeMeshのもの）だけ回す
        if (obj.pMesh == &m_cubeMesh) {
            obj.transform.SetRotation(0, angle, 0);
        }
   
        obj.Draw(m_context.Get(), m_constantBuffer.Get(), view, proj);
    }

    // 描画結果を画面に反映（スワップ）
    m_swapChain->Present(1, 0);
}

void GameInstance::Finalize() 
{

}