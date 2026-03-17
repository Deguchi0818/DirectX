#pragma pack_matrix(row_major)

cbuffer ConstantBuffer : register(b0)
{
    matrix wvp; // World-View-Projection 行列
};

cbuffer BoneBuffer : register(b1)
{
    matrix gBoneMatrices[256];
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    uint4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHTS;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

//cbuffer ConstantBuffer : register(b0)
//{
//    matrix worldViewProj;
//}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float totalWeight = input.boneWeights.x + input.boneWeights.y +
                        input.boneWeights.z + input.boneWeights.w;

    float4x4 skinMatrix;

    // 重みが設定されているかチェック（0より大きい場合だけ計算）
    if (totalWeight > 0.0f)
    {
        skinMatrix =
            gBoneMatrices[input.boneIndices.x] * input.boneWeights.x +
            gBoneMatrices[input.boneIndices.y] * input.boneWeights.y +
            gBoneMatrices[input.boneIndices.z] * input.boneWeights.z +
            gBoneMatrices[input.boneIndices.w] * input.boneWeights.w;
    }
    else
    {
        // 重みがない（地面やブロック）場合は、何もしない行列（単位行列）にする
        skinMatrix = float4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    
    
    // 座標変換
    float4 localPos = mul(float4(input.pos, 1.0f), skinMatrix);
    output.pos = mul(localPos, wvp);
    
    output.normal = mul(input.normal, (float3x3) skinMatrix);
    output.color = input.color;
    output.uv = input.uv;

    return output;
}