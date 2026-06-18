#include "Block.h"

void Block::OnCollision(Collider* other)
{

}


void Block::Initialize(BlockType blockType, Vector3 position) {

	/// モデルの初期化
	this->blockType = blockType;
	blockModel = std::make_unique<Object3D>();
	blockModel->Initialize();
	transform = {
		// Scale
		{1.0f, 1.0f, 1.0f},
		// Rotate
		{0.0f, 0.0f, 0.0f},
		// Translate
		position
	};


	switch (this->blockType) {
	case BlockType::Air:
		break;
	case BlockType::GrassBlock:
		blockModel->SetModel("GamePlay/Blocks/grassblock");
		break;
	case BlockType::SoilBlock:
		blockModel->SetModel("GamePlay/Blocks/soilblock");
		break;
	case BlockType::breakBlock:
		blockModel->SetModel("GamePlay/Blocks/breakblock");
		break;
	case BlockType::moveBlock:
		blockModel->SetModel("GamePlay/Blocks/moveblock");
		break;
	case BlockType::jumpBlock:
		blockModel->SetModel("GamePlay/Blocks/sandblock");
		break;
	case BlockType::kGoalUp:
		blockModel->SetModel("GamePlay/Blocks/goalblock/up");
		break;
	case BlockType::kGoalDown:
		blockModel->SetModel("GamePlay/Blocks/goalblock/down");
		break;
	case BlockType::unBreakable:
		blockModel->SetModel("GamePlay/Blocks/unbreakableblock");
		break;
	case BlockType::damageBlock:
		blockModel->SetModel("GamePlay/Blocks/damageblock");
		break;
	case BlockType::toggleBlockOn:
		blockModel->SetModel("GamePlay/Blocks/toggleblock_on");
		break;
	case BlockType::toggleBlockOff:
		blockModel->SetModel("GamePlay/Blocks/toggleblock_off");
		break;
	case BlockType::BossSpawnableBlock:
		// blockModel->SetModel("GamePlay/Blocks/bossspawnableblock");
		blockModel->SetModel("GamePlay/Blocks/sandblock");
		break;
	case BlockType::BossEntrance:
		// 見た目は無し(プレイヤーが通り抜けるトリガーゾーン)
		break;
	default:
		blockModel->SetModel("GamePlay/Blocks/grassblock");
		break;
	}
	blockModel->SetTransform(transform);


	if (blockType == BlockType::toggleBlockOn || blockType == BlockType::toggleBlockOff) {
		toggleOnEffect_ = ParticlePresets::CreateToggleOnBurst(position);
		toggleOffEffect_ = ParticlePresets::CreateToggleOffBurst(position);
	}

}



void Block::Update() {
	if (blockType == BlockType::Air || 
		blockType == BlockType::BossEntrance ||
		!isAlive_) return;
	// 移動ブロックの処理
	if (blockType == BlockType::moveBlock) {
		if (moveRight_) {
			moveTime_ += moveSpeed_;
			if (moveTime_ > 1.0f) {
				moveTime_ = 1.0f;
				moveRight_ = false; // 右端に到達したら左へ
			}
		} else {
			moveTime_ -= moveSpeed_;
			if (moveTime_ < 0.0f) {
				moveTime_ = 0.0f;
				moveRight_ = true; // 左端に到達したら右へ
			}
		}

		// ---- イージング関数（easeInOutSine） ----
		// 0～1 → 0～1へ滑らかに補間
		float eased = 0.5f - 0.5f * cosf(moveTime_ * 3.14159f);

		// -range ～ +range の範囲で往復
		transform.translate.x = (eased * 2.0f - 1.0f) * moveRange_;

		// モデルに反映
		blockModel->SetTransform(transform);
	}
	blockModel->Update();

	// トグルブロックの演出更新
	if (toggleOnEffect_) {
		toggleOnEffect_->Update();
	}
	if (toggleOffEffect_) {
		toggleOffEffect_->Update();
	}
}

void Block::Draw() {
	// ブロックの描画 / 描画対象が無い場合と壊れている場合は描画しない
	if (blockType == BlockType::Air || 
		blockType == BlockType::BossEntrance ||
		!isAlive_) return;
	blockModel->Draw();
}

Block* Block::CreateBlock(BlockType blockType, Vector3 position)
{
	// ブロックの生成
	Block* newBlock = new Block();
	// 初期化
	newBlock->Initialize(blockType, position);
	// 生成したブロックを返す
	return newBlock;
}

void Block::UpdateToggleVisual(bool toggleState)
{
	// トグルブロック以外の場合は処理しない
	if (blockType != BlockType::toggleBlockOff && blockType != BlockType::toggleBlockOn) {
		return;
	}
	// 現在の状態を判定
	const bool isSolidNow = IsSolid(toggleState);

	// 初回は記録だけ
	if (!toggleVisualInitialized_) {
		wasSolid_ = isSolidNow;
		toggleVisualInitialized_ = true;
	}
	// 状態変化時だけ演出
	else if (wasSolid_ != isSolidNow) {
		Vector3 effectPos = transform.translate;
		effectPos.z -= 1.0f;

		if (isSolidNow) {
			if (toggleOnEffect_) {
				toggleOnEffect_->SetTranslate(effectPos);
				toggleOnEffect_->Play();
			}
		} else {
			if (toggleOffEffect_) {
				toggleOffEffect_->SetTranslate(effectPos);
				toggleOffEffect_->Play();
			}
		}

		wasSolid_ = isSolidNow;
	}

	// 見た目更新
	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	if (!isSolidNow) {
		scale = { 0.7f, 0.7f, 0.7f };
	}
	transform.scale = scale;
	blockModel->SetTransform(transform);
}


bool Block::IsSolid(bool toggleState) const
{
	// Airブロックは常に非表示で判定もない
	if (!isAlive_) {
		return false;
	}
	// ブロックの種類に応じて判定の有無を返す
	switch (blockType) {
		// Airブロックは常に非表示で判定もない
	case BlockType::Air:
		return false;
		// Onブロックはトグル状態がオンのときだけ判定がある
	case BlockType::toggleBlockOn:
		return toggleState;
		// Offブロックはトグル状態がオフのときだけ判定がある
	case BlockType::toggleBlockOff:
		return !toggleState;

	default:
		return true;
	}
}