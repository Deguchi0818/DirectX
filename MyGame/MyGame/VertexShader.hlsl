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
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    uint4 indices : BLENDINDICES;
    float4 weights : BLENDWEIGHTS;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

cbuffer ConstantBuffer : register(b0)
{
    matrix worldViewProj;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // 座標変換
    output.pos = mul(float4(input.pos, 1.0f), wvp);
    
    output.color = input.color;
    output.uv = input.uv; // ★データを渡す
    output.normal = input.normal; // ★データを渡す

    return output;
}