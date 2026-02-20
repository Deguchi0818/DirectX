cbuffer ConstantBuffer : register(b0)
{
    matrix wvp; // World-View-Projection 行列
};

cbuffer BoneBuffer : register(b1)
{
    matrix gBoneMatrices[128];
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color : COLOR;
    uint4 indices : BLENDINDICES;
    float4 weights : BLENDWEIGHTS;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    matrix worldViewProj;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // --- スキニング計算 ---
    float4 localPos = float4(input.pos, 1.0f);
    float4 skinnedPos = float4(0, 0, 0, 0);

    // 4つのボーンの影響を重み付けして合計する
    for (int i = 0; i < 4; i++)
    {
        // 重みが0より大きい場合だけ計算
        if (input.weights[i] > 0)
        {
            // 頂点を各ボーンの行列で変換し、重みを掛ける
            skinnedPos += mul(localPos, gBoneMatrices[input.indices[i]]) * input.weights[i];
        }
    }
    // 最後に w を 1 に固定（計算上必要）
    skinnedPos.w = 1.0f;
    // --------------------------
    
    // 頂点座標に行列を掛け算して動かす！
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    output.color = input.color;
    return output;
}