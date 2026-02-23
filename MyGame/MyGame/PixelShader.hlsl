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

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = tex.Sample(samp, input.uv);
    float4 baseColor;

    // テクスチャがある場所はテクスチャ、ない場所は頂点色（input.color）
    if (texColor.a < 0.01f)
    {
        baseColor = input.color;
    }
    else
    {
        baseColor = texColor;
        // ★ 修正：透明になりすぎるのを防ぐため、clipを無効化するか値を下げる
        clip(baseColor.a - 0.05f); 
    }

    // 影の計算
    float3 lightDir = normalize(float3(1, -1, 1));
    float diffuse = dot(normalize(input.normal), -lightDir);
    float2 toonUV = float2(0.5f, 1.0f - (diffuse * 0.5f + 0.5f));
    float4 shadowFactor = toon.Sample(samp, toonUV);

    // ★ 修正：影を計算しつつ、暗くなりすぎないように最低限の明るさを保証（0.5f + ...）
    return baseColor * (shadowFactor * 0.5f + 0.5f);
}