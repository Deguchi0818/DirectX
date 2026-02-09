#pragma once
#include "Transform.h"
#include "Common.h"
#include "Mesh.h"
#include "Model.h"

#include <d3d11.h>
#include <wrl/client.h>

class GameObject
{
public:
	Transform transform;
    Model* pModel;

    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* cb, const MyMatrix4x4& view, const MyMatrix4x4& proj) 
    {
        if (!pModel) return; // メッシュがない場合は何もしない

        // 行列の計算
        transform.UpdateMatrix();
        MyMatrix4x4 wvp = MyMatrix4x4::Multiply(transform.GetWorldMatrix(), view);
        wvp = MyMatrix4x4::Multiply(wvp, proj);

        // 定数バッファの更新
        ConstantBufferData cbData;
        MyMatrix4x4 finalMat = wvp.Transpose();
        memcpy(&cbData.wvp, &finalMat, sizeof(MyMatrix4x4));
        context->UpdateSubresource(cb, 0, nullptr, &cbData, 0, 0);

        // シェーダーに定数バッファをセット
        context->VSSetConstantBuffers(0, 1, &cb);

        pModel->Draw(context);
    }
};

