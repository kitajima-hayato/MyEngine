#include "Particle.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer MaterialBuffer : register(b0)
{
    Material gMaterial;
}

Texture2D<float4> gTexture : register(t1);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float2 uv = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform).xy;
    float4 tex = gTexture.Sample(gSampler, uv);

    // 明るさ（黒地のテクスチャ用）
    float luminance = max(tex.r, max(tex.g, tex.b));

    const float cutStart = 0.03f;
    const float cutEnd = 0.15f;
    float keyAlpha = smoothstep(cutStart, cutEnd, luminance);

    float4 col = gMaterial.color * tex * input.color;
    

    // 黒抜きで作ったαを適用
    col.a *= keyAlpha;
     // Premultiplied Alpha：RGBにαを掛ける（黒フチ対策の本体）
    col.rgb *= col.a;

    if (col.a <= 0.001f)
    {
        discard;
    }

   

    output.color = col;
    return output;
}