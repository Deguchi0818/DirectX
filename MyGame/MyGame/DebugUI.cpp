#include "DebugUI.h"
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include "Model.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

bool DebugUI::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hWnd)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(device, context);

    return m_debugRenderer.Initialize(device);
}

void DebugUI::Finalize()
{
    m_debugRenderer.Finalize();
}

void DebugUI::BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::BuildWindows(Scene& scene, Camera& camera, float deltaTime)
{
    Player& player = scene.GetPlayer();
    Model* pModel = player.pModel;

    static float timer = 0.0f;
    timer += deltaTime;

    ImGui::Begin("Debug Menu");
    ImGui::Text("Player Settings");
    ImGui::SliderFloat("Jump Power", &player.GetJumpPower(), 0.0f, 20.0f);
    ImGui::SliderFloat("Move Speed", &player.GetMoveSpeed(), 0.0f, 20.0f);

    if (pModel)
    {
        static int selectedBone = 10;
        ImGui::SliderInt("Select Bone ID", &selectedBone, 0, (int)pModel->m_bones.size() - 1);
    }

    ImGui::SliderFloat("Mouse Sensitivity", &camera.GetSensitivity(), 0.0001f, 0.01f);
    ImGui::SliderFloat("Right Stick Sensitivity", &camera.GetRightStickSensitivity(), 0.0001f, 0.1f);

    ImGui::Separator();
    ImGui::Text("[Animation Debug]");
    if (pModel)
    {
        ImGui::Text("Model Bone Count: %d", (int)pModel->m_bones.size());
        ImGui::Text("Animation Count: %d", (int)pModel->m_animations.size());
    }
    ImGui::Text("Bone Matrices: %d", (int)scene.GetSkinMatrices().size());
    ImGui::Text("Timer: %.2f", timer);
    ImGui::End();

    if (m_isDebugMode)
    {
        ImGui::Begin("Physics Debug");
        if (player.m_hitHead)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "HEAD CRASH!");
        }
        if (ImGui::Button("Reset Flag")) player.m_hitHead = false;
        ImGui::End();
    }
}

void DebugUI::RenderImGui()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void DebugUI::DrawColliders(ID3D11DeviceContext* context, Scene& scene,
    const MyMatrix4x4& view, const MyMatrix4x4& proj)
{
    if (!m_isDebugMode) return;

    scene.GetPlayer().m_showCollider = true;
    scene.GetSword().m_showCollider = true;

    // 1つのオブジェクトのコライダーを線として登録する
    auto AddColliders = [&](GameObject& obj)
        {
            if (!obj.m_showCollider) return;

            const MyMatrix4x4& colMat = obj.transform.GetWorldMatrix();

            for (const auto& col : obj.m_colliders)
            {
                if (col.type == ColliderType::AABB)
                {
                    m_debugRenderer.AddAABB(col.GetWorldAABB(colMat), { 0.0f, 1.0f, 0.0f, 1.0f });
                }
                else if (col.type == ColliderType::Sphere)
                {
                    m_debugRenderer.AddSphere(col.GetWorldSphere(colMat), { 1.0f, 0.0f, 0.0f, 1.0f });
                }
                else if (col.type == ColliderType::Capsule)
                {
                    m_debugRenderer.AddCapsule(col.GetWorldCapsule(colMat), { 1.0f, 0.0f, 0.0f, 1.0f });
                }
            }
        };

    AddColliders(scene.GetPlayer());
    AddColliders(scene.GetSword());

    for (auto& obj : scene.GetGameObjects()) AddColliders(*obj);
    for (auto& obj : scene.GetTerrain())     AddColliders(*obj);

    MyMatrix4x4 viewProj = MyMatrix4x4::Multiply(view, proj);
    m_debugRenderer.Render(context, ToXM(viewProj));
}