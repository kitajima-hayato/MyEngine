#pragma once

/// <summary>
/// ブロックタイプ列挙型
/// ブロックの種類を定義する
/// </summary>

enum class BlockType
{
	Air = 0,
	GrassBlock = 1,
	SoilBlock = 2,
	kGoalUp = 3,
	kGoalDown = 4,
	breakBlock = 5,
	moveBlock = 6,
	jumpBlock = 7,
	unBreakable = 8,
	damageBlock = 9,

	// 範囲検索の最終値
	/// 検索を行う際にCountを指定することで範囲検索を可能にできる
	Count,	
};




// 新しいブロックを追加するときは、下のリストを上から順にチェックすればOK
//
// BlockType に追加
//
// enum class BlockType に Count の前で追加
//
//	既存の数値は変えない
//
//	Map クラス の ImGui 表示名を追加
//
//	blockTypeNames[] に "NewBlockName" を追加
//
//	インデックスと enum の値が対応していることを確認
//
//	色テーブルに追加
//
//	GetBlockColorByType(BlockType) の switch に case を追加
//
//	グリッド・プレビューの色はここを使うように統一しておく
//
//	マップ生成 / 見た目
//
//	GenerateMapBlock() などのブロック生成処理の switch に case を追加
//
//	挙動 / 当たり判定（必要なら）
//
//	プレイヤーやギミックの判定で switch (blockType) 等に case を追加
//
//	CSVの互換性チェック
//
//	既存の CSV を読み込んで想定通りのマップか確認
//
//	おかしかったら enum の順番や値をいじっていないか確認