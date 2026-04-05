#pragma once
#include "DirectXCommon.h"
#include "Game/Camera/Camera.h"
#include <wrl.h>
#include <d3d12.h>

class Object3DInstancingCommon
{
public:
    static Object3DInstancingCommon* GetInstance();
    static void DeleteInstance();

    void Initialize(Engine::DirectXCommon* dxCommon);
    void DrawSettingCommon();
    void Finalize();

    Engine::DirectXCommon* GetDxCommon() const { return dxCommon_; }
    void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }
    Camera* GetDefaultCamera() const { return defaultCamera_; }

private:
    void CreateRootSignature();
    void CreateGraphicsPipeLine();

private:
    Object3DInstancingCommon() = default;
    ~Object3DInstancingCommon() = default;
    Object3DInstancingCommon(const Object3DInstancingCommon&) = delete;
    Object3DInstancingCommon& operator=(const Object3DInstancingCommon&) = delete;

    static Object3DInstancingCommon* instance_;

    Camera* defaultCamera_ = nullptr;
    Engine::DirectXCommon* dxCommon_ = nullptr;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    D3D12_INPUT_LAYOUT_DESC inputLayoutDescs{};

    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

    D3D12_BLEND_DESC blendDesc{};
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

    D3D12_INPUT_ELEMENT_DESC inputElementDescs_[12]{};
};