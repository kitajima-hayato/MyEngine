#include "Block.h"
#include "engine/3d/ModelManager.h"

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
	default:
		blockModel->SetModel("GamePlay/Blocks/grassblock");
		break;
	}
	blockModel->SetTransform(transform);


}



void Block::Update() {
	if (blockType == BlockType::Air || !isAlive_) return;
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
}

void Block::Draw() {
	// ブロックの描画 / 描画対象が無い場合と壊れている場合は描画しない
	if (blockType == BlockType::Air || !isAlive_) return;
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
	const bool isSolidNow = IsSolid(toggleState);

	// 初回または状態変化時だけモデル差し替え
	if (!toggleVisualInitialized_ || wasSolid_ != isSolidNow) {
		if (blockType == BlockType::toggleBlockOn) {
			if (isSolidNow) {
				blockModel->SetModel("GamePlay/Blocks/toggleblock_on");
			} else {
				//blockModel->SetModel("GamePlay/Blocks/toggleblock_on_off");
				blockModel->SetModel("GamePlay/Blocks/nullblock");
			}
		} else if (blockType == BlockType::toggleBlockOff) {
			if (isSolidNow) {
				blockModel->SetModel("GamePlay/Blocks/toggleblock_off");
			} else {
				//blockModel->SetModel("GamePlay/Blocks/toggleblock_off_off");
				blockModel->SetModel("GamePlay/Blocks/nullblock");
			}
		}

		wasSolid_ = isSolidNow;
		toggleVisualInitialized_ = true;
	}

	// スケール反映
	if (isSolidNow) {
		transform.scale = { 1.0f, 1.0f, 1.0f };
	} else {
		transform.scale = { 0.7f, 0.7f, 0.7f };
	}

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