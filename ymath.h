#ifndef MATH_DEF
#define MATH_DEF
#include <stdio.h>
#include <cmath>
#include <float.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <memory>

// #define EPSILON FLT_EPSILON
#define EPSILON 0.0005
#define PI 3.14159265359

enum class Axis {
	X,
	Y,
	Z
};

class Vector2 {
public:
	float x, y;
	Vector2() : x(0), y(0) {}
	Vector2(float _x, float _y) : x(_x), y(_y) {}

	bool operator == (const Vector2 &v) const { return v.x == x && v.y == y; }
	bool operator != (const Vector2 &v) const { return v.x != x && v.y != y; }
	Vector2 &operator *= (float s){ x *= s; y *= s; return *this; }
	Vector2 operator * (float s) const { return Vector2(x*s, y*s); }
	Vector2 &operator *= (const Vector2 &v){ x *= v.x; y *= v.y; return *this; }
	Vector2 operator * (const Vector2 &v) const { return Vector2(x*v.x, y*v.y); }
	Vector2 &operator /= (float s){ x /= s; y /= s; return *this; }
	Vector2 operator / (float s) const { return Vector2(x / s, y / s); }
	Vector2 &operator /= (const Vector2 &v){ x /= v.x; y /= v.y; return *this; }
	Vector2 operator / (const Vector2 &v) const { return Vector2(x / v.x, y / v.y); }
	Vector2 &operator -= (float s){ x -= s; y -= s; return *this; }
	Vector2 operator - (float s) const { return Vector2(x - s, y - s); }
	Vector2 &operator -= (const Vector2 &v){ x -= v.x; y -= v.y; return *this; }
	Vector2 operator - (const Vector2 &v) const { return Vector2(x - v.x, y - v.y); }
	Vector2 &operator += (float s){ x += s; y += s; return *this; }
	Vector2 operator + (float s) const { return Vector2(x + s, y + s); }
	Vector2 &operator += (const Vector2 &v){ x += v.x; y += v.y; return *this; }
	Vector2 operator + (const Vector2 &v) const { return Vector2(x + v.x, y + v.y); }

	inline float magnitude() const { return sqrtf(powf(x, 2) + powf(y, 2)); }
	inline Vector2 &normalize() { float mag = magnitude(); if (!mag) return *this; x /= mag; y /= mag; return *this; }
	inline float dot(const Vector2 &v) const { return (x * v.x) + (y * v.y); }

	bool operator<(const Vector2 that) const{
		return x < that.x && y < that.y;
	}

	bool operator>(const Vector2 that) const{
		return x > that.x && y > that.y;
	}

	float &operator[] (int index){
		if (index == 0) return x;
		if (index == 1) return y;
		return x;
	}

	float operator[] (int index) const {
		if (index == 0) return x;
		if (index == 1) return y;
	}
};

class Vector3 {
public:
	float x, y, z;

	Vector3() : x(0), y(0), z(0) {}
	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	Vector3(float *xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]) {}

	bool operator == (const Vector3 &v) const { return v.x == x && v.y == y && v.z == z; }
	bool operator != (const Vector3 &v) const { return v.x != x && v.y != y && v.z != z; }
	Vector3 &operator *= (float s){ x *= s; y *= s; z *= s; return *this; }
	Vector3 operator * (float s) const { return Vector3(x*s, y*s, z*s); }
	Vector3 &operator *= (const Vector3 &v){ x *= v.x; y *= v.y; z *= v.z; return *this; }
	Vector3 operator * (const Vector3 &v) const { return Vector3(x*v.x, y*v.y, z*v.z); }
	Vector3 &operator /= (float s){ x /= s; y /= s; z /= s; return *this; }
	Vector3 operator / (float s) const { return Vector3(x / s, y / s, z / s); }
	Vector3 &operator /= (const Vector3 &v){ x /= v.x; y /= v.y; z /= v.z; return *this; }
	Vector3 operator / (const Vector3 &v) const { return Vector3(x / v.x, y / v.y, z / v.z); }
	Vector3 &operator -= (float s){ x -= s; y -= s; z -= s; return *this; }
	Vector3 operator - (float s) const { return Vector3(x - s, y - s, z - s); }
	Vector3 &operator -= (const Vector3 &v){ x -= v.x; y -= v.y; z -= v.z; return *this; }
	Vector3 operator - (const Vector3 &v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
	Vector3 &operator += (float s){ x += s; y += s; z += s; return *this; }
	Vector3 operator + (float s) const { return Vector3(x + s, y + s, z + s); }
	Vector3 &operator += (const Vector3 &v){ x += v.x; y += v.y; z += v.z; return *this; }
	Vector3 operator + (const Vector3 &v) const { return Vector3(x + v.x, y + v.y, z + v.z); }

	inline Vector3 abs(){ return Vector3(fabs(x), fabs(y), fabs(z)); }
	inline float dot(const Vector3 &v) const { return (x * v.x) + (y * v.y) + (z * v.z); }
	inline float dot(const Vector2 &v) const { return (x * v.x) + (y * v.y); }
	inline Vector3 cross(Vector3 v) const { return Vector3((y * v.z) - (z * v.y), (z * v.x) - (x * v.z), (x * v.y) - (y * v.x)); }
	inline float magnitude() const { return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2)); }
	inline Vector3 &normalize() { float mag = magnitude(); if (!mag) return *this; x /= mag; y /= mag; z /= mag; return *this; }

	bool operator<(const Vector3 that) const{
		return x < that.x && y < that.y && z < that.z;
	}

	bool operator>(const Vector3 that) const{
		return x > that.x && y > that.y && z > that.z;
	}

	float &operator[] (int index){
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;

		return x;
	}

	float operator[] (int index) const {
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;

		return 0;
	}
};

class Vector4 {
public:
	float x, y, z, w;
	Vector4() : x(0), y(0), z(0), w(0){}
	Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

	bool operator == (const Vector4 &v){ return v.x == x && v.y == y && v.z == z && v.w == w; }
	bool operator != (const Vector4 &v){ return v.x != x && v.y != y && v.z != z && v.w != w; }
	Vector4 &operator *= (float s){ x *= s; y *= s; z *= s; return *this; }
	Vector4 operator * (float s) const { return Vector4(x*s, y*s, z*s, w*s); }
	Vector4 &operator *= (const Vector4 &v){ x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	Vector4 operator * (const Vector4 &v) const { return Vector4(x*v.x, y*v.y, z*v.z, z*v.w); }
	Vector4 &operator /= (float s){ x /= s; y /= s; z /= s; w /= s; return *this; }
	Vector4 operator / (float s) const { return Vector4(x / s, y / s, z / s, w / s); }
	Vector4 &operator /= (const Vector4 &v){ x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
	Vector4 operator / (const Vector4 &v) const { return Vector4(x / v.x, y / v.y, z / v.z, w / v.w); }
	Vector4 &operator -= (float s){ x -= s; y -= s; z -= s; w -= s; return *this; }
	Vector4 operator - (float s) const { return Vector4(x - s, y - s, z - s, w - s); }
	Vector4 &operator -= (const Vector4 &v){ x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	Vector4 operator - (const Vector4 &v) const { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }
	Vector4 &operator += (float s){ x += s; y += s; z += s; w += s; return *this; }
	Vector4 operator + (float s) const { return Vector4(x + s, y + s, z + s, w + s); }
	Vector4 &operator += (const Vector4 &v){ x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	Vector4 operator + (const Vector4 &v) const { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }

	Vector3 xyz(){ return Vector3(x, y, z); }
	inline Vector4 conjugate() const { return Vector4(-x, -y, -z, w); }
	inline float dot(const Vector4 &v) const { return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w); }
	inline float dot(const Vector3 &v) const { return (x * v.x) + (y * v.y) + (z * v.z); }
	inline float dot(const Vector2 &v) const { return (x * v.x) + (y * v.y); }
	inline Vector4 cross(Vector4 v) const { return Vector4((y * v.z) - (z * v.y), (z * v.x) - (x * v.z), (x * v.y) - (y * v.x), 1); }
	inline float magnitude() const { return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2) + pow(w, 2)); }
	inline Vector4 &normalize() { float mag = magnitude(); if (!mag) return *this; x /= mag; y /= mag; z /= mag; w /= mag; return *this; }

	bool operator<(const Vector4 that) const{
		return x > that.x && y > that.y && z > that.z && w > that.w;
	}

	bool operator>(const Vector4 that) const{
		return x < that.x && y < that.y && z < that.z && w < that.w;
	}

	float &operator[] (int index){
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		if (index == 3) return w;
		return x;
	}

	float operator[] (int index) const {
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		if (index == 3) return w;
	}

};

class Quaternion {
public:
	float x, y, z, w;
	Quaternion() : x(0), y(0), z(0), w(1){}
	Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

	bool operator == (const Quaternion &v){ return v.x == x && v.y == y && v.z == z && v.w == w; }
	bool operator != (const Quaternion &v){ return v.x != x && v.y != y && v.z != z && v.w != w; }

	Quaternion operator * (const Quaternion &q2){
		Quaternion ret = {
			w * q2.x + x * q2.w + y * q2.z - z * q2.y,
			w * q2.y - x * q2.z + y * q2.w + z * q2.x,
			w * q2.z + x * q2.y - y * q2.x + z * q2.w,
			w * q2.w - x * q2.x - y * q2.y - z * q2.z,
		};
		return ret;
	}

	Quaternion &operator *= (const Quaternion &q2){
		*this = *this * q2;
		return *this;
	}

	Vector3 operator * (Vector3 v){
		Quaternion q1 = *this * Quaternion(v.x, v.y, v.z, 0);
		q1 = q1 * Quaternion(-x, -y, -z, w);

		return{ q1.x, q1.y, q1.z };
	}

	Quaternion(Vector3 v, float a){
		this->x = v.x * sinf(a / 2);
		this->y = v.y * sinf(a / 2);
		this->z = v.z * sinf(a / 2);
		this->w = cosf(a / 2);
	}

	Quaternion &operator /= (float s){ x /= s; y /= s; z /= s; w /= s; return *this; }
	Quaternion operator / (float s) const { return Quaternion(x / s, y / s, z / s, w / s); }
	Vector3 xyz(){ return Vector3(x, y, z); }
	inline Quaternion conjugate() const { return Quaternion(-x, -y, -z, w); }
	inline Quaternion inverse() const { return conjugate() / pow(magnitude(), 2); }
	inline float magnitude() const { return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2) + pow(w, 2)); }
	inline Quaternion &normalize() { float mag = magnitude(); if (!mag) return *this; *this /= mag; return *this; }

	bool operator<(const Quaternion that) const{
		return x > that.x && y > that.y && z > that.z && w > that.w;
	}

	bool operator>(const Quaternion that) const{
		return x < that.x && y < that.y && z < that.z && w < that.w;
	}

	float &operator[] (int index){
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		if (index == 3) return w;
		return x;
	}

	float operator[] (int index) const {
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		if (index == 3) return w;
	}
};


class Matrix3 {
public:

	float m[3][3];

	Matrix3(){
	}

	Matrix3(const float mat[3][3]){
		memcpy(&m[0][0], &mat[0][0], 9 * sizeof(float));
	}

	Matrix3(const Matrix3& mat){
		memcpy(&m[0][0], &mat.m[0][0], 9 * sizeof(float));
	}

	Matrix3(float mat00, float mat01, float mat02,
		float mat10, float mat11, float mat12,
		float mat20, float mat21, float mat22){
		m[0][0] = mat00;
		m[0][1] = mat01;
		m[0][2] = mat02;
		m[1][0] = mat10;
		m[1][1] = mat11;
		m[1][2] = mat12;
		m[2][0] = mat20;
		m[2][1] = mat21;
		m[2][2] = mat22;
	}

	float determinant() const{
		const float cof00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
		const float cof10 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
		const float cof20 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

		return m[0][0] * cof00 + m[0][1] * cof10 + m[0][2] * cof20;
	}

	Matrix3 inverse() const {
		Matrix3 res(m[1][1] * m[2][2] - m[1][2] * m[2][1],
			m[0][2] * m[2][1] - m[0][1] * m[2][2],
			m[0][1] * m[1][2] - m[0][2] * m[1][1],
			m[1][2] * m[2][0] - m[1][0] * m[2][2],
			m[0][0] * m[2][2] - m[0][2] * m[2][0],
			m[0][2] * m[1][0] - m[0][0] * m[1][2],
			m[1][0] * m[2][1] - m[1][1] * m[2][0],
			m[0][1] * m[2][0] - m[0][0] * m[2][1],
			m[0][0] * m[1][1] - m[0][1] * m[1][0]);

		const float det = m[0][0] * res[0][0] + m[0][1] * res[1][0] + m[0][2] * res[2][0];

		if (fabs(det)>EPSILON){
			res /= det;
			return res;
		}
		else
			return ZERO;
	}

	Matrix3 transpose() const{
		Matrix3 res;
		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				res.m[j][i] = m[i][j];
		}
		return res;
	}

	void from_zyx(float z, float y, float x){

		const float  cz = cos(z);
		const float  sz = sin(z);
		const float  cy = cos(y);
		const float  sy = sin(y);
		const float  cx = cos(x);
		const float  sx = sin(x);
		const float  sxsy = sx*sy;
		const float  cxsy = cx*sy;

		m[0][0] = cy*cz;
		m[0][1] = sxsy*cz - cx*sz;
		m[0][2] = cxsy*cz + sx*sz;

		m[1][0] = cy*sz;
		m[1][1] = sxsy*sz + cx*cz;
		m[1][2] = cxsy*sz - sx*cz;

		m[2][0] = -sy;
		m[2][1] = sx*cy;
		m[2][2] = cx*cy;

	}

	Matrix3 operator+(const Matrix3& mat) const {
		Matrix3 res;
		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				res.m[i][j] = m[i][j] + mat.m[i][j];
		}
		return res;

	}

	Matrix3 operator-(const Matrix3& mat) const{
		Matrix3 res;
		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				res.m[i][j] = m[i][j] - mat.m[i][j];
		}
		return res;
	}

	Matrix3 operator*(const Matrix3& mat) const{

		Matrix3 res;

		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				res.m[i][j] = m[i][0] * mat.m[0][j] +
				m[i][1] * mat.m[1][j] +
				m[i][2] * mat.m[2][j];
		}
		return res;

	}

	Matrix3 operator*(float scalar) const {
		Matrix3 res;
		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				res.m[i][j] = scalar*m[i][j];
		}
		return res;

	}
	Matrix3 operator/(float scalar) const{
		if (scalar != (float)0.0) {
			Matrix3 res;
			const float invscalar = ((float)1.0) / scalar;
			for (int i = 0; i<3; i++) {
				for (int j = 0; j<3; j++)
					res.m[i][j] = invscalar*m[i][j];
			}
			return res;
		}

		else
			return MAX_REAL;

	}

	Matrix3 &operator+=(const Matrix3& mat){
		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				m[i][j] += mat.m[i][j];
		}
		return *this;
	}

	Matrix3 &operator-=(const Matrix3& mat){
		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				m[i][j] -= mat.m[i][j];
		}
		return *this;
	}

	Matrix3 &operator*=(float scalar){
		for (int i = 0; i<3; i++){
			for (int j = 0; j<3; j++)
				m[i][j] *= scalar;
		}
		return *this;
	}

	Matrix3 &operator/=(float scalar){
		if (scalar != (float)0.0){
			const float invscalar = ((float)1.0) / scalar;
			for (int i = 0; i<3; i++){
				for (int j = 0; j<3; j++)
					m[i][j] *= invscalar;
			}
			return *this;
		}
		else{
			*this = MAX_REAL;
			return *this;
		}
	}

	Vector3 operator*(const Vector3& vec) const {
		Vector3 res;
		for (int i = 0; i<3; i++)
			res[i] = m[i][0] * vec[0] + m[i][1] * vec[1] + m[i][2] * vec[2];
		return res;
	}

	bool operator==(const Matrix3& mat) const{
		return memcmp(m, mat.m, 9 * sizeof(float)) == 0;
	}

	bool operator!=(const Matrix3& mat) const{
		return memcmp(m, mat.m, 9 * sizeof(float)) != 0;
	}

	const float* operator[](int i) const{
		if (0 <= i && i<3)
			return &m[i][0];
		return nullptr;
	}

	float* operator[](int i){
		if (0 <= i && i<3)
			return &m[i][0];
		return nullptr;
	}

	operator const float*() const{
		return (const float*)m;
	}

	operator float*(){
		return (float*)m;
	}

	static const Matrix3 ZERO;
	static const Matrix3 IDENTITY;
	static const Matrix3 MAX_REAL;
};

class Matrix4 {
public:

	float m[16];

	Matrix4(){
		Identity();
	}

	Matrix4(float *_m) { memcpy(m, _m, 16 * sizeof(float)); }
	Matrix4(std::initializer_list<float> l) { if (l.size() < 16) return; std::copy(l.begin(), l.end(), m); }

	Matrix4 &Ortho(float l, float r, float t, float b, float n, float f){
		float Matrix4[16] = {
			2 / (r - l), 0, 0, -((r + l) / (r - l)),
			0, 2 / (t - b), 0, -((t + b) / (t - b)),
			0, 0, (-2) / (f - n), ((f + n) / (f - n)),
			0, 0, 0, 1
		};

		memcpy(m, Matrix4, sizeof(Matrix4));
		return *this;
	}

	Matrix4 &Ortho(float w, float h){
		Identity();
		m[0] = 2.0f / w;
		m[5] = -2.0f / h;
		m[3] = -1.0f;
		m[7] = 1.0f;
		m[10] = 0.0f;
		return *this;
	}

	Matrix4 &Perspective(float fov, float a, float n, float f){
		float Matrix4[4][4] = {};

		float tanHalfFov = tan(fov * 0.5f);

		Matrix4[0][0] = 1.0f / (a * tanHalfFov);
		Matrix4[1][1] = 1.0f / tanHalfFov;
		Matrix4[2][2] = f / (n - f);
		Matrix4[3][2] = -1.0f;
		Matrix4[2][3] = (f * n) / (n - f);
		Matrix4[3][3] = 0.0f;

		memcpy(m, Matrix4, sizeof(Matrix4));
		return *this;
	}

	float &At(int y, int x){
		return m[(y * 4) + x];
	}

	float operator [] (int index){
		return m[index];
	}

	Vector4 operator *(Vector4 vert){
		Vector4 out;
		out.x = ((vert.x * m[0]) + (vert.y * m[1]) + (vert.z * m[2]) + (vert.w * m[3]));
		out.y = ((vert.x * m[4]) + (vert.y * m[5]) + (vert.z * m[6]) + (vert.w * m[7]));
		out.z = ((vert.x * m[8]) + (vert.y * m[9]) + (vert.z * m[10]) + (vert.w * m[11]));
		out.w = ((vert.x * m[12]) + (vert.y * m[13]) + (vert.z * m[14]) + (vert.w * m[15]));
		return out;
	}

	Matrix4 operator* (Matrix4 b){
		float r[16] = {
			(m[0] * b.m[0]) + (m[1] * b.m[4]) + (m[2] * b.m[8]) + (m[3] * b.m[12]),
			(m[0] * b.m[1]) + (m[1] * b.m[5]) + (m[2] * b.m[9]) + (m[3] * b.m[13]),
			(m[0] * b.m[2]) + (m[1] * b.m[6]) + (m[2] * b.m[10]) + (m[3] * b.m[14]),
			(m[0] * b.m[3]) + (m[1] * b.m[7]) + (m[2] * b.m[11]) + (m[3] * b.m[15]),
			(m[4] * b.m[0]) + (m[5] * b.m[4]) + (m[6] * b.m[8]) + (m[7] * b.m[12]),
			(m[4] * b.m[1]) + (m[5] * b.m[5]) + (m[6] * b.m[9]) + (m[7] * b.m[13]),
			(m[4] * b.m[2]) + (m[5] * b.m[6]) + (m[6] * b.m[10]) + (m[7] * b.m[14]),
			(m[4] * b.m[3]) + (m[5] * b.m[7]) + (m[6] * b.m[11]) + (m[7] * b.m[15]),
			(m[8] * b.m[0]) + (m[9] * b.m[4]) + (m[10] * b.m[8]) + (m[11] * b.m[12]),
			(m[8] * b.m[1]) + (m[9] * b.m[5]) + (m[10] * b.m[9]) + (m[11] * b.m[13]),
			(m[8] * b.m[2]) + (m[9] * b.m[6]) + (m[10] * b.m[10]) + (m[11] * b.m[14]),
			(m[8] * b.m[3]) + (m[9] * b.m[7]) + (m[10] * b.m[11]) + (m[11] * b.m[15]),
			(m[12] * b.m[0]) + (m[13] * b.m[4]) + (m[14] * b.m[8]) + (m[15] * b.m[12]),
			(m[12] * b.m[1]) + (m[13] * b.m[5]) + (m[14] * b.m[9]) + (m[15] * b.m[13]),
			(m[12] * b.m[2]) + (m[13] * b.m[6]) + (m[14] * b.m[10]) + (m[15] * b.m[14]),
			(m[12] * b.m[3]) + (m[13] * b.m[7]) + (m[14] * b.m[11]) + (m[15] * b.m[15]),
		};

		return Matrix4(r);
	}

	Vector3 operator*(const Vector3 &vert){
		Vector3 out;
		out.x = ((vert.x * m[0]) + (vert.y * m[1]) + (vert.z * m[2]) + (1 * m[3]));
		out.y = ((vert.x * m[4]) + (vert.y * m[5]) + (vert.z * m[6]) + (1 * m[7]));
		out.z = ((vert.x * m[8]) + (vert.y * m[9]) + (vert.z * m[10]) + (1 * m[11]));
		return out;
	}

	Matrix4 &Identity(){
		memset(m, 0, sizeof(m));
		m[0] = 1;
		m[5] = 1;
		m[10] = 1;
		m[15] = 1;
		return *this;
	}

	Matrix4 &Inverse(){
		float m00 = m[0], m01 = m[1], m02 = m[2], m03 = m[3];
		float m10 = m[4], m11 = m[5], m12 = m[6], m13 = m[7];
		float m20 = m[8], m21 = m[9], m22 = m[10], m23 = m[11];
		float m30 = m[12], m31 = m[13], m32 = m[14], m33 = m[15];

		m[0] = m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33;
		m[1] = m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33;
		m[2] = m02*m13*m31 - m03*m12*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33;
		m[3] = m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23;
		m[4] = m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33;
		m[5] = m02*m23*m30 - m03*m22*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33;
		m[6] = m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33;
		m[7] = m02*m13*m20 - m03*m12*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23;
		m[8] = m11*m23*m30 - m13*m21*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33;
		m[9] = m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33;
		m[10] = m01*m13*m30 - m03*m11*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33;
		m[11] = m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23;
		m[12] = m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32;
		m[13] = m01*m22*m30 - m02*m21*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32;
		m[14] = m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32;
		m[15] = m01*m12*m20 - m02*m11*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22;
		float det = (m00 * m[0]) + (m01 * m[4]) + (m02 * m[8]) + (m03 * m[12]);
		Scale(1 / det);
		return *this;
	}

	Matrix4 &Scale(const float amount){
		for (int k = 0; k < 16; k++) m[k] *= amount;
		return *this;
	}

	float operator[](const int index) const {
		if (index < 0 || index > 16) return 0.0f;
		return m[index];
	}

	Matrix4 &operator =(float cpyM[16]) {
		memcpy(m, cpyM, 16 * sizeof(float));
		return *this;
	}

	Matrix4 operator +(const Matrix4 &m1) const {
		float retM[16] = {
			m[0] + m1[0], m[1] + m1[1], m[2] + m1[2], m[3] + m1[3],
			m[4 + 0] + m1[4 + 0], m[4 + 1] + m1[4 + 1], m[4 + 2] + m1[4 + 2], m[4 + 3] + m1[4 + 3],
			m[8 + 0] + m1[8 + 0], m[8 + 1] + m1[8 + 1], m[8 + 2] + m1[8 + 2], m[8 + 3] + m1[8 + 3],
			m[12 + 0] + m1[12 + 0], m[12 + 1] + m1[12 + 1], m[12 + 2] + m1[12 + 2], m[12 + 3] + m1[12 + 3],
		};
		Matrix4 ret = retM;
		return ret;
	}

	Matrix4 &operator +=(const Matrix4 &m1) { *this = *this + m1; return *this; }


	Matrix4 operator -(const Matrix4 &m1) const {
		float retM[16] = {
			m[0] - m1[0], m[1] - m1[1], m[2] - m1[2], m[3] - m1[3],
			m[4 + 0] - m1[4 + 0], m[4 + 1] - m1[4 + 1], m[4 + 2] - m1[4 + 2], m[4 + 3] - m1[4 + 3],
			m[8 + 0] - m1[8 + 0], m[8 + 1] - m1[8 + 1], m[8 + 2] - m1[8 + 2], m[8 + 3] - m1[8 + 3],
			m[12 + 0] - m1[12 + 0], m[12 + 1] - m1[12 + 1], m[12 + 2] - m1[12 + 2], m[12 + 3] - m1[12 + 3],
		};

		Matrix4 ret = retM;
		return ret;
	}

	Matrix4 &operator -=(const Matrix4 &m1) { *this = *this - m1; return *this; }

	Matrix4 operator *(const Matrix4 &b) const {
		float retM[16] = {
			(m[0] * b[0]) + (m[1] * b[4]) + (m[2] * b[8]) + (m[3] * b[12]),
			(m[0] * b[1]) + (m[1] * b[5]) + (m[2] * b[9]) + (m[3] * b[13]),
			(m[0] * b[2]) + (m[1] * b[6]) + (m[2] * b[10]) + (m[3] * b[14]),
			(m[0] * b[3]) + (m[1] * b[7]) + (m[2] * b[11]) + (m[3] * b[15]),
			(m[4] * b[0]) + (m[5] * b[4]) + (m[6] * b[8]) + (m[7] * b[12]),
			(m[4] * b[1]) + (m[5] * b[5]) + (m[6] * b[9]) + (m[7] * b[13]),
			(m[4] * b[2]) + (m[5] * b[6]) + (m[6] * b[10]) + (m[7] * b[14]),
			(m[4] * b[3]) + (m[5] * b[7]) + (m[6] * b[11]) + (m[7] * b[15]),
			(m[8] * b[0]) + (m[9] * b[4]) + (m[10] * b[8]) + (m[11] * b[12]),
			(m[8] * b[1]) + (m[9] * b[5]) + (m[10] * b[9]) + (m[11] * b[13]),
			(m[8] * b[2]) + (m[9] * b[6]) + (m[10] * b[10]) + (m[11] * b[14]),
			(m[8] * b[3]) + (m[9] * b[7]) + (m[10] * b[11]) + (m[11] * b[15]),
			(m[12] * b[0]) + (m[13] * b[4]) + (m[14] * b[8]) + (m[15] * b[12]),
			(m[12] * b[1]) + (m[13] * b[5]) + (m[14] * b[9]) + (m[15] * b[13]),
			(m[12] * b[2]) + (m[13] * b[6]) + (m[14] * b[10]) + (m[15] * b[14]),
			(m[12] * b[3]) + (m[13] * b[7]) + (m[14] * b[11]) + (m[15] * b[15]),
		};

		Matrix4 ret = retM;
		return ret;
	}

	Matrix4 &Translate(const Vector3 &vector) {
		m[3] = vector.x;
		m[7] = vector.y;
		m[11] = vector.z;
		return *this;
	}

	Matrix4 &operator *=(const Matrix4 &m1) { *this = *this * m1; return *this; }
};

typedef struct {
	Vector3 pos;
	Vector3 line;
} Ray;

typedef struct {
	float width;
	float length;
	float rotation;
	float x;
	float y;
	float z;
} Triangle;

class Rect {
public:
	float x, y, z, width, height;
	Rect() : x(0), y(0), z(0), width(0), height(0) {}
	inline Rect(float _x, float _y, float _z, float _w, float _h) : x(_x), y(_y), z(_z), width(_w), height(_h) {}
	inline Rect(const Vector3 &pos, float _w, float _h) : x(pos.x), y(pos.y), z(pos.z), width(_w), height(_h) {}
	float CheckCollisionRay(Ray ray) const;
	inline Vector2 xy() const { return Vector2(x, y); }
	inline Vector3 xyz() const { return Vector3(x, y, z); }
	inline Vector2 wh() const { return Vector2(width, height); }

	inline bool CheckCollision(const Rect &r) const {
		if (x <= r.x + r.width  && width + x >= r.x &&
			y <= r.y + r.height && height + y >= r.y) return true;

		return false;
	}

	inline bool IsCompletelyInside(const Rect &r) const {
		if (x >= r.x && x + width <= r.x + r.width &&
			y >= r.y && y + height <= r.y + r.height) return true;

		return false;
	}
};

// class Joint {
// public:

//     // Joint(float m, Joint *p): mag(m), parent(p){}
//     Joint(Vector3 sPos, Vector3 ePos, Quaternion quat, Joint *p, float mag):
//     startingPos(sPos),
//     endingPos(ePos),
//     parent(p),
//     origEndingPos(ePos),
//     origStartingPos(sPos),
//     mag(mag),
//     T(quat),
//     savedT(quat)
//     {
//         limitsSet[0] = 0;
//         limitsSet[1] = 0;
//         limitsSet[2] = 0;
//     }

//     Vector3 startingPos;
//     Vector3 endingPos;
//     Joint *parent;
//     Vector3 origEndingPos;
//     Vector3 origStartingPos;

//     float mag;

//     void ValidateRotation();
//     void SetLimit(int idx, float min, float max);
//     Vector2 GetLimit(int idx){ return limits[idx]; }
//     bool IsLimited(int idx){ return limitsSet[idx]; }
//     Vector3 GetEndPoint(){ return T*Vector3(0, mag, 0); }
//     // Vector3 GetEndPoint(){ return T*(endingPos-startingPos); }
//     Vector3 GetRight() { return T*Vector3(1,0,0); }
//     Vector3 GetUp() { return T*Vector3(0,1,0); }
//     Vector3 GetForward() { return T*Vector3(0,0,-1); }
//     void ApplyAngleChange(float theta, const Vector3 &axis);
//     void SaveTransformation(){ savedT = T; }
//     void LoadTransformation(){ T = savedT; }
//     Quaternion T, savedT;
// private:
//     bool limitsSet[3];
//     Vector2 limits[3];

// };

// class Arm {
// public:

//     Arm():errorMargin(0.000005){}
//     Arm(Vector3 b, std::vector<std::shared_ptr<Joint>> j) : base(b), joints(j), errorMargin(0.000005){}

//     Vector3 ComputeJacobianSegment(int jointIndex, Axis axis);
//     Vector3 CalculateEndEffector(int joint = -1);
//     float GetMaxLength();
//     void Reset();
//     void Solve(Vector3 goal);
//     Vector3 base;
//     std::vector<std::shared_ptr<Joint>> joints;
//     float errorMargin;
// };


class Plane {
public:
	float a;
	float b;
	float c;
	float d;
	Plane() : a(0), b(0), c(0), d(0) {}
	Plane(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) {}
	float magnitude(){ return sqrt(a*a + b*b + c*c); }
	void normalize(){ float mag = magnitude(); a /= mag; b /= mag; c /= mag; d /= mag; }
	float DistanceToPoint(Vector3 point) { return a*point.x + b*point.y + c*point.z + d; }
	bool operator == (Plane v){ return v.a == a && v.b == b && v.c == c && v.d == d; }
	bool operator != (Plane v){ return v.a != a && v.b != b && v.c != c && v.d != d; }
	Plane &operator *= (float s){ a *= s; b *= s; c *= s; return *this; }
	Plane operator * (float s){ return Plane(a*s, b*s, c*s, d*s); }
	Plane &operator *= (Plane v){ a *= v.a; b *= v.b; c *= v.c; d *= v.d; return *this; }
	Plane operator * (Plane v){ return Plane(a*v.a, b*v.b, c*v.c, c*v.d); }
	Plane &operator /= (float s){ a /= s; b /= s; c /= s; d /= s; return *this; }
	Plane operator / (float s){ return Plane(a / s, b / s, c / s, d / s); }
	Plane &operator /= (Plane v){ a /= v.a; b /= v.b; c /= v.c; d /= v.d; return *this; }
	Plane operator / (Plane v){ return Plane(a / v.a, b / v.b, c / v.c, d / v.d); }
	Plane &operator -= (float s){ a -= s; b -= s; c -= s; d -= s; return *this; }
	Plane operator - (float s){ return Plane(a - s, b - s, c - s, d - s); }
	Plane &operator -= (Plane v){ a -= v.a; b -= v.b; c -= v.c; d -= v.d; return *this; }
	Plane operator - (Plane v){ return Plane(a - v.a, b - v.b, c - v.c, d - v.d); }
	Plane &operator += (float s){ a += s; b += s; c += s; d += s; return *this; }
	Plane operator + (float s){ return Plane(a + s, b + s, c + s, d + s); }
	Plane &operator += (Plane v){ a += v.a; b += v.b; c += v.c; d += v.d; return *this; }
	Plane operator + (Plane v){ return Plane(a + v.a, b + v.b, c + v.c, d + v.d); }
};

Quaternion Math_QuatLookAt(Vector3 &forward, Vector3 &up);
Vector3 Math_RotateMatrixToEuler(float *m);
float Math_Clamp(float m, float min, float max);
void Math_InverseMatrixNxN(float *mat, int n);
float Math_Determinant(float *mat, int n);
void Math_Mat4ToMat3(float *mat4, float *mat3);
void Math_InverseMatrixMat3(float *mat3);
void Math_TransposeMatrix3x3(float *matrix);
Quaternion Math_MatrixToQuat(float *matrix);
void Math_OuterProduct(Vector3 vec, Vector3 trans, float *matrix);
void Math_RotateMatrix(float *matrix, Vector3 angles);
void Math_Perspective(float *matrix, float fov, float a, float n, float f);
void Math_Ortho(float *matrix, float l, float r, float t, float b, float n, float f);
void Math_MatrixMatrixMult(float *res, float *a, float *b);
void Math_LookAt(float *ret, Vector3 eye, Vector3 center, Vector3 up);
void Math_RotateAroundAxis(Vector3 p, float a, float *);
void Math_MatrixFromQuat(Quaternion q, float*);
Quaternion Math_Slerp(Quaternion q, Quaternion q2, float);
Vector3 Math_Lerp(Vector3 a1, Vector3 a2, float t);
Vector3 Math_MatrixMult(Vector3, float*);
Vector4 Math_MatrixMult4(Vector4, float*);
Quaternion Math_QuatMult(Quaternion q1, Quaternion q2);
void Math_CopyMatrix(float *m1, float *m2);
void Math_InverseMatrix(float *m);
void Math_ScaleMatrix(float *matrix, int n, float amount);
void Math_ScalingMatrixXYZ(float *matrix, const Vector3 &amount);
Vector3 Math_Rotate(Vector3 pos, Vector3 angles);
void Math_TranslateMatrix(float *matrix, Vector3 vector);
void Math_ScalingMatrix(float *matrix, float amount);
void Math_MatrixMatrixAdd(float *matrix, float *m0, float *m1);
void Math_MatrixMatrixSub(float *matrix, float *m0, float *m1);
Quaternion Math_Quat(Vector3 v, float a);
Vector3 Math_QuatRotate(Quaternion q, Vector3 v);
// void Math_Ik(std::vector<Joint> &joints, Vector3 &goal);
Vector3 Math_QuatToAxisAngle(Quaternion quat, float *angle);
Vector3 Math_AxisAngleToEuler(Vector3 axis, float angle);
Vector3 Math_QuatToEuler(Quaternion quat);
Quaternion Math_EulerToQuat(const Vector3 euler);
// Vector3 Math_RotateMatrixToEuler(float *m);
float Math_GetAngleBetweenQuats(Quaternion &qa, Quaternion &qb, const Vector3 &n);

#endif