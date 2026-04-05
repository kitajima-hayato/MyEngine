#include "Object3dInstancing.hlsli"
struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;

    // slot1（インスタンスVB）から来る想定
    float4 iWorld0 : TEXCOORD1;
    float4 iWorld1 : TEXCOORD2;
    float4 iWorld2 : TEXCOORD3;
    float4 iWorld3 : TEXCOORD4;

    float4 iWvp0 : TEXCOORD5;
    float4 iWvp1 : TEXCOORD6;
    float4 iWvp2 : TEXCOORD7;
    float4 iWvp3 : TEXCOORD8;

    float4 iColor : COLOR0;
};


ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);


VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4x4 world = float4x4(
        input.iWorld0,
        input.iWorld1,
        input.iWorld2,
        input.iWorld3);

    float4x4 wvp = float4x4(
        input.iWvp0,
        input.iWvp1,
        input.iWvp2,
        input.iWvp3);

    output.position = mul(input.position, wvp);
    output.texcoord = input.texcoord;

    output.normal = normalize(mul(input.normal, (float3x3) world));
    output.worldPosition = mul(input.position, world).xyz;

    output.color = input.iColor;
    return output;
}

