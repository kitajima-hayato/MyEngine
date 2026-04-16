#include "ParticleManager.h"
#include <Logger.h>
#include "MakeMatrix.h"
#include <TextureManager.h>
#include "WinAPI.h"
#include <numbers>

using namespace Engine;

ParticleManager* ParticleManager::instance = nullptr;
ParticleManager* ParticleManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new ParticleManager();
	}
	return instance;

}

void ParticleManager::DeleteInstance()
{
	if (instance) {
		instance->Finalize();
		delete instance;
		instance = nullptr;
	}

}

void ParticleManager::Finalize()
{
	// すでに解放済みなら何もしない（何度呼ばれても安全にする）
	if (!dxCommon) {
		return;
	}

	// 1) particleGroups を全解放（Mapした instancingResource を Unmap してから Reset）
	for (auto& [name, group] : particleGroups) {
		if (group.instancingResource) {
			group.instancingResource->Unmap(0, nullptr);
			group.instancingData = nullptr;
			group.instancingResource.Reset();
		}
	}
	particleGroups.clear();

	// 2) vertex/material を Unmap（Mapしたままだった）
	if (vertexResource) {
		vertexResource->Unmap(0, nullptr);
		vertexData = nullptr;
	}
	if (materialResource) {
		materialResource->Unmap(0, nullptr);
		materialData = nullptr;
	}

	// 3) D3D12オブジェクトを解放（RootSig/PSO/Resource）
	ringVertexBuffer.Reset();
	vertexResource.Reset();
	materialResource.Reset();
	graphicsPipelineState.Reset();
	rootSignature.Reset();

	// 4) 外部参照を切る（Framework側の寿命に依存しないように）
	camera = nullptr;
	srvManager = nullptr;
	dxCommon = nullptr;
}

ParticleManager::~ParticleManager()
{
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera)
{
	// メンバ変数の初期化
	this->dxCommon = dxCommon;
	this->srvManager = srvManager;
	// カメラの取得
	this->camera = camera;

	// ブレンドモードの設定
	blendMode = BlendMode::kBlendModeAdd;
	//blendMode = BlendMode::kBlendModeNormal;
	// ランダムエンジンの初期化
	InitializeRandomEngine();
	// パイプラインの生成
	CreatePipeline();
	// 頂点データの初期化(座標等)
	InitializeVertexData();
	// マテリアルの初期化
	InitializeMaterial();

	// リングエフェクトの初期化
	//CreateRingVertex();

	// シリンダーエフェクト
	//CreateCylinderVertex();
}

void ParticleManager::InitializeRandomEngine()
{
	// ランダムエンジンの初期化
	std::random_device rnd;
	randomEngine = std::mt19937(rnd());
}

void ParticleManager::CreatePipeline()
{
	// ルートシグネチャの作成
	CreateRootSignature();
	// グラフィックスパイプラインの設定
	SetGraphicsPipeline();
}

void ParticleManager::CreateRootSignature()
{
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // t0
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	D3D12_DESCRIPTOR_RANGE descriptorRangeTexture[1] = {};
	descriptorRangeTexture[0].BaseShaderRegister = 1; // t1始まる
	descriptorRangeTexture[0].NumDescriptors = 1;
	descriptorRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
	descriptorRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	D3D12_ROOT_PARAMETER rootParameters[3] = {};


	// RootParameter作成。複数設定できるので配列
	// 0.Material
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// 1.TransformMatrix
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	// 2.Texture
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeTexture;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeTexture);
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Samplerの設定
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成

	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));



	// 2. InputLayoutの設定
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// 3. BlendDateの設定
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = true;

	SetBlendMode(blendDesc, blendMode);
	//currentBlendMode = BlendMode::kBlendModeAdd;  // 現在のブレンドモード
	// α値のブレンド
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	// RasterizerStateの設定
	// カリングなし
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 塗りつぶしモード
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// 4. Shaderをコンパイルする
	vertexShaderBlob = dxCommon->CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = dxCommon->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}




void ParticleManager::SetGraphicsPipeline()
{
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
										vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
										pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// Dehiscenceの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むのか設定（気にしなくて良い）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 実際に生成
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}


void ParticleManager::SetBlendMode(D3D12_BLEND_DESC& desc, BlendMode mode)
{
	switch (mode)
	{
	case kBlendModeNormal: // 
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case kBlendModeAdd:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case kBlendModeSubtract:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case kBlendModeMultiply:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_DEST_COLOR;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		break;
	case kBlendModeScreen:
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_COLOR;
		break;
	default:
		assert(false);
		break;
	}
}



void ParticleManager::InitializeVertexData()
{

	// パーティクルの頂点データを初期化
	modelData.vertices.push_back({ .position = {1.0f, 1.0f, 0.0f, 1.0f},   .texcoord = {0.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f, 1.0f, 0.0f, 1.0f},  .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {1.0f, -1.0f, 0.0f, 1.0f},  .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {1.0f, -1.0f, 0.0f, 1.0f},  .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f, 1.0f, 0.0f, 1.0f},  .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.vertices.push_back({ .position = {-1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });

	// バッファービューの作成
	CreateVertexBufferView();
}

void ParticleManager::CreateVertexBufferView()
{
	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファービュー
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 1頂点分のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	// 頂点数
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// データの書き込み / マップ
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// データの書き込み
	memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

}

void ParticleManager::InitializeMaterial()
{
	// リソース生成
	materialResource = dxCommon->CreateBufferResource(sizeof(Material));

	// マテリアルデータに書き込み
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// マテリアルデータの初期化
	materialData->color = Vector4(0.3f, 1.0f, 0.7f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string textureFilePath)
{

	// 登録済みの名前か確認
	if (particleGroups.contains(name))
	{
		// 登録されていたらリターン
		Logger::Log("既に登録されているパーティクルグループ名です");
		return;
	}
	// 新たな空のパーティクルグループを作成しコンテナに登録
	particleGroups[name] = ParticleGroup();
	// マテリアルデータの作成
	ParticleGroup& particleGroup = particleGroups[name];

	// 新たなパーティクルのマテリアルデータを作成
	particleGroup.materialData.textureFilePath = textureFilePath;
	// テクスチャの読み込み
	TextureManager::GetInstance()->LoadTexture(particleGroup.materialData.textureFilePath);

	particleGroup.textureSrvIndex = TextureManager::GetInstance()->GetSrvIndex(particleGroup.materialData.textureFilePath);


	particleGroup.instancingResource = dxCommon->CreateBufferResource(sizeof(ParticleForGPU) * kMaxParticle);

	// インスタンシング用のリソースを作成
	particleGroup.instancingSrvIndex = srvManager->Allocate();
	// 書き込むためのアドレスを取得
	particleGroup.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData));



	srvManager->CreateSRVforStructuredBuffer(
		particleGroup.instancingSrvIndex,
		particleGroup.instancingResource.Get(),
		kMaxParticle,
		sizeof(ParticleForGPU));

	// インスタンス数を初期化
	particleGroup.kNumInstance = 0;
}

void ParticleManager::Update()
{
	uvOffset.x += uvScrollSpeed.x * kDeltaTime;
	uvOffset.y += uvScrollSpeed.y * kDeltaTime;

	// wrap (0〜1の範囲に保つ)
	uvOffset.x = std::fmod(uvOffset.x, 1.0f);
	uvOffset.y = std::fmod(uvOffset.y, 1.0f);
	materialData->uvTransform = MakeTranslateMatrix({ uvOffset.x, uvOffset.y, 0.0f });

	// 行列の更新
	UpdateMatrix();
	// 全てのパーティクルグループの処理
	UpdateParticle();

}

void ParticleManager::UpdateMatrix()
{
	// カメラ行列の取得
	cameraMatrix = camera->GetWorldMatrix();
	// ビュー行列の取得
	viewMatrix = camera->GetViewMatrix();
	// ワールド行列の取得
	backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);

	// ビルボード用の行列
	billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
}

void ParticleManager::UpdateParticle()
{
	// 全てのパーティクルグループについて処理
	for (auto& [name, particleGroup] : particleGroups)
	{
		particleGroup.kNumInstance = 0;
		// 各パーティクルを更新
		for (auto particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();)
		{
			// インスタンス数が上限を超えていないならインスタンスデータを書き込む
			if (particleGroup.kNumInstance < kMaxParticle)
			{
				// 生存時間を過ぎていたら更新せず描画対象にしない
				if (particleIterator->lifeTime <= particleIterator->currentTime)
				{
					// 消す
					particleIterator = particleGroup.particles.erase(particleIterator);
					continue;
				}
				// パーティクル１個ごとの更新
				particleIterator->transform.translate.x += particleIterator->velocity.x * kDeltaTime;
				particleIterator->transform.translate.y += particleIterator->velocity.y * kDeltaTime;
				particleIterator->transform.translate.z += particleIterator->velocity.z * kDeltaTime;
				particleIterator->currentTime += kDeltaTime;

				Matrix4x4 worldMatrixParticle = MakeAffineMatrix(particleIterator->transform.scale, particleIterator->transform.rotate, particleIterator->transform.translate);
				Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrixParticle, camera->GetViewProjectionMatrix());
				// インスタンシングデータの書き込み
				particleGroup.instancingData[particleGroup.kNumInstance].WVP = worldViewProjectionMatrix;
				particleGroup.instancingData[particleGroup.kNumInstance].World = worldMatrixParticle;
				particleGroup.instancingData[particleGroup.kNumInstance].color = particleIterator->color;
				particleGroup.instancingData[particleGroup.kNumInstance].color.w = 1.0f - (particleIterator->currentTime / particleIterator->lifeTime);

				++particleGroup.kNumInstance;
			}
			// 次のパーティクルに進める
			++particleIterator;
		}
	}
}



void ParticleManager::Draw()
{

	// コマンド : パイプラインステートオブジェクトを設定
	dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	// コマンド : プリミティブトロポジ(描画形状)を設定
	dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// コマンド : VertexBufferViewを設定
	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// コマンド : ルートシグネチャを設定
	dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// マテリアルデータの更新
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 全てのパーティクルグループについて処理
	for (auto& [name, particleGroup] : particleGroups)
	{
		if (particleGroup.kNumInstance == 0)
		{
			continue;
		}

		// 行列データの更新
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(
			1,
			srvManager->GetGPUDescriptorHandle(particleGroup.instancingSrvIndex));

		// インスタンシングデータの更新
		//srvManager->SetGraphicsDescriptorTable(1, particleGroup.instancingSrvIndex);

		// シェーダリソースビューの設定
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2,
			srvManager->GetGPUDescriptorHandle(particleGroup.textureSrvIndex));

		// 描画
		dxCommon->GetCommandList()->DrawInstanced(6, particleGroup.kNumInstance, 0, 0);
	}
}

void ParticleManager::DeleteParticleGroup(const std::string& name)
{
	auto it = particleGroups.find(name);
	if (it != particleGroups.end()) {
		// Unmapでリークしないようにする
		if (it->second.instancingResource) {
			it->second.instancingResource->Unmap(0, nullptr);
			it->second.instancingData = nullptr; // ポインタの無効化
			it->second.instancingResource.Reset(); // ComPtrの解放
		}

		// SRVの解放も（もし必要であれば）
		// srvManager->Release(it->second.srvIndex); // 必要なら

		// Mapから削除
		particleGroups.erase(it);
	}
}


// ParticleはEmitterを生成しないと描画されないのでこのメソッドだけを呼び出しても描画されない
void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count)
{
	// 登録済みのパーティクルグループ名かチェック
	auto it = particleGroups.find(name);
	// パーティクルグループが存在することを確認
	assert(it != particleGroups.end());
	// 指定されたパーティクルグループにパーティクルを追加
	ParticleGroup& group = it->second;
	// 各パーティクルを生成し追加
	for (uint32_t i = 0; i < count; ++i) {
		Particle newParticle = MakeParticle(randomEngine, position);
		group.particles.push_back(newParticle);
	}
}

void ParticleManager::EffectEmit(const std::string& name, const Vector3& position, uint32_t count)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end());
	ParticleGroup& group = it->second;

	for (uint32_t i = 0; i < count; ++i) {
		Particle newParticle;
		newParticle.transform.scale = { 0.1f, 0.1f, 0.1f };
		newParticle.transform.rotate = { 0.0f,0.0f,0.0f };
		newParticle.transform.translate = position;
		newParticle.velocity = { 0.0f, 0.0f, 0.0f };
		newParticle.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		newParticle.lifeTime = 1.0f;
		newParticle.currentTime = 0.0f;
		newParticle = MakeCylinderEffect(position);
		group.particles.push_back(newParticle);
	}
}

Particle ParticleManager::MakeParticle(std::mt19937& randomEngine, const  Vector3& position)
{
	Particle particle;
	std::uniform_real_distribution<float> distPosition(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distVelocity(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 3.3f, 0.0f };
	Vector3 randomPosition = { distPosition(randomEngine), distPosition(randomEngine),5.0f };
	particle.transform.translate = position + randomPosition;
	particle.velocity = { distVelocity(randomEngine), distVelocity(randomEngine), distVelocity(randomEngine) };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

Particle ParticleManager::MakePrimitiveEffect(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;

	std::uniform_real_distribution<float>distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float>distScale(0.4f, 1.5f);

	particle.transform.scale = { 0.05f,distScale(randomEngine),1.0f };// 横につぶす
	particle.transform.rotate = { 0.0f,0.0f,distRotate(randomEngine) };
	particle.transform.translate = Vector3(translate.x, translate.y, 15.0f);
	particle.velocity = { 0.0f,0.0f,0.0f };
	//white
	particle.color = { 1.0f,1.0f,1.0f,1.0f };
	particle.lifeTime = 10.0f;
	particle.currentTime = 0;
	return particle;


#pragma region ボンドルド
	//Particle particle;
	//particle.transform.scale = { 0.05f,1.0f,1.0f };// 横につぶす
	//particle.transform.rotate = { 0.0f,0.0f,0.0f, };
	//particle.transform.translate = Vector3(translate.x, translate.y, 15.0f);
	//particle.velocity = { 0.0f,0.0f,0.0f };
	////purple
	//particle.color = { 102.0f, 51.0f, 153.0f, 1.0f };
	//particle.lifeTime = 10.0f;
	//particle.currentTime = 0;
	//return particle;
#pragma endregion
}

Particle ParticleManager::MakeExplosionParticle(std::mt19937& randomEngine, const Vector3& position)
{
	Particle particle;

	// ランダムな方向ベクトルを生成（球面上のランダムな点）
	std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> distSpeed(2.0f, 5.0f);   // 速い速度
	std::uniform_real_distribution<float> distColor(0.8f, 1.0f);   // 明るい色
	std::uniform_real_distribution<float> distLifetime(0.5f, 1.5f); // 短命

	// 球面座標からランダムな方向を計算
	float theta = distAngle(randomEngine);  // 水平角度
	float phi = distAngle(randomEngine);    // 垂直角度
	float speed = distSpeed(randomEngine);

	// 放射状の速度ベクトル（中心から外側へ）
	particle.velocity = {
		std::sin(phi) * std::cos(theta) * speed,  // X方向
		std::sin(phi) * std::sin(theta) * speed,  // Y方向
		std::cos(phi) * speed                      // Z方向
	};

	particle.transform.scale = { 0.5f, 0.5f, 0.5f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = position;

	// 爆発らしい色（赤・オレンジ・黄色）
	particle.color = {
		1.0f,                        // 赤は最大
		distColor(randomEngine),     // 緑はランダム
		distColor(randomEngine) * 0.3f, // 青は少なめ
		1.0f
	};

	particle.lifeTime = distLifetime(randomEngine);
	particle.currentTime = 0.0f;

	return particle;
}


Particle ParticleManager::MakeSmokeParticle(std::mt19937& randomEngine, const Vector3& position)
{
	Particle particle;

	std::uniform_real_distribution<float> distPosition(-0.3f, 0.3f);
	std::uniform_real_distribution<float> distUpSpeed(0.3f, 0.8f);    // 上昇速度
	std::uniform_real_distribution<float> distSideSpeed(-0.2f, 0.2f); // 横の揺れ
	std::uniform_real_distribution<float> distLifetime(0.5f, 1.0f);

	// 主に上方向に移動
	particle.velocity = {
		distSideSpeed(randomEngine),  // わずかに横にずれる
		distUpSpeed(randomEngine),    // 上に昇る
		distSideSpeed(randomEngine)   // わずかに横にずれる
	};

	particle.transform.scale = { 0.3f, 0.3f, 0.3f };  // 大きめ
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = position + Vector3{
		distPosition(randomEngine),
		distPosition(randomEngine),
		0.0f
	};

	// 煙らしい色（グレー）
	std::uniform_real_distribution<float> distGray(0.7f, 0.9f);
	float gray = distGray(randomEngine);
	particle.color = { gray, gray, gray, 1.0f };

	particle.lifeTime = distLifetime(randomEngine);
	particle.currentTime = 0.0f;

	return particle;
}



Particle ParticleManager::MakeSparkParticle(std::mt19937& randomEngine, const Vector3& position)
{
	Particle particle;
	// 
	std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * std::numbers::pi_v<float>);
	// 速め
	std::uniform_real_distribution<float> distSpeed(3.0f, 8.0f);
	// 寿命
	std::uniform_real_distribution<float> distLifetime(0.1f, 0.3f);
	// 角度
	std::uniform_real_distribution<float> distUpAngle(-0.5f, 0.5f);

	float angle = distAngle(randomEngine);
	float speed = distSpeed(randomEngine);
	float upAngle = distUpAngle(randomEngine);

	// 主に横方向に飛び散る
	particle.velocity = {
		std::cos(angle) * speed,          // X方向
		upAngle * speed * 0.3f,           // Y方向
		std::sin(angle) * speed * 0.2f    // Z方向
	};

	particle.transform.scale = { 0.4f, 0.4f, 0.4f };  // 小さい
	particle.transform.rotate = { 0.0f, 0.0f, angle };
	particle.transform.translate = position;

	// スパークらしい色（明るい黄色・オレンジ・白)
	std::uniform_real_distribution<float> distColor(0.8f, 1.0f);
	particle.color = {
		1.0f,                        // 赤は最大
		distColor(randomEngine),     // 緑はランダム（黄色っぽく）
		distColor(randomEngine) * 0.2f, // 青は少なめ
		1.0f
	};

	particle.lifeTime = distLifetime(randomEngine);
	particle.currentTime = 0.0f;

	return particle;
}

Particle ParticleManager::MakeMagicCircleParticle(
	std::mt19937& randomEngine,
	const Vector3& position,
	float angle,      // 円周上の角度
	float radius)     // 半径
{
	Particle particle;

	std::uniform_real_distribution<float> distHeight(-0.1f, 0.1f);
	std::uniform_real_distribution<float> distColor(0.7f, 1.0f);

	// 円周上の位置を計算
	float x = std::cos(angle) * radius;
	float z = std::sin(angle) * radius;

	particle.transform.scale = { 0.3f, 0.3f, 0.3f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };

	// 地面に平行に配置（Y軸はほぼ固定）
	particle.transform.translate = {
		position.x + x,
		position.y + distHeight(randomEngine),  // 少しだけ高さにばらつき
		position.z + z
	};

	// ゆっくり回転（円周に沿って移動）
	float rotationSpeed = 0.5f;  // 回転速度
	particle.velocity = {
		-std::sin(angle) * rotationSpeed,  // 円周方向の速度
		0.0f,                              // Y軸方向には動かない
		std::cos(angle) * rotationSpeed
	};

	// 魔法陣らしい色（青・紫・シアン）
	particle.color = {
		distColor(randomEngine) * 0.3f,  // 赤は少なめ
		distColor(randomEngine) * 0.5f,  // 緑も少なめ
		distColor(randomEngine),         // 青は強め
		1.0f
	};

	particle.lifeTime = 5.0f;  // 長く表示
	particle.currentTime = 0.0f;

	return particle;
}

Particle ParticleManager::MakeJumpDustParticle(std::mt19937& randomEngine, const Vector3& position)
{
	Particle p;

	// 足元から少しだけ散る
	std::uniform_real_distribution<float> distX(-0.3f, 0.12f);
	std::uniform_real_distribution<float> distSpeedX(-0.35f, 0.35f);
	std::uniform_real_distribution<float> distSpeedY(0.25f, 0.65f);
	std::uniform_real_distribution<float> distScale(0.10f, 0.16f);
	std::uniform_real_distribution<float> distLife(0.12f, 0.22f);
	// スケール（小さめでランダム）
	const float s = distScale(randomEngine);
	p.transform.scale = { s, s, s };
	p.transform.rotate = { 0.0f, 0.0f, 0.0f };

	// 位置（足元の周りに少し散らす）
	p.transform.translate = position + Vector3{ distX(randomEngine), 0.02f, 0.0f };

	// 速度（少しだけ上＋左右）
	p.velocity = { distSpeedX(randomEngine), distSpeedY(randomEngine), 0.0f };

	// 色（砂埃：薄い黄土色系）
	p.color = { 0.85f, 0.78f, 0.60f, 1.0f };

	p.lifeTime = distLife(randomEngine);
	p.currentTime = 0.0f;

	return p;
}


Particle ParticleManager::MakeLandDustParticle(std::mt19937& randomEngine, const Vector3& position)
{

	Particle p;

	std::uniform_real_distribution<float> distX(-0.4f, 0.22f);
	std::uniform_real_distribution<float> distSpeedX(-1.8f, 1.8f);
	std::uniform_real_distribution<float> distSpeedY(0.12f, 0.40f);
	std::uniform_real_distribution<float> distScale(0.12f, 0.24f);
	std::uniform_real_distribution<float> distLife(0.18f, 0.35f);

	const float s = distScale(randomEngine);
	p.transform.scale = { s, s, s };
	p.transform.rotate = { 0.0f, 0.0f, 0.0f };

	p.transform.translate = position + Vector3{ distX(randomEngine), 0.02f, 0.0f };

	p.velocity = { distSpeedX(randomEngine), distSpeedY(randomEngine), 0.0f };

	// 砂色（αは控えめに）
	p.color = { 0.82f, 0.74f, 0.58f, 0.85f };

	p.lifeTime = distLife(randomEngine);
	p.currentTime = 0.0f;

	return p;
}

void ParticleManager::CreateRingVertex()
{
	std::vector<VertexData> vertices;
	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.2f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	for (uint32_t i = 0; i < kRingDivide; ++i) {
		float angle0 = i * radianPerDivide;
		float angle1 = (i + 1) * radianPerDivide;

		float sin0 = std::sin(angle0);
		float cos0 = std::cos(angle0);
		float sin1 = std::sin(angle1);
		float cos1 = std::cos(angle1);

		// 外側・内側の各点
		Vector4 outer0 = { sin0 * kOuterRadius, cos0 * kOuterRadius, 0.0f, 1.0f };
		Vector4 outer1 = { sin1 * kOuterRadius, cos1 * kOuterRadius, 0.0f, 1.0f };
		Vector4 inner0 = { sin0 * kInnerRadius, cos0 * kInnerRadius, 0.0f, 1.0f };
		Vector4 inner1 = { sin1 * kInnerRadius, cos1 * kInnerRadius, 0.0f, 1.0f };

		Vector2 uvOuter0 = { float(i) / kRingDivide, 0.0f };
		Vector2 uvOuter1 = { float(i + 1) / kRingDivide, 0.0f };
		Vector2 uvInner0 = { float(i) / kRingDivide, 1.0f };
		Vector2 uvInner1 = { float(i + 1) / kRingDivide, 1.0f };

		Vector3 normal = { 0.0f, 0.0f, 1.0f };

		// 三角形 1（outer0, outer1, inner0）
		vertices.push_back({ outer0, uvOuter0, normal });
		vertices.push_back({ outer1, uvOuter1, normal });
		vertices.push_back({ inner0, uvInner0, normal });

		// 三角形 2（inner0, outer1, inner1）
		vertices.push_back({ inner0, uvInner0, normal });
		vertices.push_back({ outer1, uvOuter1, normal });
		vertices.push_back({ inner1, uvInner1, normal });
	}

	// 頂点バッファの作成
	size_t vertexSize = sizeof(VertexData) * vertices.size();

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = vertexSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	dxCommon->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));

	VertexData* mapData = nullptr;
	vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mapData));
	memcpy(mapData, vertices.data(), vertexSize);
	vertexBuffer->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView{};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<UINT>(vertexSize);
	vbView.StrideInBytes = sizeof(VertexData);

	ringVertexBuffer = vertexBuffer;
	ringVertexBufferView = vbView;
	ringVertexCount = static_cast<uint32_t>(vertices.size());
}

void ParticleManager::DrawRing()
{
	if (!ringVertexBuffer) return;

	auto* cmdList = dxCommon->GetCommandList();

	// 必要なパイプラインセット
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	cmdList->SetPipelineState(graphicsPipelineState.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &ringVertexBufferView);
	cmdList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// テクスチャSRV（仮にindex=0とする）
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = srvManager->GetGPUDescriptorHandle(0);
	cmdList->SetGraphicsRootDescriptorTable(2, textureHandle);

	cmdList->DrawInstanced(ringVertexCount, 1, 0, 0);

}


Particle ParticleManager::MakeUpArrowParticle(std::mt19937& randomEngine, const Vector3& position)
{
	// パーティクルの基礎情報
	Particle p;
	// 位置のばらつき
	std::uniform_real_distribution<float> distX(-0.5f, 0.5f);
	std::uniform_real_distribution<float> distY(-0.3f, 0.3f);
	std::uniform_real_distribution<float> distSpeedX(-0.05f, 0.05f);
	std::uniform_real_distribution<float> distSpeedY(0.6f, 1.0f);
	std::uniform_real_distribution<float> distScale(0.18f, 0.28f);
	std::uniform_real_distribution<float> distLife(0.35f, 0.55f);

	float s = distScale(randomEngine);

	p.transform.scale = { s, s, s };
	p.transform.rotate = { 0.0f, 0.0f, 0.0f };
	p.transform.translate = position + Vector3{
		distX(randomEngine),
		distY(randomEngine),
		0.0f
	};

	// 上方向に流す
	p.velocity = {
		distSpeedX(randomEngine),
		distSpeedY(randomEngine),
		0.0f
	};

	// 目立つ色
	p.color = { 1.0f, 0.95f, 0.3f, 1.0f };

	p.lifeTime = distLife(randomEngine);
	p.currentTime = 0.0f;

	return p;
}

Particle ParticleManager::MakeRingEffect(const Vector3& position) {
	Particle particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };     // サイズ（大きすぎると画面外）
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = { 0.0f, 0.0f, 5.0f }; // Z = 5.0f で手前に表示されやすい
	particle.velocity = { 0.0f, 0.0f, 0.0f };
	particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	particle.lifeTime = 1.0f;
	particle.currentTime = 0.0f;
	return particle;
}

Particle ParticleManager::MakeCylinderEffect(const Vector3& position)
{
	Particle particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };     // サイズ（大きすぎると画面外）
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = position;
	particle.velocity = { 0.0f, 0.0f, 0.0f };
	particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	particle.lifeTime = 1.0f;
	particle.currentTime = 0.0f;
	return particle;
}

void ParticleManager::CreateCylinderVertex() {
	std::vector<VertexData> vertices;
	const uint32_t kCylinderDivide = 32;
	const float kTopRadius = 1.0f;
	const float kBottomRadius = 1.0f;
	const float kHeight = 3.0f;
	const float halfHeight = kHeight * 0.5f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivide);

	for (uint32_t i = 0; i < kCylinderDivide; ++i) {
		float angle0 = i * radianPerDivide;
		float angle1 = (i + 1) * radianPerDivide;

		float sin0 = std::sin(angle0);
		float cos0 = std::cos(angle0);
		float sin1 = std::sin(angle1);
		float cos1 = std::cos(angle1);

		float u0 = float(i) / float(kCylinderDivide);
		float u1 = float(i + 1) / float(kCylinderDivide);

		// 各頂点の位置
		Vector4 top0 = { sin0 * kTopRadius, +halfHeight, cos0 * kTopRadius, 1.0f };
		Vector4 top1 = { sin1 * kTopRadius, +halfHeight, cos1 * kTopRadius, 1.0f };
		Vector4 bottom0 = { sin0 * kBottomRadius, -halfHeight, cos0 * kBottomRadius, 1.0f };
		Vector4 bottom1 = { sin1 * kBottomRadius, -halfHeight, cos1 * kBottomRadius, 1.0f };

		// テクスチャ座標
		Vector2 uvTop0 = { u0, 0.0f };
		Vector2 uvTop1 = { u1, 0.0f };
		Vector2 uvBottom0 = { u0, 1.0f };
		Vector2 uvBottom1 = { u1, 1.0f };

		// 法線ベクトル（正規化）
		Vector3 normal0 = Vector3{ sin0, 0.0f, cos0 }.Normalized();
		Vector3 normal1 = Vector3{ sin1, 0.0f, cos1 }.Normalized();

		// 側面：三角形2枚（1クアッド）
		vertices.push_back({ top0,    uvTop0,    normal0 });
		vertices.push_back({ top1,    uvTop1,    normal1 });
		vertices.push_back({ bottom0, uvBottom0, normal0 });

		vertices.push_back({ top1,    uvTop1,    normal1 });
		vertices.push_back({ bottom1, uvBottom1, normal1 });
		vertices.push_back({ bottom0, uvBottom0, normal0 });
	}

	// 頂点バッファ作成
	size_t vertexSize = sizeof(VertexData) * vertices.size();

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = vertexSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));
	assert(SUCCEEDED(hr));

	VertexData* mapData = nullptr;
	vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mapData));
	memcpy(mapData, vertices.data(), vertexSize);
	vertexBuffer->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView{};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = static_cast<UINT>(vertexSize);
	vbView.StrideInBytes = sizeof(VertexData);

	ringVertexBuffer = vertexBuffer;
	ringVertexBufferView = vbView;
	ringVertexCount = static_cast<uint32_t>(vertices.size());
}



void ParticleManager::DrawCylinder()
{
	if (!ringVertexBuffer) return;

	auto* cmdList = dxCommon->GetCommandList();

	// 必要なパイプラインセット
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	cmdList->SetPipelineState(graphicsPipelineState.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &ringVertexBufferView);
	cmdList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// テクスチャSRV（仮にindex=0とする）
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = srvManager->GetGPUDescriptorHandle(0);
	cmdList->SetGraphicsRootDescriptorTable(2, textureHandle);

	cmdList->DrawInstanced(ringVertexCount, 1, 0, 0);


}

void ParticleManager::EnsureParticleGroup(const std::string& name, const std::string& textureFilePath)
{
	// パーティクルグループが存在しない場合は作成
	if (!HasParticleGroup(name))
	{
		// パーティクルグループを作成
		CreateParticleGroup(name, textureFilePath);
	}
}

Particle ParticleManager::MakeParticleByType(std::mt19937& randomEngine, const Vector3& position, EffectType type)
{
	// EffectTypeに応じて適切なパーティクルを生成
	switch (type)
	{
	case EffectType::Explosion:
		return MakeExplosionParticle(randomEngine, position);
	case EffectType::Smoke:
		return MakeSmokeParticle(randomEngine, position);
	case EffectType::Spark:
		return MakeSparkParticle(randomEngine, position);
	case EffectType::JumpDust:
		return MakeJumpDustParticle(randomEngine, position);
	case EffectType::LandDust:
		return MakeLandDustParticle(randomEngine, position);
	case EffectType::UpArrow:
		return MakeUpArrowParticle(randomEngine, position);
	case EffectType::Default:
	default:
		return MakeParticle(randomEngine, position);
	}
}

Particle ParticleManager::MakeParticleByTypeWithColor(
	std::mt19937& randomEngine,
	const Vector3& position,
	EffectType type,
	const Vector4& colorTint)
{
	Particle particle;

	switch (type)
	{
	case EffectType::Explosion:
		particle = MakeExplosionParticle(randomEngine, position);
		break;
	case EffectType::Smoke:
		particle = MakeSmokeParticle(randomEngine, position);
		break;
	case EffectType::Spark:
		particle = MakeSparkParticle(randomEngine, position);
		break;
	case EffectType::JumpDust:
		particle = MakeJumpDustParticle(randomEngine, position);
		break;
	case EffectType::LandDust:
		particle = MakeLandDustParticle(randomEngine, position);
		break;
	case EffectType::UpArrow:
		particle = MakeUpArrowParticle(randomEngine, position);
		break;
	case EffectType::Default:
	default:
		particle = MakeParticle(randomEngine, position);
		break;
	}

	// 色を乗算で適用
	particle.color.x *= colorTint.x;
	particle.color.y *= colorTint.y;
	particle.color.z *= colorTint.z;
	particle.color.w *= colorTint.w;

	return particle;
}

void ParticleManager::EmitWithEffectType(const std::string& name, const Vector3& position, uint32_t count, EffectType effectType)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end());

	ParticleGroup& group = it->second;
	for (uint32_t i = 0; i < count; ++i) {
		Particle newParticle = MakeParticleByType(randomEngine, position, effectType);
		group.particles.push_back(newParticle);
	}
}

void ParticleManager::EmitMagicCircle(const std::string& name, const Vector3& position, uint32_t count, float radius)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end());

	ParticleGroup& group = it->second;

	// 円周を均等に分割
	float angleStep = (2.0f * std::numbers::pi_v<float>) / count;

	for (uint32_t i = 0; i < count; ++i) {
		float angle = angleStep * i;
		Particle newParticle = MakeMagicCircleParticle(randomEngine, position, angle, radius);
		group.particles.push_back(newParticle);
	}
}


void ParticleManager::EmitComplexMagicCircle(
	const std::string& name,
	const Vector3& position)
{
	// 内側の円（小さい、速く回転）
	EmitMagicCircle(name, position, 20, 2.0f);

	// 中間の円（中くらい、中速回転）
	EmitMagicCircle(name, position, 30, 4.0f);

	// 外側の円（大きい、ゆっくり回転）
	EmitMagicCircle(name, position, 40, 6.0f);

	// 中心に光る点
	Particle centerParticle;
	centerParticle.transform.scale = { 1.0f, 1.0f, 1.0f };
	centerParticle.transform.translate = position;
	centerParticle.velocity = { 0.0f, 0.0f, 0.0f };
	centerParticle.color = { 0.5f, 0.7f, 1.0f, 1.0f };
	centerParticle.lifeTime = 5.0f;
	centerParticle.currentTime = 0.0f;

	auto it = particleGroups.find(name);
	if (it != particleGroups.end()) {
		it->second.particles.push_back(centerParticle);
	}
}

void ParticleManager::EmitWithEffectTypeAndColor(
	const std::string& name,
	const Vector3& position,
	uint32_t count,
	EffectType effectType,
	const Vector4& colorTint)
{
	auto it = particleGroups.find(name);
	assert(it != particleGroups.end());

	ParticleGroup& group = it->second;
	for (uint32_t i = 0; i < count; ++i) {
		Particle newParticle = MakeParticleByTypeWithColor(randomEngine, position, effectType, colorTint);
		group.particles.push_back(newParticle);
	}
}
