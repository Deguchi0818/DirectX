Texture2D tex : register(t0); // 読み込んだ画像
Texture2D toon : register(t1);
SamplerState samp : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // テクスチャから色をサンプリング
    float4 texColor = tex.Sample(samp, input.uv);

    float4 baseColor;
    if (texColor.a < 0.001f && length(texColor.rgb) < 0.001f)
    {
        baseColor = input.color;
    }
    else
    {
        baseColor = texColor;
        clip(baseColor.a - 0.1f);
    }

    // 影の計算
    float3 lightDir = normalize(float3(1, -1, 1));
    float diffuse = dot(normalize(input.normal), -lightDir);
    
    // 影パレット(toon.png)から色を拾う
    // diffuseを0.0～1.0の範囲にして、トゥーンテクスチャをサンプリング
    float2 toonUV = float2(0.5f, 1.0f - (diffuse * 0.5f + 0.5f));
    float4 shadowFactor = toon.Sample(samp, toonUV);

    // 最終的な色を出力
    return baseColor * shadowFactor;
}