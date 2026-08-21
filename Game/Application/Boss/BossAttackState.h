#pragma once
#include "engine/math/MyMath.h"

// 前方宣言
class BossEnemy;

/// <summary>
/// ボスの攻撃を１回分を表すステートの基底クラス
/// マイフレームUPdateを呼びIsFineshedがtureになったら攻撃終了
/// </summary>
class BossAttackState
{
public:
	// ボスの攻撃のデストラクタ
	virtual ~BossAttackState() = default;

	/// <summary>
	/// ボスの攻撃の更新
	/// </summary>
	/// <param name="boss">攻撃をするボス</param>
	virtual void Update(BossEnemy& boss) = 0;

	/// <summary>
	/// 攻撃が終了したかどうかを返す
	/// </summary>
	bool IsFinished()const { return finished; }

protected:
	// 攻撃が終了したかどうか
	bool finished = false;
};

// 単発：指定した高さに1発落とす
class BossAttackSingleShot : public BossAttackState
{
public:
	/// <param name="offsetX">ボスからどれだけ横にずらして落とすか</param>
	/// <param name="landY">着弾させる高さ</param>
	BossAttackSingleShot(float offsetX, float landY)
		: offsetX_(offsetX), landY_(landY) {}
	void Update(BossEnemy& boss) override;

private:
	float offsetX_;
	float landY_;
};

// 連射：interval秒おきにcount回、横と高さを少しずつずらして階段状に落とす
class BossAttackRapidFire : public BossAttackState
{
public:
	/// <param name="count">発射回数</param>
	/// <param name="interval">発射間隔(秒)</param>
	/// <param name="offsetXStep">1発ごとにずらす着弾X</param>
	/// <param name="landYBegin">1発目の着弾高さ</param>
	/// <param name="landYStep">1発ごとにずらす着弾高さ</param>
	BossAttackRapidFire(int count, float interval, float offsetXStep,
		float landYBegin, float landYStep)
		: count_(count), interval_(interval), offsetXStep_(offsetXStep),
		landYBegin_(landYBegin), landYStep_(landYStep) {}
	void Update(BossEnemy& boss) override;

private:
	int count_;
	float interval_;
	float offsetXStep_;
	float landYBegin_;
	float landYStep_;
	int fired_ = 0;
	float timer_ = 0.0f;
};

// 横に広がるwayCount発を同時発射（着弾点を山なり／谷型に並べる）
class BossAttackNWay : public BossAttackState
{
public:
	/// <param name="wayCount">弾数</param>
	/// <param name="spanX">着弾点の左右の広がり幅</param>
	/// <param name="landYCenter">中央の弾の着弾高さ</param>
	/// <param name="landYCurve">端の弾を何段持ち上げるか（負なら谷型）</param>
	BossAttackNWay(int wayCount, float spanX, float landYCenter, float landYCurve = 0.0f)
		: wayCount_(wayCount), spanX_(spanX),
		landYCenter_(landYCenter), landYCurve_(landYCurve) {}
	void Update(BossEnemy& boss) override;

private:
	int wayCount_;
	float spanX_;
	float landYCenter_;
	float landYCurve_;
};

// 薙ぎ払い：着弾点を横へ流しながら、高さも一緒に変化させる
class BossAttackSweep : public BossAttackState
{
public:
	/// <param name="count">発射数</param>
	/// <param name="interval">発射間隔(秒)</param>
	/// <param name="spanX">薙ぎ払う横幅</param>
	/// <param name="landYBegin">薙ぎ始めの着弾高さ</param>
	/// <param name="landYEnd">薙ぎ終わりの着弾高さ</param>
	/// <param name="reverse">trueで逆方向に薙ぎ払う</param>
	BossAttackSweep(int count, float interval, float spanX,
		float landYBegin, float landYEnd, bool reverse = false)
		: count_(count), interval_(interval), spanX_(spanX),
		landYBegin_(landYBegin), landYEnd_(landYEnd), reverse_(reverse) {}
	void Update(BossEnemy& boss) override;

private:
	int count_;
	float interval_;
	float spanX_;
	float landYBegin_;
	float landYEnd_;
	bool reverse_;
	int fired_ = 0;
	float timer_ = 0.0f;
};

// 拡散連射：NWayをsetCount回、段ごとに高さを変えて撃つ
class BossAttackBurstNWay : public BossAttackState
{
public:
	/// <param name="setCount">NWayを撃つ回数</param>
	/// <param name="wayCount">1セットあたりの弾数</param>
	/// <param name="spanX">1セットの着弾X幅</param>
	/// <param name="interval">セット間隔(秒)</param>
	/// <param name="landYBegin">1セット目の着弾高さ</param>
	/// <param name="landYStep">セットごとの高さ変化</param>
	/// <param name="offsetXStep">セットごとの横ずらし</param>
	BossAttackBurstNWay(int setCount, int wayCount, float spanX, float interval,
		float landYBegin, float landYStep, float offsetXStep = 0.0f)
		: setCount_(setCount), wayCount_(wayCount), spanX_(spanX), interval_(interval),
		landYBegin_(landYBegin), landYStep_(landYStep), offsetXStep_(offsetXStep) {}
	void Update(BossEnemy& boss) override;

private:
	int setCount_;
	int wayCount_;
	float spanX_;
	float interval_;
	float landYBegin_;
	float landYStep_;
	float offsetXStep_;
	int firedSet_ = 0;
	float timer_ = 0.0f;
};

// 溜め攻撃：予兆を見せてから、格子状（市松模様）に一斉ばらまき
class BossAttackCharge : public BossAttackState
{
public:
	/// <param name="chargeTime">溜め時間(秒)</param>
	/// <param name="columns">横方向の弾数</param>
	/// <param name="rows">縦方向の段数</param>
	/// <param name="spanX">ばらまく横幅</param>
	/// <param name="landYBottom">最下段の着弾高さ</param>
	/// <param name="landYTop">最上段の着弾高さ</param>
	BossAttackCharge(float chargeTime, int columns, int rows,
		float spanX, float landYBottom, float landYTop)
		: chargeTime_(chargeTime), columns_(columns), rows_(rows),
		spanX_(spanX), landYBottom_(landYBottom), landYTop_(landYTop) {}
	void Update(BossEnemy& boss) override;

private:
	float chargeTime_;
	int columns_;
	int rows_;
	float spanX_;
	float landYBottom_;
	float landYTop_;
	float timer_ = 0.0f;
};

// 狙い撃ち：プレイヤーの現在位置（X/Yとも）を狙って落とす
class BossAttackAimed : public BossAttackState
{
public:
	/// <param name="count">発射数</param>
	/// <param name="interval">発射間隔(秒)</param>
	/// <param name="aimOffsetY">プレイヤーの何段上を狙うか</param>
	BossAttackAimed(int count, float interval, float aimOffsetY = 1.5f)
		: count_(count), interval_(interval), aimOffsetY_(aimOffsetY) {}
	void Update(BossEnemy& boss) override;

private:
	int count_;
	float interval_;
	float aimOffsetY_;
	int fired_ = 0;
	float timer_ = 0.0f;
};
// 突進：奥からプレイヤーのいる手前(z=0)へ突っ込んでくる
class BossAttackRush : public BossAttackState
{
public:
	/// <param name="telegraphTime">狙いを定める予告時間(秒)</param>
	/// <param name="rushSpeed">突進の速さ(1フレームあたりのZ移動量)</param>
	/// <param name="returnSpeed">奥へ戻る速さ</param>
	BossAttackRush(float telegraphTime, float rushSpeed, float returnSpeed)
		: telegraphTime_(telegraphTime), rushSpeed_(rushSpeed), returnSpeed_(returnSpeed) {}
	void Update(BossEnemy& boss) override;

private:
	// 突進の進行段階
	enum class Step { kTelegraph, kRush, kReturn };
	Step step_ = Step::kTelegraph;

	float telegraphTime_;
	float rushSpeed_;
	float returnSpeed_;
	float timer_ = 0.0f;
	// 突進開始前のZ座標
	float homeZ_ = 0.0f;
	bool homeSaved_ = false;
};

// 叩きつけ：プレイヤーと同じX/Zまで移動し、少し高い位置から真下へ落ちてくる
class BossAttackSlam : public BossAttackState
{
public:
	/// <param name="approachTime">プレイヤーの真上へ移動する時間(秒)</param>
	/// <param name="aimTime">真上で狙いを定める時間(秒)</param>
	/// <param name="heightAbove">プレイヤーから何マス上に構えるか</param>
	/// <param name="fallAccel">落下の加速度</param>
	/// <param name="recoverTime">着地後の硬直時間(秒)</param>
	BossAttackSlam(float approachTime, float aimTime, float heightAbove,
		float fallAccel, float recoverTime)
		: approachTime_(approachTime), aimTime_(aimTime), heightAbove_(heightAbove),
		fallAccel_(fallAccel), recoverTime_(recoverTime) {}
	void Update(BossEnemy& boss) override;

private:
	// 叩きつけの進行段階
	enum class Step { kApproach, kAim, kFall, kRecover };
	Step step_ = Step::kApproach;

	float approachTime_;
	float aimTime_;
	float heightAbove_;
	float fallAccel_;
	float recoverTime_;
	float timer_ = 0.0f;
	float fallSpeed_ = 0.0f;
	// 攻撃開始前の位置
	Vector3 homePos_ = { 0.0f, 0.0f, 0.0f };
	bool homeSaved_ = false;
};