#include "Object3DInstancingCommon.h"
#include "Logger.h"

using namespace Engine;

Object3DInstancingCommon* Object3DInstancingCommon::instance_ = nullptr;

Object3DInstancingCommon* Object3DInstancingCommon::GetInstance()
{
    if (!instance_) instance_ = new Object3DInstancingCommon();
    return instance_;
}

void Object3DInstancingCommon::DeleteInstance()
{
    if (instance_) {
        instance_->Finalize();
        delete instance_;
        instance_ = nullptr;
    }
}

void Object3DInstancingCommon::Initialize(DirectXCommon* dxCommon)
{
    dxCommon_ = dxCommon;
    CreateGraphicsPipeLine();
}

void Object3DInstancingCommon::DrawSettingCommon()
{
    dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
    dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3DInstancingCommon::Finalize()
{
    if (!dxCommon_) return;
    graphicsPipelineState_.Reset();
    rootSignature_.Reset();
    vertexShaderBlob_.Reset();
    pixelShaderBlob_.Reset();
    defaultCamera_ = nullptr;
    dxCommon_ = nullptr;
}

void Object3DInstancingCommon::CreateRootSignature()
{
    descriptionRootSignature.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE rangeTex{};
    rangeTex.BaseShaderRegister = 0;
    rangeTex.NumDescriptors = 1;
    rangeTex.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    rangeTex.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE rangeSky{};
    rangeSky.BaseShaderRegister = 1;
    rangeSky.NumDescriptors = 1;
    rangeSky.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    rangeSky.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER root[6]{};

    // [0] Material b0 (PS)
    root[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    root[0].Descriptor.ShaderRegister = 0;

    // [1] (互換用ダミー：WVPの枠)
    root[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    root[1].Descriptor.ShaderRegister = 0;

    // [2] Texture t0 (PS)
    root[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    root[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    root[2].DescriptorTable.pDescriptorRanges = &rangeTex;
    root[2].DescriptorTable.NumDescriptorRanges = 1;

    // [3] Light b1 (PS)
    root[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    root[3].Descriptor.ShaderRegister = 1;

    // [4] Skybox t1 (PS)
    root[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    root[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    root[4].DescriptorTable.pDescriptorRanges = &rangeSky;
    root[4].DescriptorTable.NumDescriptorRanges = 1;

    // [5] Camera b2 (PS)
    root[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    root[5].Descriptor.ShaderRegister = 2;

    descriptionRootSignature.pParameters = root;
    descriptionRootSignature.NumParameters = _countof(root);

    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    descriptionRootSignature.pStaticSamplers = &sampler;
    descriptionRootSignature.NumStaticSamplers = 1;

    Microsoft::WRL::ComPtr<ID3DBlob> sig, err;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &err);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(err->GetBufferPointer()));
        assert(false);
    }
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void Object3DInstancingCommon::CreateGraphicsPipeLine()
{
    CreateRootSignature();

    // slot0
    inputElementDescs_[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElementDescs_[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElementDescs_[2] = { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    // slot1 instance
    inputElementDescs_[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[4] = { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[5] = { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[6] = { "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[7] = { "TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[8] = { "TEXCOORD", 6, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[9] = { "TEXCOORD", 7, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[10] = { "TEXCOORD", 8, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
    inputElementDescs_[11] = { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

    inputLayoutDescs.pInputElementDescs = inputElementDescs_;
    inputLayoutDescs.NumElements = _countof(inputElementDescs_);

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    vertexShaderBlob_ = dxCommon_->CompileShader(L"resources/shaders/Object3dInstancing.VS.hlsl", L"vs_6_0");
    pixelShaderBlob_ = dxCommon_->CompileShader(L"resources/shaders/Object3dInstancing.PS.hlsl", L"ps_6_0");

    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
    pso.pRootSignature = rootSignature_.Get();
    pso.InputLayout = inputLayoutDescs;
    pso.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() };
    pso.PS = { pixelShaderBlob_->GetBufferPointer(),  pixelShaderBlob_->GetBufferSize() };
    pso.BlendState = blendDesc;
    pso.RasterizerState = rasterizerDesc;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.SampleDesc.Count = 1;
    pso.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pso.DepthStencilState = depthStencilDesc;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&graphicsPipelineState_));
    assert(SUCCEEDED(hr));
}