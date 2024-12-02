#include <Math/MathTypes.hpp>

template<typename T>
struct V4 {
public:
	using DataType = std::conditional_t<std::is_same_v<T, float>, __m128, __m256d>;

	alignas(16) DataType data;  // 256-bit AVX register
	alignas(16) T x, y, z, w;         // Individual elements for scalar operations

	V4() : x(0), y(0), z(0), w(0) {
		if constexpr (std::is_same_v<T, float>) {
			// For float: Use _mm256_set_ps and _mm256_add_ps
			data = _mm_set_ps(w, z, y, x);  // Load x, y, z, w into v1
		}
		else {
			// For double: Use _mm256_set_pd and _mm256_add_pd
			data = _mm256_set_pd(w, z, y, x);  // Load x, y, z, w into v1
		}
	}

	V4(T r, T g, T b, T a) : x(r), y(g), z(b), w(a) {
		if constexpr (std::is_same_v<T, float>) {
			// For float: Use _mm256_set_ps and _mm256_add_ps
			data = _mm_set_ps(w, z, y, x);  // Load x, y, z, w into v1
		}
		else {
			// For double: Use _mm256_set_pd and _mm256_add_pd
			data = _mm256_set_pd(w, z, y, x);  // Load x, y, z, w into v1
		}
	}

	// SIMD operator+ using AVX
	V4 operator+(const V4& o) {
		// Load the current object's data and the other object's data into AVX registers
		DataType v1, v2, result;
		if constexpr (std::is_same_v<T, float>) {
			// For float: Use _mm256_set_ps and _mm256_add_ps
			v1 = _mm_set_ps(w, z, y, x);  // Load x, y, z, w into v1
			v2 = _mm_set_ps(o.w, o.z, o.y, o.x);  // Load o.x, o.y, o.z, o.w into v2
			result = _mm256_add_ps(v1, v2);  // Perform SIMD addition
		}
		else {
			// For double: Use _mm256_set_pd and _mm256_add_pd
			v1 = _mm256_set_pd(w, z, y, x);  // Load x, y, z, w into v1
			v2 = _mm256_set_pd(o.w, o.z, o.y, o.x);  // Load o.x, o.y, o.z, o.w into v2
			result = _mm256_add_pd(v1, v2);  // Perform SIMD addition
		}

		// Store the result back into a V4 object
		V4 res;
		if constexpr (std::is_same_v<T, float>) {
			// For float: Use _mm256_store_ps
			_mm_store_ps(reinterpret_cast<float*>(&res.data), result);
		}
		else {
			// For double: Use _mm256_store_pd
			_mm256_store_pd(reinterpret_cast<double*>(&res.data), result);
		}

		// Store the scalar values from the SIMD result
		res.x = reinterpret_cast<T*>(&res.data)[0];
		res.y = reinterpret_cast<T*>(&res.data)[1];
		res.z = reinterpret_cast<T*>(&res.data)[2];
		res.w = reinterpret_cast<T*>(&res.data)[3];

		return res;
	}

	V4 operator*(T a) {
		V4 res;
		if constexpr (std::is_same_v<T, float>) {
			__m128 v = _mm_set1_ps(a);  // Load scalar a into all elements of a __m256
			res.data = _mm_mul_ps(data, v);  // Perform SIMD multiplication
			_mm_store_ps(reinterpret_cast<float*>(&res.data), res.data);
		}
		else {
			__m256d v = _mm256_set1_pd(a);  // Load scalar a into all elements of a __m256d
			res.data = _mm256_mul_pd(data, v);  // Perform SIMD multiplication
			_mm256_store_pd(reinterpret_cast<double*>(&res.data), res.data);
		}
		res.x = reinterpret_cast<T*>(&res.data)[0];
		res.y = reinterpret_cast<T*>(&res.data)[1];
		res.z = reinterpret_cast<T*>(&res.data)[2];
		res.w = reinterpret_cast<T*>(&res.data)[3];
		return res;
	}

	// Output the vector values (excluding the SIMD data)
	friend std::ostream& operator<<(std::ostream& os, const V4& vec) {
		return os << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w;
	}
};

void CheckSupportedSIMD() {
#if defined(SIMD_SUPPORTED_NEON)
	std::cout << "arm NEON is supported.\n";
#endif
#if defined(SIMD_SUPPORTED_AVX)
	std::cout << "AVX is supported.\n";
#endif
#if defined(SIMD_SUPPORTED_AVX2)
	std::cout << "AVX2 is supported.\n";
#endif
#if defined(SIMD_SUPPORTED_SSE)
	std::cout << "SSE is supported.\n";
#endif
#if defined(SIMD_SUPPORTED_SSE2)
	std::cout << "SSE2 is supported.\n";
#endif
}

void TestSIMD(){
	LOG_INFO("\n SIMD:\n");
	CheckSupportedSIMD();

	V4<double> v1(1.0f, 2.0, 3.0, 4.0);
	V4<double> v2(5.0f, 6.0, 7.0, 8.0);

	LOG_INFO("Add:");
	V4 v3 = v1 + v2;
	std::cout << v3 << std::endl;

	LOG_INFO("Mul:");
	V4 v4 = v1 * 2.0;
	std::cout << v4 << std::endl;

}