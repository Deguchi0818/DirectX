#pragma once
#include "Transform.h"
#include <DirectXMath.h>
#include <vector>

class GameObject;

// --------------------------------------------------------
// BoneAttachment
//   モデルのボーンにオブジェクトを取り付ける仕組み。
//   武器に限らず、盾・帽子・エフェクトなども同じ形で扱える。
//
//   内部にボーンの姿勢を保持するノードを持ち、
//   取り付けたオブジェクトの親として設定する。
//   これにより world = 対象のlocal * ボーンのworld が成立する。
// --------------------------------------------------------
class BoneAttachment
{
public:
    // 取り付ける(対象の親をこのノードに設定する)
    void Attach(GameObject* target, int boneIndex);

    // ボーンの姿勢を反映し、対象の行列を確定させる
    //   ownerWorld : ボーンを持つモデル側のワールド行列(プレイヤーなど)
    //   boneWorlds : Model::CalculateBoneMatrices が出力したボーン行列
    void Update(const MyMatrix4x4& ownerWorld,
        const std::vector<DirectX::XMMATRIX>& boneWorlds);

    bool IsValid() const { return m_target != nullptr && m_boneIndex >= 0; }

private:
    GameObject* m_target = nullptr;   // 所有しない。参照のみ。
    int m_boneIndex = -1;

    Transform m_boneTransform;        // ボーンの姿勢を保持するノード
};