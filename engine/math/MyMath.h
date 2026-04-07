#pragma once
#include<vector>
#include<cmath>
#include<string>
#pragma region ベクトル
struct Vector2 {
	float x;
	float y;
};
struct Vector3 {
    float x;
    float y;
    float z;

    // 加算演算子
    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    // 減算演算子
    Vector3 operator-(const Vector3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    // スカラー乗算演算子
    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    // スカラー除算演算子
    Vector3 operator/(float scalar) const {
        return { x / scalar, y / scalar, z / scalar };
    }

    // 加算代入演算子
    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    // 減算代入演算子
    Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    // スカラー乗算代入演算子
    Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    // スカラー除算代入演算子
    Vector3& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // 等価演算子
    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    // 不等価演算子
    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }

    // ノルム（長さ）を取得
    float Length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // 正規化したベクトルを返す（長さ1にする）
    Vector3 Normalized() const {
        float len = Length();
        if (len == 0.0f) {
            return { 0.0f, 0.0f, 0.0f }; // 0除算回避
        }
        return { x / len, y / len, z / len };
    }
};

struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};
#pragma endregion
#pragma region 行列
struct Matrix3x3 {
	float m[3][3];
};
struct Matrix4x4 {
	float m[4][4];
};
#pragma endregion

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};
struct ModelData {
	std::vector<VertexData>vertices;
	MaterialData material;
	std::vector<uint32_t> indices;
};
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
struct DirectionalLight {
	Vector4 color; //ライトの色
	Vector3 direction; //ライトの向き
	float intensity;	//輝度
};
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};
struct SpriteData {
	Vector2 pos;
	Vector2 size;
	float rotate;
	Vector4 color;
};
//struct AccelerationField {
//	Vector3 acceleration;
//	AABB area;
//};

struct TextureVertex {
    Vector4 position; 
	Vector2 texcoord;
};

struct CameraForGPU {
    Vector3 worldPosition;
};

#pragma region パーティクル構造体
// パーティクル構造体

struct Particle
{
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};
struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};
struct EffectInstance
{
    Transform transform;     // 位置・回転・スケール
    Vector4 color;           // 色や透明度（フェード用）
    float lifeTime;          // 全体の寿命
    float currentTime;       // 経過時間
    bool isActive = true;    // アクティブ
};
struct Object3DInstanceData
{
    Vector4 world0;
    Vector4 world1;
    Vector4 world2;
    Vector4 world3;

    Vector4 wvp0;
    Vector4 wvp1;
    Vector4 wvp2;
    Vector4 wvp3;

    Vector4 color; 
};

enum BlendMode {
    //!< ブレンド無し
    kBlendModeNone,
    //!< 通常αブレンド デフォルト src * srcA + Dest * ( 1 - srcA )
    kBlendModeNormal,
    //!< 加算 src * srcA + Dest * 1;
    kBlendModeAdd,
    //!< 減算 Dest * 1 - src * srcA 
    kBlendModeSubtract,
    //!< 乗算 src * 0 + Dest * src
    kBlendModeMultiply,
    //!< スクリーン src * ( 1 - Dest )+ Dest * 1
    kBlendModeScreen,
    //!< 利用してはいけない
    kCountOfBlendMode,
};

#pragma endregion