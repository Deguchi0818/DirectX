#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>
#include <vector>
#include <chrono>

#include "GameObject.h"
#include "Camera.h"
#include "Graphics.h"
#include "Shader.h"
#include "Model.h"
#include "Player.h"
#include "PhysicsEngine.h"

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
	void CreateScene();                      // オブジェクトの配置と物理登録

	void UpdateSystem();

	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_constantBuffer;

	std::vector<GameObject> m_gameObjects;
	std::vector<GameObject> m_terrain;

	Model m_cubeModel;
	Model m_planeModel;

	Graphics m_graphics;
	Shader m_baseShader;
	Camera m_camera;
	Player m_player;
	PhysicsEngine m_physics;

	std::chrono::high_resolution_clock::time_point m_lastTime;
	float m_deltaTime = 0.0f;
};

