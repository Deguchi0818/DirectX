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

    CreateScene();

    m_camera.GetTransform().SetPosition(0.0f, 2.0f, -5.0f);

    m_lastTime = std::chrono::high_resolution_clock::now();

    Input::Initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(m_graphics.GetDevice(), m_graphics.GetContext());

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
    static float timer = 0.0f;
    timer += m_deltaTime;

    std::vector<DirectX::XMMATRIX> animMatrices;
    std::vector<bool> hasAnim;
    std::string currentAnim = m_player.GetCurrentAnimName();
    myModel.UpdateAnimation(currentAnim, timer, animMatrices, hasAnim);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ここにデバッグメニューの内容を書く
    ImGui::Begin("Debug Menu");
    ImGui::Text("Player Settings");
    // ジャンプ力や移動速度をスライダーで調整できるようにする
    ImGui::SliderFloat("Jump Power", &m_player.GetJumpPower(), 0.0f, 20.0f);
    ImGui::SliderFloat("Move Speed", &m_player.GetMoveSpeed(), 0.0f, 20.0f);
    static int selectedBone = 10; // 動かしたいボーンの番号
    ImGui::SliderInt("Select Bone ID", &selectedBone, 0, (int)myModel.m_bones.size() - 1);
    ImGui::SliderFloat("Mouse Sensitivity", &m_camera.GetSensitivity(), 0.0001f, 0.01f);
    ImGui::SliderFloat("Right Stick Sensitivity", &m_camera.GetRightStickSensitivity(), 0.0001f, 0.1f);
    
    ImGui::Separator();
    ImGui::Text("[Animation Debug]");
    ImGui::Text("Model Bone Count: %d", (int)myModel.m_bones.size());
    ImGui::Text("Animation Count: %d", (int)myModel.m_animations.size());

    int animBoneCount = 0;
    for (bool b : hasAnim) {
        if (b) animBoneCount++;
    }
    ImGui::Text("Animated Bones: %d", animBoneCount);
    ImGui::Text("Timer: %.2f", timer);
    ImGui::End();

    if (m_isDebugMode) 
    {
        ImGui::Begin("Physics Debug");
        if (m_player.m_hitHead) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "HEAD CRASH!"); // 赤文字で表示
        }
        // 確認が終わったらフラグを戻す処理など
        if (ImGui::Button("Reset Flag")) m_player.m_hitHead = false;
        ImGui::End();
    }

    // 描画開始
    m_graphics.BeginScene(0.1f, 0.2f, 0.4f, 1.0f);

    auto context = m_graphics.GetContext();



    // GPUに送る最終的な行列
    std::vector<DirectX::XMMATRIX> finalBones(256, DirectX::XMMatrixIdentity());
    // GPUに送る最終的な行列
    std::vector<DirectX::XMMATRIX> worldMatrices(256, DirectX::XMMatrixIdentity());

    // ボーンの階層順に計算するための再帰処理
    std::vector<bool> isCalculated(myModel.m_bones.size(), false);

    std::function<void(int)> CalcBoneMatrix = [&](int boneIdx) {
        if (isCalculated[boneIdx]) return; // 既に計算済みならスキップ

        int parentIdx = myModel.m_bones[boneIdx].parentIndex;

        // 親がいるなら、自分の計算の前に親を計算させる
        if (parentIdx != -1) {
            CalcBoneMatrix(parentIdx);
        }


        DirectX::XMVECTOR det;
        // 自分の初期姿勢
        DirectX::XMMATRIX globalBind = DirectX::XMMatrixInverse(&det, myModel.m_bones[boneIdx].offset);

        // 親からの相対的な位置（ローカル空間の初期姿勢）を計算
        DirectX::XMMATRIX localBind;
        if (parentIdx != -1) 
        {
            localBind = globalBind * myModel.m_bones[parentIdx].offset;
        }
        else {
            localBind = globalBind;
        }
        DirectX::XMMATRIX newLocal;
        if (hasAnim[boneIdx])
        {
            if (parentIdx != -1)
            {
                // 子ボーン：位置（Trans）は「Tポーズ（localBind）」から、回転（Rot）は「アニメ（animMatrices）」から！
                DirectX::XMVECTOR bindScale, bindRot, bindTrans;
                DirectX::XMMatrixDecompose(&bindScale, &bindRot, &bindTrans, localBind);

                DirectX::XMVECTOR animScale, animRot, animTrans;
                DirectX::XMMatrixDecompose(&animScale, &animRot, &animTrans, animMatrices[boneIdx]);

                animRot = DirectX::XMQuaternionNormalize(animRot); // 爆発を防ぐ安全装置

                // 綺麗な関節位置を保ったまま、回転だけを適用して合成！
                newLocal = DirectX::XMMatrixScalingFromVector(bindScale) *
                    DirectX::XMMatrixRotationQuaternion(animRot) *
                    DirectX::XMMatrixTranslationFromVector(bindTrans);
            }
            else
            {
                // ルートボーン（一番親の腰など）：歩く・走るの移動量が含まれるのでそのまま使う
                newLocal = animMatrices[boneIdx];
            }
        }
        else 
        {
            newLocal = localBind;
        }

        if (parentIdx != -1) {
            worldMatrices[boneIdx] = newLocal * worldMatrices[parentIdx];
        }
        else {
            worldMatrices[boneIdx] = newLocal;
        }

        isCalculated[boneIdx] = true;
        };

    // 全てのボーンに対して再帰計算を実行
    for (int i = 0; i < (int)myModel.m_bones.size(); i++) 
    {
        CalcBoneMatrix(i);
    }

    // 最後にスキニング行列を確定させる
    for (int i = 0; i < (int)myModel.m_bones.size(); i++) 
    {
        finalBones[i] = myModel.m_bones[i].offset * worldMatrices[i];
    }


    // GPUへ転送
    m_baseShader.UpdateBones(context, finalBones);
    m_baseShader.Bind(context);

    ImGui::Begin("Debug Menu");
    ImGui::Text("Model Bone Count: %d", (int)myModel.m_bones.size());
    ImGui::End();

    // 行列の準備
    auto view = m_camera.GetViewMatrix();
    float aspect = 1280.0f / 720.0f;
    auto proj = m_camera.GetProjectionMatrix(aspect);

    // 全てのオブジェクトを描画するループ
    for (auto& obj : m_gameObjects)
    {
        obj.Draw(context, &m_baseShader, m_constantBuffer.Get(), view, proj);
    }

    for (auto& obj : m_terrain)
    {
        obj.Draw(context, &m_baseShader, m_constantBuffer.Get(), view, proj);
    }

    m_player.Draw(context, &m_baseShader, m_constantBuffer.Get(), view, proj);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (m_isDebugMode)
    {
        // プレイヤーのコライダーを常に表示する設定
        m_player.m_showCollider = true;

        // コライダーを DebugRenderer に登録するための便利なラムダ式（関数内関数）
        auto AddCollidersToDebug = [&](GameObject& obj) {
            if (!obj.m_showCollider) return;

            for (const auto& col : obj.m_colliders)
            {
                if (col.type == ColliderType::AABB) {
                    AABB worldAABB = col.GetWorldAABB(obj.transform.GetPosition(), obj.transform.GetScale());
                    m_debugRenderer.AddAABB(worldAABB, { 0.0f, 1.0f, 0.0f, 1.0f }); // 緑色
                }
                else if (col.type == ColliderType::Sphere) {
                    Sphere worldSphere = col.GetWorldSphere(obj.transform.GetPosition(), obj.transform.GetScale());
                    m_debugRenderer.AddSphere(worldSphere, { 1.0f, 0.0f, 0.0f, 1.0f }); // 赤色
                }
                else if (col.type == ColliderType::Capsule)
                {
                    Capsule worldCapsule = col.GetWorldCapsule(obj.transform.GetPosition(), obj.transform.GetScale());
                    m_debugRenderer.AddCapsule(worldCapsule, { 1.0f, 0.0f, 0.0f, 1.0f }); // 赤色で描画
                }
            }
            };

        // プレイヤーのコライダーを登録
        AddCollidersToDebug(m_player);

        // ゲームオブジェクトのコライダーを登録
        for (auto& obj : m_gameObjects) {
            // テスト用に全部表示したい場合はコメントアウトを外す
            // obj.m_showCollider = true; 
            AddCollidersToDebug(obj);
        }

        // 地形(Terrain)のコライダーを登録
        for (auto& obj : m_terrain) {
            // obj.m_showCollider = true;
            AddCollidersToDebug(obj);
        }

        // 登録したすべての線を GPU に送って描画する！
        MyMatrix4x4 viewProj = MyMatrix4x4::Multiply(view, proj);
        DirectX::XMMATRIX xmViewProj = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)viewProj.m);
        m_debugRenderer.Render(context, xmViewProj);
    }

    //描画終了
    m_graphics.EndScene();
}

bool GameInstance::CreateAssets(ID3D11Device* device) 
{
    if (!m_baseShader.Load(device, L"VertexShader.hlsl", L"PixelShader.hlsl")) return false;

    myModel.LoadFromFile(device, "Asset/Idle.fbx", "Idle");

    myModel.LoadAnimation("Running", "Asset/Running.fbx");
    myModel.LoadAnimation("Jump","Asset/Jump.fbx");

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> toonSRV;
    HRESULT hr = DirectX::CreateWICTextureFromFile(device, L"Asset/test/toon.png", nullptr, &toonSRV);

    if (SUCCEEDED(hr)) {
        myModel.SetToonTexture(toonSRV);
        m_cubeModel.SetToonTexture(toonSRV);   // 立方体
        m_planeModel.SetToonTexture(toonSRV);  // 地面
        m_playerModel.SetToonTexture(toonSRV);  // 地面
    }

    myModel.SetToonTexture(toonSRV);

    m_cubeModel.CreateCube(device, 1.0f, { 0.0f, 0.5f, 1.0f, 1.0f });
    m_playerModel.CreateCube(device, 1.0f, { 0.5f, 0.0f, 0.0f, 1.0f });
    m_planeModel.CreatePlane(device, 1.0f, 1.0f, { 0.0f, 0.5f, 0.0f, 1.0f });

    // 定数バッファ作成
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBufferData);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;          // 動的に変更可能にする
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);
    m_debugRenderer.Initialize(device);

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
    floor.transform.SetScale(50.0f, 1.0f, 50.0f);
    floor.isStatic = true;
    floor.AddCollider("floor_main", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 0.01f, 1.0f });
    m_terrain.push_back(floor);

    GameObject wall;
    wall.pModel = &m_cubeModel;
    wall.transform.SetPosition(5.0f, 1.0f, 0.0f);
    wall.transform.SetScale(1.0f, 7.0f, 5.0f);
    wall.isStatic = true;
    wall.m_isTrigger = false;
    wall.AddCollider("wall_main", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    wall.m_friction = 1.0f;
    m_gameObjects.push_back(wall);

    GameObject block;
    block.pModel = &m_cubeModel;
    block.transform.SetPosition(0.0f, 3.5f, 0.0f);
    block.transform.SetScale(1.0f, 1.0f, 1.0f);
    block.isStatic = false;
    block.m_useGravity = true;
    block.m_isTrigger = false;
    block.m_showCollider = true;
    block.AddCollider("block", ColliderType::AABB, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
    m_gameObjects.push_back(block);

    GameObject coin;
    coin.pModel = &m_cubeModel;
    coin.transform.SetPosition(-3.0f, 1.0f, 2.0f);
    coin.transform.SetScale(0.5f, 0.5f, 0.5f);
    coin.isStatic = true;
    coin.m_isTrigger = true;
    coin.m_showCollider = true;
    auto& coinCol = coin.AddCollider("coin", ColliderType::Sphere, { 0,0,0 }, { 1,1,1 });
    coinCol.radius = 0.5f;
    coinCol.isTrigger = true;
    m_gameObjects.push_back(coin);


    //GameObject character;
    //character.pModel = &myModel; // さきほど読み込んだモデルをセット
    //character.transform.SetPosition(0.0f, 1.0f, 0.0f); // 座標を設定
    //character.transform.SetRotation(0.0f, 0.0f, 0.0);
    //character.transform.SetScale(1.0f, 1.0f, 1.0f);    // サイズを調整

    //m_gameObjects.push_back(character);

    m_player.Initialize(&myModel);

    m_physics.AddDynamicObject(&m_player);
    for (auto& obj : m_terrain)
    {
        m_physics.AddStaticObject(&obj);
    }
    for (auto& obj : m_gameObjects)
    {
        if (obj.isStatic)
        {
            m_physics.AddStaticObject(&obj);  // コインなどはこっち
        }
        else
        {
            m_physics.AddDynamicObject(&obj); // 木箱などはこっち
        }
    }

   
}

void GameInstance::Finalize() 
{
    CoUninitialize();
}