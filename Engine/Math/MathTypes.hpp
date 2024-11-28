#pragma once

#include "Defines.hpp"
#include "DMath.hpp"

#include "Vector.hpp"
#include "Vertex.hpp"
#include "Matrix.hpp"
#include "Frustum.hpp"
#include "Quaternion.hpp"

#if defined(__AVX2__)
#ifndef SIMD_SUPPORTED
#define SIMD_SUPPORTED
#endif
#define SIMD_SUPPORTED_AVX2
#elif defined(__ARM_NEON)
#ifndef SIMD_SUPPORTED
#define SIMD_SUPPORTED
#endif
#define SIMD_SUPPORTED_NEON
#elif defined(__SSE__)
#ifndef SIMD_SUPPORTED
#define SIMD_SUPPORTED
#endif
#define SIMD_SUPPORTED_SSE
#endif

struct DAPI Axis {
	inline static TVector3<float> X = TVector3<float>{ 1.0f, 0.0f, 0.0f };
	inline static TVector3<float> Y = TVector3<float>{ 0.0f, 1.0f, 0.0f };
	inline static TVector3<float> Z = TVector3<float>{ 0.0f, 0.0f, 1.0f };
};

template<typename T>
inline T RangeConvertfloat(T value, T old_min, T old_max, T new_min, T new_max) {
	return (((value - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min;
}

inline void RGB2Uint(unsigned int r, unsigned int g, unsigned int b, unsigned int* rgb) {
	*rgb = (((r & 0x0FF) << 16) | ((g & 0x0FF) << 8) | (b & 0xFF));
}

inline void UInt2RGB(unsigned int rgb, unsigned int* r, unsigned int* g, unsigned int* b) {
	*r = (rgb >> 16) & 0x0FF;
	*g = (rgb >> 8) & 0x0FF;
	*b = (rgb) & 0x0FF;
}

inline void RGB2Vec(unsigned int r, unsigned int g, unsigned int b, TVector3<float>* rgb) {
	rgb->r = r / 255.0f;
	rgb->g = g / 255.0f;
	rgb->b = b / 255.0f;
}

inline void Vec2RGB(TVector3<float>* rgb, unsigned int* r, unsigned int* g, unsigned int* b) {
	*r = static_cast<int>(rgb->r * 255);
	*g = static_cast<int>(rgb->g * 255);
	*b = static_cast<int>(rgb->b * 255);
}

template<typename Type>
inline TQuaternion<Type> MatrixToQuat(const TMatrix4<Type>& M) {
	float trace = M[0] + M[5] + M[10];
	TQuaternion<Type> q;

	if (trace > 0) {
		float S = Dsqrt(trace + 1.0f) * 2;
		q.w = 0.25f * S;
		q.x = (M[9] - M[6]) / S;
		q.y = (M[2] - M[8]) / S;
		q.z = (M[4] - M[1]) / S;
	}
	else {
		if (M[0] > M[5] && M[0] > M[10]) {
			float S = Dsqrt(1.0f + M[0] - M[5] - M[10]) * 2;
			q.w = (M[9] - M[6]) / S;
			q.x = 0.25f * S;
			q.y = (M[1] + M[4]) / S;
			q.z = (M[2] + M[8]) / S;
		}
		else if (M[5] > M[10]) {
			float S = Dsqrt(1.0f + M[5] - M[0] - M[10]) * 2;
			q.w = (M[2] - M[8]) / S;
			q.x = (M[1] + M[4]) / S;
			q.y = 0.25f * S;
			q.z = (M[6] + M[9]) / S;
		}
		else {
			float S = Dsqrt(1.0f + M[10] - M[0] - M[5]) * 2;
			q.w = (M[4] - M[1]) / S;
			q.x = (M[2] + M[8]) / S;
			q.y = (M[6] + M[9]) / S;
			q.z = 0.25f * S;
		}
	}

	if (q.Length() != 1.0f) {
		return TQuaternion<Type>();
	}

	return q;
}

template<typename Type>
inline TMatrix4<Type> QuatToMatrix(const TQuaternion<Type>& q) {
	TMatrix4<Type> Matrix = TMatrix4<Type>::Identity();
	TQuaternion<Type> n = q;
	n.Normalize();

	Matrix.data[0] = 1.0f - 2.0f * (n.y * n.y + n.z * n.z);
	Matrix.data[1] = 2.0f * (n.x * n.y + n.z * n.w);
	Matrix.data[2] = 2.0f * (n.x * n.z - n.y * n.w);

	Matrix.data[4] = 2.0f * (n.x * n.y - n.z * n.w);
	Matrix.data[5] = 1.0f - 2.0f * (n.x * n.x + n.z * n.z);
	Matrix.data[6] = 2.0f * (n.y * n.z - n.x * n.w);

	Matrix.data[8] = 2.0f * (n.x * n.z + n.y * n.w);
	Matrix.data[9] = 2.0f * (n.y * n.z - n.x * n.w);
	Matrix.data[10] = 1.0f - 2.0f * (n.x * n.x + n.y * n.y);

	return Matrix;
}

// Calculates a rotation matrix based on the quaternion and the passed in center point.
template<typename Type>
inline TMatrix4<Type> QuatToRotationMatrix(const TQuaternion<Type>& q, const TVector3<Type>& center) {
	TMatrix4<Type> Matrix;
	Type* o = Matrix.data;

	o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
	o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
	o[2] = 2.0f * ((q.x * q.z) - (q.y + q.w));
	o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

	o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
	o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
	o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
	o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

	o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
	o[9] = 2.0f * ((q.y * q.z) + (q.x * q.w));
	o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
	o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

	o[12] = 0.0f;
	o[13] = 0.0f;
	o[14] = 0.0f;
	o[15] = 1.0f;

	return Matrix;
}

template<typename T>
inline TQuaternion<T> QuaternionSlerp(TQuaternion<T> q0, TQuaternion<T> q1, float percentage) {
	TQuaternion<T> Quat;

	TQuaternion<T> v0 = q0.Normalize();
	TQuaternion<T> v1 = q1.Normalize();

	// Compute the cosine of the angle between the two vectors;
	float dot = v0.Dot(v1);

	// If the dot product is negative, slerp won't take
	// the shorter path. Note that v1 and -v1 are equivalent when the negation is applied to all four components.
	// Fix by reversing one quaternion
	if (dot < 0.0f) {
		v1.x = -v1.x;
		v1.y = -v1.y;
		v1.z = -v1.z;
		v1.w = -v1.w;
		dot = -dot;
	}

	const float DOT_THRESHOLD = 0.9995f;
	if (dot > DOT_THRESHOLD) {
		// If the inputs are too close for comfort, linearly interpolate and normalize the result.
		Quat = TQuaternion<T>{
			v0.x + ((v1.x - v0.x) * percentage),
			v0.y + ((v1.y - v0.y) * percentage),
			v0.z + ((v1.z - v0.z) * percentage),
			v0.w + ((v1.w - v0.w) * percentage)
		};

		return Quat.Normalize();
	}

	// Since dot is in range[0, DOT_THRESHOLD], acos is safe.
	float theta_0 = DCos(dot);
	float theta = theta_0 * percentage;
	float sin_theta = DSin(theta);
	float sin_theta_0 = DSin(theta_0);

	float s0 = DCos(theta) - dot * sin_theta / sin_theta_0;
	float s1 = sin_theta / sin_theta_0;

	return TQuaternion<T>{
		(v0.x* s0) + (v1.x * s1),
			(v0.y* s0) + (v1.y * s1),
			(v0.z* s0) + (v1.z * s1),
			(v0.w* s0) + (v1.w * s1)
	};
}
