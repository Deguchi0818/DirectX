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

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

GameInstance::GameInstance() {};
GameInstance::~GameInstance() { Finalize(); }

bool GameInstance::Initialize(HWND hWnd, int width, int height) 
{
    if (!m_graphics.Initialize(hWnd, width, height)) return false;
    auto device = m_graphics.GetDevice();

    if (!CreateAssets(device)) return false;

    CreateScene();

    m_camera.GetTransform().SetPosition(0.0f, 2.0f, -5.0f);

    m_lastTime = std::chrono::high_resolution_clock::now();

    Input::Initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(m_graphics.GetDevice(), m_graphics.GetContext());

    return true;
}

void GameInstance::Update() 
{
    UpdateSystem();
   
    m_player.Update(m_deltaTime, m_camera.GetYaw());

    m_physics.Update(m_deltaTime);

    m_camera.UpdateTPS(m_player.transform.GetPosition());
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
        m_isDebugMode = !m_isDebugMode;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (m_isDebugMode || io.WantCaptureMouse)
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
        m_camera.Rotate(dx, dy);

        // マウスを中央に戻す
        SetCursorPos(centerX, centerY);
    }
    
}

void GameInstance::Render()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ここにデバッグメニューの内容を書く
    ImGui::Begin("Debug Menu");
    ImGui::Text("Player Settings");
    // ジャンプ力や移動速度をスライダーで調整できるようにする
    ImGui::SliderFloat("Jump Power", &m_player.GetJumpPower(), 0.0f, 20.0f);
    ImGui::SliderFloat("Move Speed", &m_player.GetMoveSpeed(), 0.0f, 20.0f);
    ImGui::End();

    // 描画開始
    m_graphics.BeginScene(0.1f, 0.2f, 0.4f, 1.0f);

    auto context = m_graphics.GetContext();

    m_baseShader.Bind(context);

    // 行列の準備
    auto view = m_camera.GetViewMatrix();
    float aspect = 1280.0f / 720.0f;
    auto proj = m_camera.GetProjectionMatrix(aspect);

    // 全てのオブジェクトを描画するループ
    for (auto& obj : m_gameObjects)
    {
        obj.Draw(context, m_constantBuffer.Get(), view, proj);
    }

    for (auto& obj : m_terrain)
    {
        obj.Draw(context, m_constantBuffer.Get(), view, proj);
    }

    m_player.Draw(context, m_constantBuffer.Get(), view, proj);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    //描画終了
    m_graphics.EndScene();
}

bool GameInstance::CreateAssets(ID3D11Device* device) 
{
    if (!m_baseShader.Load(device, L"VertexShader.hlsl", L"PixelShader.hlsl")) return false;

    m_cubeModel.CreateCube(device, 1.0f, { 0.0f, 0.5f, 1.0f, 1.0f });
    m_planeModel.CreatePlane(device, 1.0f, 1.0f, { 0.0f, 0.5f, 0.0f, 1.0f });

    // 定数バッファ作成
    D3D11_BUFFER_DESC cbDesc{ .ByteWidth = sizeof(ConstantBufferData), .BindFlags = D3D11_BIND_CONSTANT_BUFFER };
    device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    return true;
}

void GameInstance::CreateScene() 
{
    m_gameObjects.clear();
    m_terrain.clear();

    // オブジェクトの配置
    GameObject floor;
    floor.pModel = &m_planeModel;
    floor.transform.SetPosition(0, 0, 0);
    floor.transform.SetScale(10.0f, 1.0f, 10.0f);
    floor.isStatic = true;
    floor.AddCollider("floor_main", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    m_terrain.push_back(floor);

    GameObject wall;
    wall.pModel = &m_cubeModel;
    wall.transform.SetPosition(5.0f, 0.5f, 0.0f);
    wall.transform.SetScale(1.0f, 2.0f, 5.0f);
    wall.isStatic = true;
    wall.m_isTrigger = false;
    wall.AddCollider("wall_main", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    m_terrain.push_back(wall);

    GameObject coin;
    coin.pModel = &m_cubeModel;
    coin.transform.SetPosition(-3.0f, 1.0f, 2.0f);
    coin.transform.SetScale(0.5f, 0.5f, 0.5f);
    coin.isStatic = true;
    coin.m_isTrigger = true;
    coin.AddCollider("coin_trigger", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f }, true);
    m_gameObjects.push_back(coin);

    m_player.Initialize(&m_cubeModel);

    m_physics.AddDynamicObject(&m_player);
    for (auto& obj : m_terrain)
    {
        m_physics.AddStaticObject(&obj);
    }
    for (auto& obj : m_gameObjects) 
    {
        m_physics.AddStaticObject(&obj);
    }

   
}

void GameInstance::Finalize() 
{

}