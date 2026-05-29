Texture2D tex : register(t0); // 読み込んだ画像
Texture2D toon : register(t1);
SamplerState samp : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR; // 2. 頂点色
    float2 uv : TEXCOORD; // 3. UV
    float3 normal : NORMAL; // 4. 法線
};

cbuffer MaterialBuffer : register(b2)
{
    int useTexture;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = tex.Sample(samp, input.uv);
    float4 baseColor;

    // lengthによる判定ではなく、C++側から送られる useTexture フラグを使う
    if (useTexture == 0)
    {
        baseColor = input.color;
    }
    else
    {
        baseColor = texColor;
    }

    clip(baseColor.a - 0.05f);

    float3 lightDir = normalize(float3(1, -1, 1));
    float diffuse = dot(normalize(input.normal), -lightDir);
    //float2 toonUV = float2(0.5f, 1.0f - (diffuse * 0.5f + 0.5f));
    //float4 shadowFactor = toon.Sample(samp, toonUV);

    //return baseColor * (shadowFactor * 0.5f + 0.5f);

    float halfLambert = diffuse * 0.5f + 0.5f;

    float shadow = smoothstep(0.49f, 0.51f, halfLambert);

    shadow = shadow * 0.4f + 0.6f;
    
    return baseColor * shadow;
}