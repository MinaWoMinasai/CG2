#pragma once
#include "Struct.h"
#include <cassert>
#include <random>

const float pi = 3.14159265f;

// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2);

// 減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);

// 減算
Vector4 Subtract(const Vector4& v1, const Vector4& v2);

// スカラー倍
Vector3 Multiply(float scalar, const Vector3& v);

// 内積
float Dot(const Vector3& v1, const Vector3& v2);

// 長さ(ノルム)
float Length(const Vector3& v);

// クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2);

// 正規化
Vector3 Normalize(const Vector3& v);

// ベクトル変換
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

// 行列の加算
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
// 行列の減産
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
// 行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
// 逆行列
Matrix4x4 Inverse(const Matrix4x4& m);
// 転置行列
Matrix4x4 Transpose(const Matrix4x4& m);
// 単位行列の作成
Matrix4x4 MakeIdentity4x4();

// 平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// 拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// 座標変換
Vector3 TransformMatrix(const Vector3& vector, const Matrix4x4& matrix);

Vector4 TransformMatrix(const Vector4& vector, const Matrix4x4& matrix);

// X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);
// Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);
// Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);

// 3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

// 透視投影行列
Matrix4x4 MakePerspectiveForMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
// 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
// ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
// LookAt行列
Matrix4x4 MakeLookAtMatrix(const Vector3& eye, const Vector3& target, const Vector3& up);

float Rand(float min, float max);
Vector3 Rand(const Vector3& min, const Vector3& max);
Vector4 Rand(const Vector4& min = {0.0f, 0.0f, 0.0f, 1.0f}, const Vector4& max = {1.0f, 1.0f, 1.0f, 1.0f});

// 演算子オーバーロード
inline Vector3 operator+(const Vector3& v1, const Vector3& v2) { return Add(v1, v2); }
inline Vector3 operator-(const Vector3& v1, const Vector3& v2) { return Subtract(v1, v2); }
inline Vector3 operator*(const float& s, const Vector3& v) { return Multiply(s, v); }
inline Vector3 operator*(const Vector3& v, const float& s) { return Multiply(s, v); }
inline Vector3 operator/(const Vector3& v, const float& s) { return Multiply(1.0f / s, v); }
inline Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) { return Add(m1, m2); }
inline Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) { return Subtract(m1, m2); }
inline Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) { return Multiply(m1, m2); }

// 単項演算子
inline Vector3 operator-(const Vector3& v) { return { -v.x, -v.y, -v.z }; }
inline Vector3 operator+(const Vector3& v) { return v; }