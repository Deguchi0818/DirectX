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
    // 1. 基本の色を決定する
    float4 baseColor = tex.Sample(samp, input.uv);

    // 画像が貼られていない（アルファ値がほぼ0）パーツは、地面などの頂点色を使う
    if (baseColor.a < 0.1f)
    {
        baseColor = input.color;
    }
    else
    {
        // キャラクターなどの画像があるパーツだけ、端っこを綺麗にするために切り抜く
        clip(baseColor.a - 0.05f);
    }

    // 2. 影の計算
    float3 lightDir = normalize(float3(1, -1, 1));
    float diffuse = dot(normalize(input.normal), -lightDir);
    
    // 影パレット(toon.png)から色を拾う
    float2 toonUV = float2(0.5f, 1.0f - (diffuse * 0.5f + 0.5f));
    float4 shadowFactor = toon.Sample(samp, toonUV);

    // 3. 最終的な色を出力（基本色 × 影の濃さ）
    return baseColor * shadowFactor;
}