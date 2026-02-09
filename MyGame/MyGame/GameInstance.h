#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxmath.h>
#include <vector>

#include "GameObject.h"
#include "Camera.h"
#include "Graphics.h"
#include "Shader.h"
#include "Model.h"

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

	Microsoft::WRL::ComPtr<ID3D11Buffer>           m_constantBuffer;

	std::vector<GameObject> m_gameObjects;

	Model m_cubeModel;
	Model m_planeModel;

	Graphics m_graphics;
	Shader m_baseShader;
	Camera m_camera;
};

