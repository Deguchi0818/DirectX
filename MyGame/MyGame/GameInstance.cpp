#include "GameInstance.h"
#include "MyMatrix4x4.h"
#include "Transform.h"
#include "Common.h"
#include "Mesh.h"
#include "GeometryGenerator.h"
#include "Input.h"
#include "Collider.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Coin.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

#include <WICTextureLoader.h>
#include <functional>

using Microsoft::WRL::ComPtr;

GameInstance::GameInstance() {};
GameInstance::~GameInstance() { Finalize(); }

bool GameInstance::Initialize(HWND hWnd, int width, int height) 
{
    HRESULT hr_com = CoInitializeEx(NULL, 0);

    if (!m_graphics.Initialize(hWnd, width, height)) return false;
    auto device = m_graphics.GetDevice();

    if (!CreateAssets(device)) return false;

    m_scene.Create(m_resourceManager, m_physics);

    m_scene.UpdateAnimation();
    m_scene.UpdateTransforms();

    m_camera.GetTransform().SetPosition(0.0f, 2.0f, -5.0f);

    m_lastTime = std::chrono::high_resolution_clock::now();

    Input::Initialize();

    IMGUI_CHECKVERSION();
    m_debugUI.Initialize(device, m_graphics.GetContext(), hWnd);

    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    Microsoft::WRL::ComPtr<ID3D11BlendState> pBlendState;
    device->CreateBlendState(&bd, &pBlendState);
    //m_graphics.GetContext()->OMSetBlendState(pBlendState.Get(), nullptr, 0xffffffff);

    return true;
}

void GameInstance::Update() 
{
    UpdateSystem();
   
    m_scene.GetPlayer().Update(m_deltaTime, m_camera.GetYaw());

    m_scene.UpdateAnimation();


    m_physics.Update(m_deltaTime);

    if (m_scene.GetPlayer().IsLockOn())
    {
        m_camera.UpdateLockOn(m_scene.GetPlayer().transform.GetWorldPosition(),
            m_scene.GetPlayer().GetLockOnTarget()->transform.GetWorldPosition(),
            m_deltaTime);
    }

    m_camera.UpdateTPS(m_scene.GetPlayer().transform.GetPosition());

    m_scene.UpdateTransforms();
}

void GameInstance::UpdateSystem() 
{
    // デルタタイムの計算
    auto currentTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<float>(currentTime - m_lastTime).count();
    m_lastTime = currentTime;

    // 1フレームが長すぎた時（デバッグ中断後など）のスパイク対策
    if (m_deltaTime > 0.1f) m_deltaTime = 0.1f;

    Input::Update();

    if (Input::GetKeyDown(VK_TAB))
    {
        m_debugUI.ToggleDebugMode();
    }

    ImGuiIO& io = ImGui::GetIO();
    if (m_debugUI.IsDebugMode() || io.WantCaptureMouse)
    {
        while (::ShowCursor(TRUE) < 0);
        return;
    }
    else
    {
        while (::ShowCursor(FALSE) >= 0);

        static POINT lastMousePos;
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);

        int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
        int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;

        static bool firstFrame = true;
        if (firstFrame)
        {
            // 最初のフレームでマウスを中央に飛ばす
            SetCursorPos(centerX, centerY);
            lastMousePos = { centerX, centerY };
            firstFrame = false;
            return;
        }

        // 中央からの移動量を計算
        float dx = (float)(currentMousePos.x - centerX);
        float dy = (float)(currentMousePos.y - centerY);

        // カメラを回転させる
        if (!m_scene.GetPlayer().IsLockOn())
        {
            m_camera.Rotate(dx, dy);
        }

        // マウスを中央に戻す
        SetCursorPos(centerX, centerY);
    }
    
}

void GameInstance::Render()
{
    m_debugUI.BeginFrame();
    m_debugUI.BuildWindows(m_scene, m_camera, m_deltaTime);

    // 描画開始
    m_graphics.BeginScene(0.1f, 0.2f, 0.4f, 1.0f);

    auto context = m_graphics.GetContext();

    // GPUへ転送
    m_baseShader.UpdateBones(context, m_scene.GetSkinMatrices());
    m_baseShader.Bind(context);


    // 行列の準備
    auto view = m_camera.GetViewMatrix();
    float aspect = 1280.0f / 720.0f;
    auto proj = m_camera.GetProjectionMatrix(aspect);

    // 全てのオブジェクトを描画するループ
    m_scene.Draw(context, &m_baseShader, m_constantBuffer.Get(), view, proj);

    m_debugUI.RenderImGui();
    m_debugUI.DrawColliders(context, m_scene, view, proj);

    //描画終了
    m_graphics.EndScene();
}

bool GameInstance::CreateAssets(ID3D11Device* device) 
{
    m_baseShader.Load(device, L"VertexShader.hlsl", L"PixelShader.hlsl");

    m_resourceManager.LoadModel(device, "Player", "Asset/Idle.fbx");
    m_resourceManager.LoadModel(device, "Sword", "Asset/Sword.fbx");
    Model* pPlayerModel = m_resourceManager.GetModel("Player");
    if (pPlayerModel) 
    {
		pPlayerModel->LoadAnimation("Idle", "Asset/Idle.fbx");
        pPlayerModel->LoadAnimation("Running", "Asset/Running.fbx");
        pPlayerModel->LoadAnimation("Jump", "Asset/Jump.fbx");
        pPlayerModel->LoadAnimation("Attack", "Asset/Attack.fbx");
    }

    m_resourceManager.CreateCube(device, "Cube", 1.0f, { 0.0f, 0.5f, 1.0f, 1.0f });
    m_resourceManager.CreatePlane(device, "Plane", 1.0f, 1.0f, { 0.0f, 0.5f, 0.0f, 1.0f });

    // 定数バッファ作成
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBufferData);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;          // 動的に変更可能にする
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    return true;
}

void GameInstance::Finalize() 
{
    CoUninitialize();
}