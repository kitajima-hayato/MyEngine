#include "Object3dInstancing.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shiniess;
};
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
struct DirectionalLight
{
    float32_t4 color; //ライトの色
    float32_t3 direction; //ライトの向き
    float intensity; //輝度
};

struct Camera
{
    float32_t3 worldPosition;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

Texture2D<float4> gTexture : register(t0); //SRVのregisterはt
TextureCube<float4> gCubeTexture : register(t1); // 環境マッピング用のキューブテクスチャ

SamplerState gSampler : register(s0); //Samplerのregisterはs

//TextureCube<float32_t4> gEnvironmentTexture : register(t1);
    



PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    
 
    if (gMaterial.enableLighting != 0)
    {
        if (gMaterial.enableLighting == 1)
        {
            //通常
            float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
            float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
            
            float RdotE = dot(reflectLight, toEye);
            float specularPow = pow(saturate(RdotE), gMaterial.shiniess);
            
            float32_t3 diffuse = gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            
            float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
            
            output.color.rgb = diffuse + specular;
            
            output.color.a = gMaterial.color.a * textureColor.a;
            
            
            output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
        }
        if (gMaterial.enableLighting == 2)
        {
            //HalfLambert
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
        }
        if (gMaterial.enableLighting == 3)
        {
            /// 環境マッピング
            float3 cameraTopPosition = normalize(input.worldPosition - gCamera.worldPosition);
            float3 reflectedVector = reflect(cameraTopPosition, normalize(input.normal));
            float4 environmentColor = gCubeTexture.Sample(gSampler, reflectedVector);
            
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            float3 baseColor = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            
            
            output.color.rgb = lerp(baseColor, environmentColor.rgb, gMaterial.enableLighting);
            output.color.a = gMaterial.color.a * textureColor.a;
        }
           
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    output.color *= input.color;
    return output;
}

