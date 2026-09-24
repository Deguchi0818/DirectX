#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>
#include <vector>
#include <chrono>
#include <memory>

#include "GameObject.h"
#include "Camera.h"
#include "Graphics.h"
#include "Shader.h"
#include "Model.h"
#include "Scene.h"
#include "Player.h"
#include "PhysicsEngine.h"
#include "DebugRenderer.h"
#include "ResourceManager.h"
#include "Weapon.h"
#include "DebugUI.h"


class GameInstance
{
public:
	GameInstance();
	~GameInstance();

	bool Initialize(HWND hWnd, int width, int height);
	void Finalize();
	void Update();
	void Render();
	void Run();


private:


	bool CreateAssets(ID3D11Device* device); // モデルやシェーダーの作成

	void UpdateSystem();
	void UpdateTransforms();
	void UpdateAnimation();

	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_constantBuffer;

	ResourceManager m_resourceManager;

	Scene m_scene;
	DebugUI m_debugUI;

	Graphics m_graphics;
	Shader m_baseShader;
	Camera m_camera;
	PhysicsEngine m_physics;

	std::chrono::high_resolution_clock::time_point m_lastTime;
	float m_deltaTime = 0.0f;

	std::vector<DirectX::XMMATRIX> m_boneWorlds;    // ボーンのワールド行列
	std::vector<DirectX::XMMATRIX> m_skinMatrices;	// シェーダーへ送るスキニング行列
};

