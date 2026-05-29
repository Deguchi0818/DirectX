#include "Input.h"

// 静的メンバの実体定義
BYTE Input::m_currentStates[256] = { 0 };
BYTE Input::m_previousStates[256] = { 0 };

XINPUT_STATE Input::m_currentControllerState = { 0 };
XINPUT_STATE Input::m_previousControllerState = { 0 };
bool Input::m_isConnected = false;

void Input::Initialize() {
    memset(m_currentStates, 0, sizeof(m_currentStates));
    memset(m_previousStates, 0, sizeof(m_previousStates));

    memset(&m_currentControllerState, 0, sizeof(m_currentControllerState));
    memset(&m_previousControllerState, 0, sizeof(m_previousControllerState));
}

void Input::Update() {
    // 前の状態を保存
    memcpy(m_previousStates, m_currentStates, sizeof(m_previousStates));
    // 今の状態を取得
    GetKeyboardState(m_currentStates);

    // コントローラーの更新
    m_previousControllerState = m_currentControllerState;
    // 0番目（1台目）のコントローラーを取得
    DWORD dwResult = XInputGetState(0, &m_currentControllerState);
    m_isConnected = (dwResult == ERROR_SUCCESS);
}

bool Input::GetKey(int vKey) {
    // 最上位ビットが1なら押されている
    return (m_currentStates[vKey] & 0x80) != 0;
}

bool Input::GetKeyDown(int vKey) {
    // 今押されていて、前は押されていなかったら「押した瞬間」
    return ((m_currentStates[vKey] & 0x80) != 0) && ((m_previousStates[vKey] & 0x80) == 0);
}

bool Input::GetKeyUp(int vKey) {
    // 今押されていなくて、前は押されていたら「離した瞬間」
    return ((m_currentStates[vKey] & 0x80) == 0) && ((m_previousStates[vKey] & 0x80) != 0);
}

bool Input::GetButton(WORD button) {
    if (!m_isConnected) return false;
    return(m_currentControllerState.Gamepad.wButtons & button) != 0;
}

bool Input::GetButtonDown(WORD button) {
    if (!m_isConnected) return false;
    return((m_currentControllerState.Gamepad.wButtons & button) != 0) &&
        ((m_previousControllerState.Gamepad.wButtons & button) == 0);
}

bool Input::GetButtonUp(WORD button) {
    if (!m_isConnected) return false;
    return((m_currentControllerState.Gamepad.wButtons & button) == 0) &&
        ((m_previousControllerState.Gamepad.wButtons & button) != 0);
}

float Input::GetAxisX() {
    if (!m_isConnected) return 0.0f;    // 接続されていなければ 0

    float raw = (float)m_currentControllerState.Gamepad.sThumbLX;   // 生の値
	float deadzone = (float)XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;     // システム既定のデッドゾーン
    float maxValue = 32767.0f;

    // スティックの傾きがデッドゾーン以下なら、完全に 0 とみなす
    if (abs(raw) < deadzone) return 0.0f;

    // 傾いている方向（正か負か）を保存
    float sign = (raw > 0) ? 1.0f : -1.0f;
    // デッドゾーンの外側から最大値までの間を 0.0 ～ 1.0 にする
    float normalizedValue = (abs(raw) - deadzone) / (maxValue - deadzone);

    // 念のため 0.0～1.0 にクランプ
    if (normalizedValue > 1.0f) normalizedValue = 1.0f;
    if (normalizedValue < 0.0f) normalizedValue = 0.0f;

    return normalizedValue * sign;
}

float Input::GetAxisZ() {
    if (!m_isConnected) return 0.0f;
    
    float raw = (float)m_currentControllerState.Gamepad.sThumbLY;
    float deadzone = (float)XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    float maxValue = 32767.0f;
   
    if (abs(raw) < deadzone) return 0.0f;
    
    float sign = (raw > 0) ? 1.0f : -1.0f;
    float normalizedValue = (abs(raw) - deadzone) / (maxValue - deadzone);
   
    if (normalizedValue > 1.0f) normalizedValue = 1.0f;
    if (normalizedValue < 0.0f) normalizedValue = 0.0f;
    
    return normalizedValue * sign;
}

float Input::GetRightAxisX() {
    if (!m_isConnected) return 0.0f;

    float raw = (float)m_currentControllerState.Gamepad.sThumbRX;
    float deadzone = (float)XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
    float maxValue = 32767.0f;

    if (abs(raw) < deadzone) return 0.0f;

    float sign = (raw > 0) ? 1.0f : -1.0f;
    float normalizedValue = (abs(raw) - deadzone) / (maxValue - deadzone);

    if (normalizedValue > 1.0f) normalizedValue = 1.0f;
    if (normalizedValue < 0.0f) normalizedValue = 0.0f;

    return normalizedValue * sign;
}

float Input::GetRightAxisZ() {
    if (!m_isConnected) return 0.0f;

    float raw = (float)m_currentControllerState.Gamepad.sThumbRY;
    float deadzone = (float)XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
    float maxValue = 32767.0f;

    if (abs(raw) < deadzone) return 0.0f;

    float sign = (raw > 0) ? 1.0f : -1.0f;
    float normalizedValue = (abs(raw) - deadzone) / (maxValue - deadzone);

    if (normalizedValue > 1.0f) normalizedValue = 1.0f;
    if (normalizedValue < 0.0f) normalizedValue = 0.0f;

    return normalizedValue * sign;
}