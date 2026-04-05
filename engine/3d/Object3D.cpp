#include "Object3D.h"
#include "Object3DCommon.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include "MakeMatrix.h"
#include "ModelManager.h"
using namespace std;
using namespace Engine;
void Object3D::Initialize()
{

	this->camera = Object3DCommon::GetInstance()->GetDefaultCamera();

	CreateTransformationMatrixData();
	CreateDirectionalLightResource();
	// カメラバッファを生成
	CreateCameraResource();

	// Transformの初期化
	transform = { { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f  } };
}

void Object3D::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjection;
	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjection = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjection = worldMatrix;
	}
	wvpData->WVP = worldViewProjection;
	wvpData->World = worldMatrix;
}

void Object3D::Draw() {

	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->DrawSettingCommon();

	Object3DCommon::GetInstance()->GetDxCommon()->GetCommandList()->
		SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	Object3DCommon::GetInstance()->GetDxCommon()->GetCommandList()->
		SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	Object3DCommon::GetInstance()->GetDxCommon()->GetCommandList()->
		SetGraphicsRootConstantBufferView(5, cameraResource->GetGPUVirtualAddress());


	// 通常のモデル描画
	model->Draw();

}

void Object3D::Create(Model* model)
{
	assert(model); // モデルがnullptrでないことを確認

	this->model = model;

	// カメラを取得
	this->camera = Object3DCommon::GetInstance()->GetDefaultCamera();

	// 変換行列バッファを生成
	CreateTransformationMatrixData();

	// 光源バッファを生成
	CreateDirectionalLightResource();

	// カメラバッファを生成
	CreateCameraResource();


	// 初期Transform
	transform = {
		{ 1.0f, 1.0f, 1.0f },   // scale
		{ 0.0f, 0.0f, 0.0f },   // rotate
		{ 0.0f, 0.0f, 0.0f }    // translate
	};
}

Object3D::~Object3D()
{
	if (wvpResource) { wvpResource->Unmap(0, nullptr); wvpData = nullptr; }
	if (directionalLightResource) { directionalLightResource->Unmap(0, nullptr); directionalLightData = nullptr; }
	if (cameraResource) { cameraResource->Unmap(0, nullptr); cameraForGpuData = nullptr; }
}



void Object3D::SetModel(const std::string& filePath)
{
	// モデルを検索してセットする
	model = ModelManager::GetInstance().FindModel(filePath);
}





void Object3D::CreateTransformationMatrixData()
{
	// 変換行列リソースを作成
	wvpResource = Object3DCommon::GetInstance()->GetDxCommon()->
		CreateBufferResource(sizeof(TransformationMatrix));
	// 変換行列リソース
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 変換行列データの初期化
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

}

void Object3D::CreateDirectionalLightResource()
{
	// 平行光源リソースを作成
	directionalLightResource = Object3DCommon::GetInstance()->GetDxCommon()->
		CreateBufferResource(sizeof(DirectionalLight));
	// 平行光源リソースにデータを書き込むためのアドレスを取得してdirectionalLightDataに割り当てる
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 平行光源データの初期化 / デフォルト値
	// ライトの色
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// ライトの向き
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	// 輝度
	directionalLightData->intensity = 1.0f;
}

void Object3D::CreateCameraResource()
{
	// カメラリソースの作成
	cameraResource = Object3DCommon::GetInstance()->GetDxCommon()->
		CreateBufferResource(sizeof(CameraForGPU));
	// カメラリソースにデータを書き込むためのアドレスを取得してcameraForGpuDataに割り当てる
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGpuData));
	// カメラデータの初期化
	cameraForGpuData->worldPosition = { 0.0f,4.0f,-10.0f };


}

