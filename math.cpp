#include "stdafx.h"
#include "ymath.h"
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <float.h>
// #include <Eigen/Geometry>

const Matrix3 Matrix3::ZERO(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
const Matrix3 Matrix3::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
const Matrix3 Matrix3::MAX_REAL(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);

void Math_Ortho(float *matrix, float l, float r, float t, float b, float n, float f){
	float m[16] = {
		2 / (r - l), 0, 0, -((r + l) / (r - l)),
		0, 2 / (t - b), 0, -((t + b) / (t - b)),
		0, 0, 1 / (f - n), n / (f - n),
		0, 0, 0, 1
	};

	memcpy(matrix, m, sizeof(m));
}

void Math_Perspective(float *matrix, float fov, float a, float n, float f){
	float m[4][4] = {};

	float tanHalfFov = tan(fov * 0.5f);

	m[0][0] = 1.0f / (a * tanHalfFov);
	m[1][1] = 1.0f / tanHalfFov;
	m[2][2] = f / (n - f);
	m[3][2] = -1.0f;
	m[2][3] = (f * n) / (n - f);
	m[3][3] = 0.0f;
	memcpy(matrix, m, sizeof(m));
}

// void Math_MatrixMultNxN(float *ret, float *a, const Vector2 &aSize, float *b, const Vector2 &bSize){

//     int m = (int)aSize.x, n = (int)aSize.y;
//     const int p = (int)bSize.x, q = (int)bSize.y;

//     float first[m][n];
//     float second[p][q];

//     memcpy(&first[0][0], a, sizeof(first));
//     memcpy(&second[0][0], b, sizeof(second));

//     float multiply[m][q];

//     for (int c = 0; c < m; c++) {
//         for (int d = 0; d < q; d++) {

//             float sum = 0;

//             for (int k = 0; k < p; k++)
//                 sum += first[c][k] * second[k][d];

//             multiply[c][d] = sum;
//         }
//     }

//     memcpy(ret, multiply, sizeof(multiply));
// }

void Math_MatrixMatrixMult(float *res, float *a, float *b){

	float r[16] = {
		(a[0] * b[0]) + (a[1] * b[4]) + (a[2] * b[8]) + (a[3] * b[12]),
		(a[0] * b[1]) + (a[1] * b[5]) + (a[2] * b[9]) + (a[3] * b[13]),
		(a[0] * b[2]) + (a[1] * b[6]) + (a[2] * b[10]) + (a[3] * b[14]),
		(a[0] * b[3]) + (a[1] * b[7]) + (a[2] * b[11]) + (a[3] * b[15]),
		(a[4] * b[0]) + (a[5] * b[4]) + (a[6] * b[8]) + (a[7] * b[12]),
		(a[4] * b[1]) + (a[5] * b[5]) + (a[6] * b[9]) + (a[7] * b[13]),
		(a[4] * b[2]) + (a[5] * b[6]) + (a[6] * b[10]) + (a[7] * b[14]),
		(a[4] * b[3]) + (a[5] * b[7]) + (a[6] * b[11]) + (a[7] * b[15]),
		(a[8] * b[0]) + (a[9] * b[4]) + (a[10] * b[8]) + (a[11] * b[12]),
		(a[8] * b[1]) + (a[9] * b[5]) + (a[10] * b[9]) + (a[11] * b[13]),
		(a[8] * b[2]) + (a[9] * b[6]) + (a[10] * b[10]) + (a[11] * b[14]),
		(a[8] * b[3]) + (a[9] * b[7]) + (a[10] * b[11]) + (a[11] * b[15]),
		(a[12] * b[0]) + (a[13] * b[4]) + (a[14] * b[8]) + (a[15] * b[12]),
		(a[12] * b[1]) + (a[13] * b[5]) + (a[14] * b[9]) + (a[15] * b[13]),
		(a[12] * b[2]) + (a[13] * b[6]) + (a[14] * b[10]) + (a[15] * b[14]),
		(a[12] * b[3]) + (a[13] * b[7]) + (a[14] * b[11]) + (a[15] * b[15]),
	};

	memcpy(res, r, sizeof(r));
}

float Rect::CheckCollisionRay(Ray ray) const {

	float ret = FLT_MAX;

	Vector2 norms[4];
	Vector3 points[4];

	points[0] = { x, y, z };
	points[1] = { x + width, y, z };
	points[2] = { x + width, y + height, z };
	points[3] = { x, y + height, z };
	norms[0] = { 0, 1 };
	norms[1] = { 1, 0 };
	norms[2] = { 0, -1 };
	norms[3] = { -1, 0 };

	for (int k = 0; k < 4; k++){

		float d = (points[k] - ray.pos).dot(norms[k]) / (ray.line.dot(norms[k]));
		if (d < 0) continue;

		Vector3 collisionPoint = ray.pos + (ray.line*d);

		if (collisionPoint.x >= x && collisionPoint.x <= x + width && collisionPoint.y >= y && collisionPoint.y <= y + height)
			if (d < ret) ret = d;
	}

	return ret;
}

void Math_LookAt(float *ret, Vector3 eye, Vector3 center, Vector3 up){

	Vector3 z = (eye - center).normalize();  // Forward
	Vector3 x = up.cross(z).normalize(); // Right
	Vector3 y = z.cross(x);

	float m[] = { x.x, x.y, x.z, -(x.dot(eye)),
		y.x, y.y, y.z, -(y.dot(eye)),
		z.x, z.y, z.z, -(z.dot(eye)),
		0, 0, 0, 1 };
	memcpy(ret, m, sizeof(m));
}

void Math_OuterProduct(Vector3 v, Vector3 t, float *matrix){
	float m[16] = {
		v.x*t.x, v.x*t.y, v.x*t.z, 0,
		v.y*t.x, v.y*t.y, v.y*t.z, 0,
		v.z*t.x, v.z*t.y, v.z*t.z, 0,
		0, 0, 0, 1,
	};
	memcpy(matrix, m, sizeof(m));
}

// static float determinant(float *a, int k, int originalSize)
// {
//     float s = 1, det = 0, b[originalSize][originalSize];

//     int i, j, m, n, c;
//     if (k == 1) return (a[0]);

//     det = 0;
//     for (c = 0; c<k; c++){
//         m = 0;
//         n = 0;
//         for (i = 0; i<k; i++){
//             for (j = 0; j<k; j++){
//                 b[i][j] = 0;
//                 if (i != 0 && j != c){
//                     b[m][n] = a[(i*originalSize) + j];
//                     if (n<(k - 2))
//                         n++;
//                     else{
//                         n = 0;
//                         m++;
//                     }
//                 }
//             }
//         }
//         det = det + s * (a[c] * determinant(&b[0][0], k - 1, originalSize));
//         s = -1 * s;
//     }

//     return (det);
// }

// static void transpose(float *ret, float *mat, float *fac, int r, int originalSize){

//     int i, j;

//     float b[r][r], d;

//     for (i = 0; i<r; i++){
//         for (j = 0; j<r; j++){
//             b[i][j] = fac[(j*r) + i];
//         }
//     }

//     d = determinant(mat, r, originalSize);

//     for (i = 0; i<r; i++){
//         for (j = 0; j<r; j++){
//             ret[(i*(int)r) + j] = b[i][j] / d;
//         }
//     }
// }

// static void InverseNxN(float *ret, float *mat, int f, int originalSize){

//     float b[f][f], fac[f][f];

//     int p, q, m, n, i, j;

//     for (q = 0; q<f; q++){
//         for (p = 0; p<f; p++){
//             m = 0;
//             n = 0;
//             for (i = 0; i<f; i++){
//                 for (j = 0; j<f; j++){
//                     if (i != q && j != p){
//                         b[m][n] = mat[(i*originalSize) + j];
//                         if (n<(f - 2))
//                             n++;
//                         else{
//                             n = 0;
//                             m++;
//                         }
//                     }
//                 }
//             }

//             fac[q][p] = pow(-1, q + p) * determinant(&b[0][0], f - 1, f);
//         }
//     }
//     transpose(ret, mat, &fac[0][0], f, originalSize);
// }

// void Math_InverseMatrixNxN(float *mat, int n){
//     InverseNxN(mat, mat, n, n);
// }

void Math_InverseMatrix(float *m){
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
	Math_ScaleMatrix(m, 4, 1 / det);
}

void Math_Mat4ToMat3(float *mat4, float *mat3){
	mat3[0] = mat4[0];
	mat3[1] = mat4[1];
	mat3[2] = mat4[2];
	mat3[3] = mat4[4];
	mat3[4] = mat4[5];
	mat3[5] = mat4[6];
	mat3[6] = mat4[8];
	mat3[7] = mat4[9];
	mat3[8] = mat4[10];
}

void Math_InverseMatrixMat3(float *m){
	float matrix[16] = {
		m[0], m[1], m[2], 0,
		m[3], m[4], m[5], 0,
		m[6], m[7], m[8], 0,
		0, 0, 0, 1
	};
	Math_InverseMatrix(matrix);
	Math_Mat4ToMat3(matrix, m);
}

void Math_ScaleMatrix(float *matrix, int n, float amount){
	int k;
	for (k = 0; k < n*n; k++) matrix[k] *= amount;
}

void Math_ScalingMatrix(float *matrix, float amount){
	Math_ScalingMatrixXYZ(matrix, { amount, amount, amount });
}

void Math_ScalingMatrixXYZ(float *matrix, const Vector3 &amount){
	float m[16] = {
		amount.x, 0, 0, 0,
		0, amount.y, 0, 0,
		0, 0, amount.z, 0,
		0, 0, 0, 1 };

	memcpy(matrix, m, sizeof(m));
}

Vector3 Math_MatrixMult(Vector3 vert, float *matrix){
	Vector3 out;
	out.x = ((vert.x * matrix[0]) + (vert.y * matrix[1]) + (vert.z * matrix[2]) + (1 * matrix[3]));
	out.y = ((vert.x * matrix[4]) + (vert.y * matrix[5]) + (vert.z * matrix[6]) + (1 * matrix[7]));
	out.z = ((vert.x * matrix[8]) + (vert.y * matrix[9]) + (vert.z * matrix[10]) + (1 * matrix[11]));
	return out;
}

Vector4 Math_MatrixMult4(Vector4 vert, float *matrix){
	Vector4 out;
	out.x = ((vert.x * matrix[0]) + (vert.y * matrix[1]) + (vert.z * matrix[2]) + (vert.w * matrix[3]));
	out.y = ((vert.x * matrix[4]) + (vert.y * matrix[5]) + (vert.z * matrix[6]) + (vert.w * matrix[7]));
	out.z = ((vert.x * matrix[8]) + (vert.y * matrix[9]) + (vert.z * matrix[10]) + (vert.w * matrix[11]));
	out.w = ((vert.x * matrix[12]) + (vert.y * matrix[13]) + (vert.z * matrix[14]) + (vert.w * matrix[15]));
	return out;
}

Vector3 Math_Lerp(Vector3 a1, Vector3 a2, double t){
	return (a1*(1.0 - t)) + (a2 * t);
}

Quaternion Math_Slerp(Quaternion qa, Quaternion qb, float t){
	Quaternion qm;
	float cosHalfTheta = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y + qa.z*qb.z;
	if (cosHalfTheta < 0){
		qb.x *= -1; qb.y *= -1; qb.z *= -1; qb.w *= -1;
		cosHalfTheta *= -1;
	}
	if (fabs(cosHalfTheta) >= 1.0f){
		qm.w = qa.w;
		qm.x = qa.x;
		qm.y = qa.y;
		qm.z = qa.z;
		return qm;
	}
	float halfTheta = acos(cosHalfTheta);
	float sinHalfTheta = sinf(halfTheta);
	if (fabs(sinHalfTheta) < 0.001f){
		qm.w = (qa.w * 0.5f + qb.w * 0.5f);
		qm.x = (qa.x * 0.5f + qb.x * 0.5f);
		qm.y = (qa.y * 0.5f + qb.y * 0.5f);
		qm.z = (qa.z * 0.5f + qb.z * 0.5f);
		return qm;
	}
	float ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
	float ratioB = sinf(t * halfTheta) / sinHalfTheta;
	qm.w = (qa.w * ratioA + qb.w * ratioB);
	qm.x = (qa.x * ratioA + qb.x * ratioB);
	qm.y = (qa.y * ratioA + qb.y * ratioB);
	qm.z = (qa.z * ratioA + qb.z * ratioB);
	return qm;
}

Quaternion Math_SlerpTheta(Quaternion qa, Quaternion qb, float theta){
	Quaternion qm;
	float cosHalfTheta = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y + qa.z*qb.z;
	if (cosHalfTheta < 0){
		qb.x *= -1; qb.y *= -1; qb.z *= -1; qb.w *= -1;
		cosHalfTheta *= -1;
	}
	if (fabs(cosHalfTheta) >= 1.0f){
		qm.w = qa.w;
		qm.x = qa.x;
		qm.y = qa.y;
		qm.z = qa.z;
		return qm;
	}
	float halfTheta = acos(cosHalfTheta);
	float sinHalfTheta = sinf(halfTheta);
	if (fabs(sinHalfTheta) < 0.001){
		qm.w = (qa.w * 0.5f + qb.w * 0.5f);
		qm.x = (qa.x * 0.5f + qb.x * 0.5f);
		qm.y = (qa.y * 0.5f + qb.y * 0.5f);
		qm.z = (qa.z * 0.5f + qb.z * 0.5f);
		return qm;
	}
	float ratioA = sinf((halfTheta - (theta / 2.0f))) / sinHalfTheta;
	float ratioB = sinf((theta / 2.0f)) / sinHalfTheta;
	qm.w = (qa.w * ratioA + qb.w * ratioB);
	qm.x = (qa.x * ratioA + qb.x * ratioB);
	qm.y = (qa.y * ratioA + qb.y * ratioB);
	qm.z = (qa.z * ratioA + qb.z * ratioB);
	return qm;
}

void Math_MatrixFromQuat(Quaternion q, float *matrix){
	float qx = q.x;
	float qy = q.y;
	float qz = q.z;
	float qw = q.w;
	float n = 1.0f / sqrt(qx*qx + qy*qy + qz*qz + qw*qw);
	qx *= n; qy *= n; qz *= n; qw *= n;
	float r[16] = {
		1.0f - 2.0f*qy*qy - 2.0f*qz*qz, 2.0f*qx*qy - 2.0f*qz*qw, 2.0f*qx*qz + 2.0f*qy*qw, 0.0f,
		2.0f*qx*qy + 2.0f*qz*qw, 1.0f - 2.0f*qx*qx - 2.0f*qz*qz, 2.0f*qy*qz - 2.0f*qx*qw, 0.0f,
		2.0f*qx*qz - 2.0f*qy*qw, 2.0f*qy*qz + 2.0f*qx*qw, 1.0f - 2.0f*qx*qx - 2.0f*qy*qy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	memcpy(matrix, r, sizeof(r));
}

void Math_TranslateMatrix(float *matrix, Vector3 vector){

	float r[16] = {
		1, 0, 0, vector.x,
		0, 1, 0, vector.y,
		0, 0, 1, vector.z,
		0, 0, 0, 1,
	};

	memcpy(matrix, r, sizeof(r));
}

void Math_TransposeMatrix3x3(float *matrix){

	float r[3][3];
	float r2[3][3];
	memcpy(r2, matrix, sizeof(float) * 3 * 3);

	for (int k = 0; k < 3; k++)
		for (int f = 0; f < 3; f++)
			r[k][f] = r2[f][k];

	memcpy(matrix, r, 3 * 3 * sizeof(float));
}

void Math_CopyMatrix(float *m1, float *m2){
	int k;
	for (k = 0; k < 16; k++)
		m1[k] = m2[k];
}

float Math_GetAngleBetweenQuats(Quaternion &qa, Quaternion &qb, const Vector3 &n){

	qa.normalize();
	qb.normalize();

	// Vector3 n = {0,-1,0};

	Vector3 vec1 = qa * n;
	Vector3 vec2 = qb * n;

	vec1.normalize();
	vec2.normalize();

	float ret = acos(vec1.dot(vec2));

	if(isnan(ret)) return 0;

	if(vec1.cross(vec2).dot(n) < 0)
		ret = -ret;

	return ret;
}

Quaternion Math_QuatLookAt(Vector3 &forward, Vector3 &up){

	Vector3 forwardW = { 0, 0, -1 };

	up.normalize();

	Vector3 axis = forward.cross(forwardW).normalize();

	float theta = acos(forward.dot(forwardW));

	Vector3 b = axis.cross(forwardW).normalize();

	if (b.dot(forward) < 0) theta = -theta;

	Quaternion qb = Quaternion(axis, theta);

	Vector3 upL = (qb * up).normalize();
	Vector3 right = forward.cross(up).normalize();
	Vector3 upW = right.cross(forward).normalize();

	axis = upL.cross(upW);

	theta = acos(upL.dot(upW));

	// b = axis.cross(upW);

	// if(b.dot(upL) < 0) theta = -theta;

	return Quaternion(axis, theta) * qb;
}

void Math_RotateMatrix(float *matrix, Vector3 angles){
	float sx = sin(angles.x);
	float cx = cos(angles.x);
	float sy = sin(angles.y);
	float cy = cos(angles.y);
	float sz = sin(angles.z);
	float cz = cos(angles.z);

	float m[16] = {
		cy*cz, (-cy*sz*cx) + (sy*sx), (cy*sz*sx) + (sy*cx), 0,
		sz, cz*cx, -cz*sx, 0,
		-sy*cz, (sy*sz*cx) + (cy*sx), (-sy*sz*sx) + (cy*cx), 0,
		0, 0, 0, 1
	};

	memcpy(matrix, m, sizeof(m));
}

Vector3 Math_Rotate(Vector3 pos, Vector3 angles){
	float matrix[16];
	Math_RotateMatrix(matrix, angles);
	return Math_MatrixMult(pos, matrix);
}

Quaternion Math_QuatMult(Quaternion q1, Quaternion q2){
	Quaternion me = {
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
		q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
	};
	return me;
}

void Math_RotateAroundAxis(Vector3 p, float a, float *matrix){
	float qx = p.x * sinf(a / 2);
	float qy = p.y * sinf(a / 2);
	float qz = p.z * sinf(a / 2);
	float qw = cosf(a / 2);
	float n = 1.0f / sqrt(qx*qx + qy*qy + qz*qz + qw*qw);
	qx *= n; qy *= n; qz *= n; qw *= n;
	float r[16] = {
		1.0f - 2.0f*qy*qy - 2.0f*qz*qz, 2.0f*qx*qy - 2.0f*qz*qw, 2.0f*qx*qz + 2.0f*qy*qw, 0.0f,
		2.0f*qx*qy + 2.0f*qz*qw, 1.0f - 2.0f*qx*qx - 2.0f*qz*qz, 2.0f*qy*qz - 2.0f*qx*qw, 0.0f,
		2.0f*qx*qz - 2.0f*qy*qw, 2.0f*qy*qz + 2.0f*qx*qw, 1.0f - 2.0f*qx*qx - 2.0f*qy*qy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	memcpy(matrix, r, sizeof(r));
}

Vector3 Math_QuatRotate(Quaternion q, Vector3 v){
	Quaternion q1 = Math_QuatMult(q, { v.x, v.y, v.z, 0 });
	q1 = Math_QuatMult(q1, { -q.x, -q.y, -q.z, q.w });

	return{ q1.x, q1.y, q1.z };
}

Quaternion Math_Quat(Vector3 v, float a){
	float qx = v.x * sinf(a / 2);
	float qy = v.y * sinf(a / 2);
	float qz = v.z * sinf(a / 2);
	float qw = cosf(a / 2);
	return Quaternion (qx, qy, qz, qw);
}

void Math_MatrixMatrixAdd(float *matrix, float *m0, float *m1){
	float m[16] = {
		m0[0] + m1[0], m0[1] + m1[1], m0[2] + m1[2], m0[3] + m1[3],
		m0[4 + 0] + m1[4 + 0], m0[4 + 1] + m1[4 + 1], m0[4 + 2] + m1[4 + 2], m0[4 + 3] + m1[4 + 3],
		m0[8 + 0] + m1[8 + 0], m0[8 + 1] + m1[8 + 1], m0[8 + 2] + m1[8 + 2], m0[8 + 3] + m1[8 + 3],
		m0[12 + 0] + m1[12 + 0], m0[12 + 1] + m1[12 + 1], m0[12 + 2] + m1[12 + 2], m0[12 + 3] + m1[12 + 3],
	};

	memcpy(matrix, m, sizeof(m));
}

void Math_MatrixMatrixSub(float *matrix, float *m0, float *m1){
	float m[16] = {
		m0[0] - m1[0], m0[1] - m1[1], m0[2] - m1[2], m0[3] - m1[3],
		m0[4 + 0] - m1[4 + 0], m0[4 + 1] - m1[4 + 1], m0[4 + 2] - m1[4 + 2], m0[4 + 3] - m1[4 + 3],
		m0[8 + 0] - m1[8 + 0], m0[8 + 1] - m1[8 + 1], m0[8 + 2] - m1[8 + 2], m0[8 + 3] - m1[8 + 3],
		m0[12 + 0] - m1[12 + 0], m0[12 + 1] - m1[12 + 1], m0[12 + 2] - m1[12 + 2], m0[12 + 3] - m1[12 + 3],
	};

	memcpy(matrix, m, sizeof(m));
}

Quaternion Math_MatrixToQuat(float *m){
	Quaternion q;

	float tr = m[0] + m[5] + m[10];

	if (tr > 0) {
		float S = sqrt(tr + 1.0) * 2;
		q.w = 0.25 * S;
		q.x = (m[9] - m[6]) / S;
		q.y = (m[2] - m[8]) / S;
		q.z = (m[4] - m[1]) / S;
	}
	else if ((m[0] > m[5])&(m[0] > m[10])) {
		float S = sqrt(1.0 + m[0] - m[5] - m[10]) * 2;
		q.w = (m[9] - m[6]) / S;
		q.x = 0.25 * S;
		q.y = (m[1] + m[4]) / S;
		q.z = (m[2] + m[8]) / S;
	}
	else if (m[5] > m[10]) {
		float S = sqrt(1.0 + m[5] - m[0] - m[10]) * 2;
		q.w = (m[2] - m[8]) / S;
		q.x = (m[1] + m[4]) / S;
		q.y = 0.25 * S;
		q.z = (m[6] + m[9]) / S;
	}
	else {
		float S = sqrt(1.0 + m[10] - m[0] - m[5]) * 2;
		q.w = (m[4] - m[1]) / S;
		q.x = (m[2] + m[8]) / S;
		q.y = (m[6] + m[9]) / S;
		q.z = 0.25 * S;
	}

	return q;
}

// void Math_Ik_PseudoInverse(float *matrix, std::vector<Vector3> jacobian){

//     float nontransp[jacobian.size()][3];
//     float transpose[3][jacobian.size()];
//     memcpy(&nontransp[0][0], &jacobian[0].x, sizeof(nontransp));

//     for(int y = 0; y < (int)jacobian.size(); y++)
//         for(int x = 0; x < 3; x++)
//             transpose[x][y] = nontransp[y][x];

//     float ret[jacobian.size()][jacobian.size()];
//     Math_MatrixMultNxN(&ret[0][0], &nontransp[0][0], {(float)jacobian.size(), 3}, &transpose[0][0], {3, (float)jacobian.size()});

//     for(int k = 0; k < (int)jacobian.size(); k++){
//         for(int j = 0; j < (int)jacobian.size(); j++){
//             printf("%f ",ret[k][j] );
//         }
//         printf("\n");
//     }
//     printf("\n");

//     Math_InverseMatrixNxN(&ret[0][0], jacobian.size());

//     for(int k = 0; k < (int)jacobian.size(); k++){
//         for(int j = 0; j < (int)jacobian.size(); j++){
//             printf("%f ",ret[k][j] );
//         }
//         printf("\n");
//     }
//     printf("\n");

//     Math_MatrixMultNxN(matrix, &transpose[0][0], {3, (float)jacobian.size()}, &ret[0][0], {(float)jacobian.size(), (float)jacobian.size()});
// }

Vector3 Math_QuatToAxisAngle(Quaternion quat, float *angle){

	if (quat.w > 1) quat.normalize();

	if (angle) *angle = 2 * acos(quat.w);

	float s = sqrt(1.0 - (quat.w*quat.w));

	Vector3 axis;

	if (s < 0.001)
		return{ quat.x, quat.y, quat.z };

	return{ quat.x / s, quat.y / s, quat.z / s };
}

Vector3 Math_AxisAngleToEuler(Vector3 axis, float angle) {
	float s = sinf(angle);
	float c = cosf(angle);
	float t = 1 - c;

	axis.normalize();

	Vector3 ret;

	if ((axis.x*axis.y*t + axis.z*s) > 0.998) { // north pole singularity detected
		ret.y = 2 * atan2(axis.x*sinf(angle / 2), cosf(angle / 2));
		ret.z = PI / 2;
		ret.x = 0;
		return ret;
	}
	if ((axis.x*axis.y*t + axis.z*s) < -0.998) { // south pole singularity detected
		ret.y = -2 * atan2(axis.x*sinf(angle / 2), cosf(angle / 2));
		ret.z = -PI / 2;
		ret.x = 0;
		return ret;
	}
	ret.y = atan2(axis.y * s - axis.x * axis.z * t, 1 - (axis.y*axis.y + axis.z*axis.z) * t);
	ret.z = asin(axis.x * axis.y * t + axis.z * s);
	ret.x = atan2(axis.x * s - axis.y * axis.z * t, 1 - (axis.x*axis.x + axis.z*axis.z) * t);

	return ret;
}

Vector3 Math_RotateMatrixToEuler(float *m){
	// Assuming the angles are in radians.
	Vector3 ret;
	if (m[4] > 0.998) { // singularity at north pole
		ret.y = atan2(m[2], m[10]);
		ret.z = PI / 2;
		ret.x = 0;
		return ret;
	}
	if (m[4] < -0.998) { // singularity at south pole
		ret.y = atan2(m[2], m[10]);
		ret.z = -PI / 2;
		ret.x = 0;
		return ret;
	}

	ret.y = atan2(-m[8], m[0]);
	ret.x = atan2(-m[6], m[5]);
	ret.z = asin(m[4]);
	return ret;
}

Quaternion Math_EulerToQuat(const Vector3 euler){

	float c1 = cosf(euler.y);
	float s1 = sinf(euler.y);
	float c2 = cosf(euler.z);
	float s2 = sinf(euler.z);
	float c3 = cosf(euler.x);
	float s3 = sinf(euler.x);

	Quaternion ret;
	ret.w = sqrtf(1.0 + c1 * c2 + c1*c3 - s1 * s2 * s3 + c2*c3) / 2.0;
	float w4 = (4.0 * ret.w);
	ret.x = (c2 * s3 + c1 * s3 + s1 * s2 * c3) / w4;
	ret.y = (s1 * c2 + s1 * c3 + c1 * s2 * s3) / w4;
	ret.z = (-s1 * s3 + c1 * s2 * c3 + s2) / w4;

	return ret;
}

Vector3 Math_QuatToEuler(Quaternion quat){
	float sqw = quat.w*quat.w;
	float sqx = quat.x*quat.x;
	float sqy = quat.y*quat.y;
	float sqz = quat.z*quat.z;
	float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
	float test = quat.x*quat.y + quat.z*quat.w;

	Vector3 ret;

	if (test > 0.499*unit) { // singularity at north pole
		ret.y = 2 * atan2(quat.x, quat.w);
		ret.z = PI / 2;
		ret.x = 0;
		return ret;
	}
	if (test < -0.499*unit) { // singularity at south pole
		ret.y = -2 * atan2(quat.x, quat.w);
		ret.z = -PI / 2;
		ret.x = 0;
		return ret;
	}
	ret.y = atan2(2 * quat.y*quat.w - 2 * quat.x*quat.z, sqx - sqy - sqz + sqw);
	ret.z = asin(2 * test / unit);
	ret.x = atan2(2 * quat.x*quat.w - 2 * quat.y*quat.z, -sqx + sqy - sqz + sqw);

	return ret;
}

Vector3 Math_RotateX(Vector3 vec, float theta) {
	Matrix3 mat = { 1, 0, 0, 0, cosf(theta), -sinf(theta), 0, sinf(theta), cosf(theta) };
	return mat * vec;
}

Vector3 Math_RotateY(Vector3 vec, float theta) {
	Matrix3 mat = { cosf(theta), 0, sinf(theta), 0, 1, 0, -sinf(theta), 0, cosf(theta) };
	return mat * vec;
}

Vector3 Math_RotateZ(Vector3 vec, float theta) {
	Matrix3 mat = { cosf(theta), -sinf(theta), 0, sinf(theta), cosf(theta), 0, 0, 0, 1 };
	return mat * vec;
}

// void Joint::SetLimit(int idx, float min, float max){
//     if(min > max) std::swap(max, min);
//     limitsSet[idx] = true;
//     limits[idx] = {Math_Clamp(min, -PI, PI), Math_Clamp(max, -PI, PI)};
// }

// void Joint::ValidateRotation(){

//     Vector3 euler = Math_QuatToEuler(T);

//     if(IsLimited(0)){
//         Vector2 limits = GetLimit(0);
//         euler.x = Math_Clamp(euler.x,limits.x, limits.y);
//     }

//     if(IsLimited(1)){
//         Vector2 limits = GetLimit(1);
//         euler.y = Math_Clamp(euler.y, limits.x, limits.y);
//     }

//     if(IsLimited(2)){
//         Vector2 limits = GetLimit(2);
//         euler.z = Math_Clamp(euler.z,limits.x, limits.y);
//     }

//     T = Math_EulerToQuat(euler);
// }

// Vector3 Arm::ComputeJacobianSegment(int jointIndex, Axis axisType){

//     Joint *joint = joints[jointIndex].get();

//     Vector3 axis;

//     switch(axisType){
//         case Axis::X: axis = joint->GetRight(); break;
//         case Axis::Y: axis = joint->GetUp(); break;
//         case Axis::Z: axis = joint->GetForward(); break;
//     }

//     Vector3 originalEnd = CalculateEndEffector();

//     Quaternion lastT = joint->T;

//     joint->T = Quaternion(axis, EPSILON) * joint->T;

//     Vector3 newEnd = CalculateEndEffector();

//     joint->T = lastT;

//     Vector3 diff = newEnd - originalEnd;

//     return diff/EPSILON;
// }


// Vector3 Arm::CalculateEndEffector(int segment){

//     if(segment == -1) segment = joints.size()-1;

//     Vector3 ret = base;

//     for(int k = 0; k <= segment; k++)
//         ret += joints[k]->GetEndPoint();

//     return ret;
// }

// float Arm::GetMaxLength(){

//     float ret = 0;
//     for(int k = 0; k < (int) joints.size(); k++)
//         ret += joints[k]->mag;

//     return ret;
// }

// float Math_Clamp(float m, float min, float max){
//     if(min > max) std::swap(max, min);
//     if(m <= min) return min;
//     if(m >= max) return max;
//     return m;
// }

// void Joint::ApplyAngleChange(float theta, const Vector3 &axis){
//     T = Quaternion(axis, theta) * T;
//     ValidateRotation();
// }

// void Arm::Reset(){
//     for(int k = 0; k < (int)joints.size(); k++){
//         joints[k]->T = Quaternion();
//         joints[k]->SaveTransformation();
//     }
// }

// void Arm::Solve(Vector3 goal){

// const int maxHalfs = 10;
// const int maxIterations = 10;

// Vector3 currEnd = CalculateEndEffector();

// if((goal - base).magnitude() > (GetMaxLength() - EPSILON))
//     goal = base + (goal - base).normalize() * (GetMaxLength() - EPSILON);

// double currErr, lastErr;
// lastErr = currErr = (goal - currEnd).magnitude();

// int count = 0;

// while(currErr > errorMargin){

//     std::vector<Quaternion> lastJointTransformations;

//     for(int k = 0; k < (int)joints.size(); k++)
//         lastJointTransformations.push_back(joints[k]->T);

//     Eigen::MatrixXf jacobian(3, joints.size() * 3);

//     for(int i=0; i<3*(int)joints.size(); i+=3) {

//         std::shared_ptr<Joint> j = joints[i/3];

//         Vector3 row_theta = ComputeJacobianSegment(i/3, Axis::X);
//         Vector3 row_phi = ComputeJacobianSegment(i/3, Axis::Y);
//         Vector3 row_z = ComputeJacobianSegment(i/3, Axis::Z);

//         jacobian(0, i) = row_theta.x;
//         jacobian(1, i) = row_theta.y;
//         jacobian(2, i) = row_theta.z;
//         jacobian(0, i+1) = row_phi.x;
//         jacobian(1, i+1) = row_phi.y;
//         jacobian(2, i+1) = row_phi.z;
//         jacobian(0, i+2) = row_z.x;
//         jacobian(1, i+2) = row_z.y;
//         jacobian(2, i+2) = row_z.z;
//     }

//     Vector3 dP = goal - currEnd;
//     // printf("%f %f %f\n",dP.x, dP.y, dP.z );

//     Eigen::VectorXf changes =
//         jacobian.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(Eigen::Vector3f(dP.x, dP.y, dP.z));

//     for(int k = 0; k < (int)joints.size(); k++){

//         // printf("%f %f %f\n", changes[(k*3)], changes[(k*3)+1], changes[(k*3)+2]);
//         joints[k]->SaveTransformation();
//         joints[k]->ApplyAngleChange(changes[(k*3)], joints[k]->GetRight());
//         joints[k]->ApplyAngleChange(changes[(k*3)+1], joints[k]->GetUp());
//         joints[k]->ApplyAngleChange(changes[(k*3)+2], joints[k]->GetForward());
//     }

//     currEnd = CalculateEndEffector();

//     currErr = (goal - currEnd).magnitude();

//     int halvingCount = 0;

//     while(currErr > lastErr){

//         for(int k = 0; k < (int)joints.size(); k++){

//             joints[k]->LoadTransformation();
//         }

//         currEnd = CalculateEndEffector();

//         for(int k = 0; k < (int)joints.size(); k++){

//             changes[(k*3)] *= 0.5;
//             changes[(k*3)+1] *= 0.5;
//             changes[(k*3)+2] *= 0.5;
//             joints[k]->SaveTransformation();
//             joints[k]->ApplyAngleChange(changes[(k*3)], joints[k]->GetRight());
//             joints[k]->ApplyAngleChange(changes[(k*3)+1], joints[k]->GetUp());
//             joints[k]->ApplyAngleChange(changes[(k*3)+2], joints[k]->GetForward());
//         }

//         currEnd = CalculateEndEffector();
//         currErr = (goal - currEnd).magnitude();

//         if(halvingCount++ > maxHalfs)
//             break;
//     }

//     if(currErr > lastErr){

//         for(int k = 0; k < (int)joints.size(); k++)
//             joints[k]->T = lastJointTransformations[k];

//         break;
//     }

//     if(count++ > maxIterations)
//         break;
// }
// }


// Eigen::MatrixXf Math_Ik_FindSystemJacobian(const std::vector<Joint> &joints){

// Eigen::MatrixXf jacobian(3, joints.size() * 3);

// for(int k = 0; k < (int)joints.size(); k++){

//     Vector3 origVector = joints[joints.size()-1]->endingPos - joints[k]->startingPos;

//     Vector3 rotatedXVector = Math_RotateX(origVector,EPSILON);
//     Vector3 rotatedYVector = Math_RotateY(origVector,EPSILON);
//     Vector3 rotatedZVector = Math_RotateZ(origVector,EPSILON);

//     Vector3 differenceXVector = (rotatedXVector - origVector) / EPSILON;
//     Vector3 differenceYVector = (rotatedYVector - origVector) / EPSILON;
//     Vector3 differenceZVector = (rotatedZVector - origVector) / EPSILON;

//     jacobian(0, (k * 3)) = differenceXVector.x;
//     jacobian(1, (k * 3)) = differenceXVector.y;
//     jacobian(2, (k * 3)) = differenceXVector.z;
//     jacobian(0, (k * 3)+1) = differenceYVector.x;
//     jacobian(1, (k * 3)+1) = differenceYVector.y;
//     jacobian(2, (k * 3)+1) = differenceYVector.z;



// float length = 0;
// for(int k = 0; k < (int)joints.size(); k++)
//     length += (joints[k]->startingPos - joints[k]->endingPos).magnitude();

// if((goal - joints[0]->startingPos).magnitude() > length)
//     goal = joints[0]->startingPos + ((goal - joints[0]->startingPos).normalize()*length);

// Vector3 goalVec = goal - joints[joints.size()-1]->endingPos;

// return system.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(Eigen::Vector3f(goalVec.x, goalVec.y, goalVec.z));
// }

// void Math_UpdateRotations(std::vector<Joint> &joints, const Eigen::VectorXf &changeInRotations){


// for(int k = 0 ;k < (int)joints.size(); k++){

//     joints[k]->rotation += (Vector3){changeInRotations[(k*3)], changeInRotations[(k*3)+1], changeInRotations[(k*3)+2]};

//     Vector3 origVector = {(float)(joints[k]->origEndingPos - joints[k]->origStartingPos).magnitude(), 0, 0};

//     joints[k]->endingPos = Math_Rotate(origVector, joints[k]->rotation);

//     float angle = acosf(vec2.dot(vec1));

//     joints[k]->quat = Math_Quat(axis, angle);
// }
// }

// bool Math_ReachedGoal(const std::vector<Joint> &joints, const Vector3 &goal){

// Vector3 diffVec = joints[joints.size()-1]->endingPos - goal;
// void Math_Ik(std::vector<Joint> &joints, Vector3 &goal){

// if(!Math_ReachedGoal(joints, goal))
//     Math_UpdateRotations(joints, Math_FindChangeInTheta(joints, Math_Ik_FindSystemJacobian(joints), goal));


