#include "Object3DCommon.h"
#include "Logger.h"

using namespace Engine;

Object3DCommon* Object3DCommon::instance = nullptr;


void Object3DCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	CreateGraphicsPipeLine();
}

void Object3DCommon::DrawSettingCommon()
{
	// ルートシグネチャのセット
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// グラフィックスパプラインのセット
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	// プリミティブトポロジーのセット
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3DCommon::Finalize()
{
	// 既に解放済みなら何もしない
	if (!dxCommon_) { return; }

	graphicsPipelineState.Reset();
	rootSignature.Reset();
	vertexShaderBlob.Reset();
	pixelShaderBlob.Reset();

	defaultCamera = nullptr;
	dxCommon_ = nullptr;
}


void Object3DCommon::CreateRootSignature()
{
#pragma region RootParameter
	//RootSignature作成
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
#pragma region DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange = {};
	descriptorRange.BaseShaderRegister = 0;//0から始まる
	descriptorRange.NumDescriptors = 1;//数は１つ
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算

	// 環境マップ
	D3D12_DESCRIPTOR_RANGE descriptorRangeSkyBox = {};
	descriptorRangeSkyBox.BaseShaderRegister = 1;
	descriptorRangeSkyBox.NumDescriptors = 1;
	descriptorRangeSkyBox.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeSkyBox.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算

	#pragma endregion

	//RootParameter作成。複数設定できるので配列。今回は結果が１つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParameters[6] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//
	rootParameters[1].Descriptor.ShaderRegister = 0;//

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange;//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;//Tableで利用する数

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//CBVで使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号１を使う

	// 環境マップ
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[4].DescriptorTable.pDescriptorRanges = &descriptorRangeSkyBox;//Tableの中身の配列を指定
	rootParameters[4].DescriptorTable.NumDescriptorRanges = 1;//Tableで利用する数

	//カメラ
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[5].Descriptor.ShaderRegister = 2;
	
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//
#pragma endregion



	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// ルートシグネチャのシリアライズ結果を格納するBlob
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	// エラーメッセージが発生した場合に格納するBlob
	Microsoft::WRL::ComPtr <ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

}

void Object3DCommon::CreateGraphicsPipeLine()
{
	CreateRootSignature();

	inputElementDescs[0] = {
		.SemanticName = "POSITION",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
		.InputSlot = 0,
		.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
		.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	};
	inputElementDescs[1] = {
		.SemanticName = "TEXCOORD",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32_FLOAT,
		.InputSlot = 0,
		.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
		.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	};
	inputElementDescs[2] = {
		.SemanticName = "NORMAL",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32B32_FLOAT,
		.InputSlot = 0,
		.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
		.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	};

	// World (TEXCOORD1-4)
	inputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[4] = { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[5] = { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[6] = { "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	// WVP (TEXCOORD5-8)
	inputElementDescs[7] = { "TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[8] = { "TEXCOORD", 6, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[9] = { "TEXCOORD", 7, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	inputElementDescs[10] = { "TEXCOORD", 8, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	// Color (COLOR0)
	inputElementDescs[11] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	inputLayoutDescs.pInputElementDescs = inputElementDescs;
	inputLayoutDescs.NumElements = _countof(inputElementDescs);


	// レンダーターゲットの書き込みマスクを設定。全ての色チャンネルに書き込みを許可
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;


	// カリングモードを設定
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// ポリゴンのフィルモードを設定。ポリゴンの塗り潰し
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//　頂点シェーダーのコンパイル結果を格納するBlob
	vertexShaderBlob = dxCommon_->CompileShader(
		L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	// ピクセルシェーダーのコンパイル結果を格納するBlob
	pixelShaderBlob = dxCommon_->CompileShader(
		L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);


	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

#pragma region GraphicsPipelineState

	// グラフィックスパイプラインのステートオブジェクトのデスクリプタを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	// インプットレイアウトの設定
	graphicsPipelineStateDesc.InputLayout = inputLayoutDescs;
	// シェーダーの設定
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };//
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };//
	// ブレンドステートの設定
	graphicsPipelineStateDesc.BlendState = blendDesc;
	// ラスタライザの設定
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// レンダーターゲットの設定
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// プリミティブとポロ時の設定
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
#pragma endregion

}



Object3DCommon* Object3DCommon::GetInstance() {
	if (instance == nullptr) {
		instance = new Object3DCommon();
	}
	return instance;
}

void Object3DCommon::DeleteInstance() 
{
	if (instance) {
		instance->Finalize();
		delete instance;
		instance = nullptr;
	}
}