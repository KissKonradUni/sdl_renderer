#include "floatmath.hpp"

#include <cmath>

// Vector4f

float vector4f::length4d() const {
    union {
        __m128 simd;
        float as_array[4];
    } pow;
    pow.simd = _mm_mul_ps(simd, simd);
    return SDL_sqrtf(pow.as_array[0] + pow.as_array[1] + pow.as_array[2] + pow.as_array[3]);
}

float vector4f::length3d() const {
    vector4f copy(*this);
    copy.w = 0.0f;
    return copy.length4d();
}

vector4f vector4f::normalize4d() const {
    float len = length4d();
    if (len == 0.0f) {
        return vector4f::zero();
    }
    return vector4f(*this) / len;
}

vector4f vector4f::normalize3d() const {
    vector4f result(*this);

    float len = result.length3d();
    if (len == 0.0f) {
        return vector4f::zero();
    }
    result = result / len;
    result.w = 0.0f;

    return result;
}

float vector4f::dot4d(const vector4f& other) const {
    union {
        __m128 simd;
        float as_array[4];
    } mul;
    mul.simd = _mm_mul_ps(simd, other.simd);
    return mul.as_array[0] + mul.as_array[1] + mul.as_array[2] + mul.as_array[3];
}

float vector4f::dot3d(const vector4f& other) const {
    vector4f copy(*this);
    copy.w = 0.0f;
    vector4f copy_other(other);
    copy_other.w = 0.0f;
    return copy.dot4d(copy_other);
}

// https://geometrian.com/programming/tutorials/cross-product/index.php
vector4f vector4f::cross4d(const vector4f& other) const {
    __m128 tmp0 = _mm_shuffle_ps(simd, simd, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 tmp1 = _mm_shuffle_ps(other.simd, other.simd, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 tmp2 = _mm_mul_ps(tmp0, simd);
    __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
    __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
    return vector4f(_mm_sub_ps(tmp3, tmp4));
}

vector4f vector4f::cross3d(const vector4f& other) const {
    vector4f copy(*this);
    copy.w = 0.0f;
    vector4f copy_other(other);
    copy_other.w = 0.0f;
    return copy.cross4d(copy_other);
}

vector4f vector4f::operator+(const vector4f& other) const {
    return vector4f(_mm_add_ps(simd, other.simd));
}

vector4f vector4f::operator-(const vector4f& other) const {
    return vector4f(_mm_sub_ps(simd, other.simd));
}

vector4f vector4f::operator*(float scalar) const {
    return vector4f(_mm_mul_ps(simd, _mm_set1_ps(scalar)));
}

vector4f vector4f::operator/(float scalar) const {
    return vector4f(_mm_div_ps(simd, _mm_set1_ps(scalar)));
}

vector4f vector4f::operator-() const {
    return vector4f(_mm_sub_ps(_mm_setzero_ps(), simd));
}

vector4f vector4f::operator*(const matrix4x4f& matrix) const {
    vector4f result;

    for (int i = 0; i < 4; i++) {
        union {
            __m128 simd;
            float as_array[4];
        } mul;
        mul.simd = _mm_mul_ps(simd, matrix.simd_rows[i]);
        result.as_array[i] = mul.as_array[0] + mul.as_array[1] + mul.as_array[2] + mul.as_array[3];
    }

    return result;
}

// Quaternionf

// Matrix4x4f

matrix4x4f matrix4x4f::orthographic(float left, float right, float bottom, float top, float near, float far) {
    vector4f scale = vector4f(2.0f / (right - left), 2.0f / (top - bottom), -2.0f / (far - near), 1.0f);
    vector4f translation = vector4f(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 0.0f);

    matrix4x4f scale_matrix = matrix4x4f::scale(scale);
    matrix4x4f translation_matrix = matrix4x4f::translation(translation);

    return scale_matrix * translation_matrix;
}

matrix4x4f matrix4x4f::perspective(float fov, float aspect, float near, float far) {
    float tangent = tanf(fov / 2.0f);
    float right   = far * tangent;
    float top     = right / aspect;


    return matrix4x4f(std::array<float, 16> {
        far / right,       0.0f,                                0.0f,  0.0f,
                0.0f, far / top,                                0.0f,  0.0f,
                0.0f,       0.0f,        -(near + far) / (near - far), -1.0f,
                0.0f,       0.0f, -(2.0f * near * far) / (near - far),  0.0f
    });
}

matrix4x4f matrix4x4f::translation(float x, float y, float z) {
    return matrix4x4f(std::array<float, 16> {
        1.0f, 0.0f, 0.0f,   x,
        0.0f, 1.0f, 0.0f,   y,
        0.0f, 0.0f, 1.0f,   z,
        0.0f, 0.0f, 0.0f, 1.0f
    });
}

matrix4x4f matrix4x4f::rotation(float angle, float x, float y, float z) {
    const float c = cosf(angle);
    const float s = sinf(angle);
    const float t = 1.0f - c;

    return matrix4x4f(std::array<float, 16> {
        t * x * x + c,     t * x * y - s * z, t * x * z + s * y, 0.0f,
        t * x * y + s * z, t * y * y + c,     t * y * z - s * x, 0.0f,
        t * x * z - s * y, t * y * z + s * x, t * z * z + c,     0.0f,
        0.0f,              0.0f,              0.0f,              1.0f
    });
}

matrix4x4f matrix4x4f::lookAt(const vector4f& rotation) {
    const auto rotationMatrix = matrix4x4f::rotation(rotation.x, 1.0f, 0.0f, 0.0f) *
                                matrix4x4f::rotation(rotation.y, 0.0f, 1.0f, 0.0f) *
                                matrix4x4f::rotation(rotation.z, 0.0f, 0.0f, 1.0f);
    const auto forward = vector4f::front() * rotationMatrix;
    const auto up      = vector4f::up()    * rotationMatrix;
    const auto right   = vector4f::right() * rotationMatrix;

    matrix4x4f result = matrix4x4f(std::array<float, 16> {
        right.x,   right.y,   right.z,   0.0f,
        up.x,      up.y,      up.z,      0.0f,
        forward.x, forward.y, forward.z, 0.0f,
        0.0f,      0.0f,      0.0f,      1.0f
    });

    return result;
}

matrix4x4f matrix4x4f::scale(float x, float y, float z) {
    return matrix4x4f(std::array<float, 16> {
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    y, 0.0f, 0.0f,
        0.0f, 0.0f,    z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    });
}

matrix4x4f matrix4x4f::transpose() const {
    matrix4x4f result(*this);

    _MM_TRANSPOSE4_PS(result.simd_rows[0], result.simd_rows[1], result.simd_rows[2], result.simd_rows[3]);
    
    return result;
}

matrix4x4f matrix4x4f::operator+(const matrix4x4f& other) const {
    matrix4x4f result;

    for (int i = 0; i < 4; i++) {
        result.simd_rows[i] = _mm_add_ps(simd_rows[i], other.simd_rows[i]);
    }

    return result;
}

matrix4x4f matrix4x4f::operator-(const matrix4x4f& other) const {
    matrix4x4f result;

    for (int i = 0; i < 4; i++) {
        result.simd_rows[i] = _mm_sub_ps(simd_rows[i], other.simd_rows[i]);
    }

    return result;
}

matrix4x4f matrix4x4f::operator*(float scalar) const {
    matrix4x4f result;

    for (int i = 0; i < 4; i++) {
        result.simd_rows[i] = _mm_mul_ps(simd_rows[i], _mm_set1_ps(scalar));
    }

    return result;
}

matrix4x4f matrix4x4f::operator*(const matrix4x4f& other) const {
    matrix4x4f result;

    for (int i = 0; i < 4; i++) {
        result.simd_rows[i] = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(simd_rows[0], _mm_set1_ps(other.as_array_rows[i][0])),
                _mm_mul_ps(simd_rows[1], _mm_set1_ps(other.as_array_rows[i][1]))
            ),
            _mm_add_ps(
                _mm_mul_ps(simd_rows[2], _mm_set1_ps(other.as_array_rows[i][2])),
                _mm_mul_ps(simd_rows[3], _mm_set1_ps(other.as_array_rows[i][3]))
            )
        );
    }

    return result;
}

// Tests

void logMatrix(const matrix4x4f &matrix) {
    SDL_Log("Matrix4x4f:");
    for (int i = 0; i < 4; i++) {
        SDL_Log("> Row %d: %f, %f, %f, %f", i, matrix.as_array_rows[i][0], matrix.as_array_rows[i][1], matrix.as_array_rows[i][2], matrix.as_array_rows[i][3]);
    }
}

void runFloatMathTests() {
    // Vector4f tests
    SDL_Log("\nVector4f tests:");

    vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    vector4f b(5.0f, 6.0f, 7.0f, 8.0f);

    SDL_Log("a = (%f, %f, %f, %f)", a.x, a.y, a.z, a.w);
    SDL_Log("b = (%f, %f, %f, %f)", b.x, b.y, b.z, b.w);

    vector4f c = a + b;
    SDL_Log("a + b = (%f, %f, %f, %f)", c.x, c.y, c.z, c.w);

    c = a - b;
    SDL_Log("a - b = (%f, %f, %f, %f)", c.x, c.y, c.z, c.w);

    c = a * 2.0f;
    SDL_Log("a * 2 = (%f, %f, %f, %f)", c.x, c.y, c.z, c.w);

    c = a / 2.0f;
    SDL_Log("a / 2 = (%f, %f, %f, %f)", c.x, c.y, c.z, c.w);

    c = -a;
    SDL_Log("-a = (%f, %f, %f, %f)", c.x, c.y, c.z, c.w);

    float len = a.length3d();
    SDL_Log("\n3D Length of a = %f", len);
    c = a.normalize3d();
    SDL_Log("3D Normalized a = (%f, %f, %f, %f)", c.x, c.y, c.z, c.w);

    float dot = a.dot4d(b) / (a.length4d() * b.length4d());
    SDL_Log("\n3D Dot product of a and b = %f", dot);
    SDL_Log("Should be: ~0.969");

    vector4f cross = a.cross3d(b);
    SDL_Log("\n3D Cross product of a and b = (%f, %f, %f, %f)", cross.x, cross.y, cross.z, cross.w);
    SDL_Log("Should be: (-4, 8, -4, 0)");

    // Quaternionf tests

    // Matrix4x4f tests
    vector4f one = vector4f::one();

    SDL_Log("\nMatrix4x4f tests:");
    SDL_Log("\nIdentity matrix:");
    matrix4x4f i = matrix4x4f::identity();
    logMatrix(i);

    vector4f result = one * i;
    SDL_Log("Result of multiplying (1, 1, 1, 1) by the matrix:");
    SDL_Log("(%f, %f, %f, %f)", result.x, result.y, result.z, result.w);

    SDL_Log("\nTranslation matrix:");
    matrix4x4f t = matrix4x4f::translation(1.0f, 0.0f, 0.0f);
    logMatrix(t);

    result = one * t;
    SDL_Log("Result of multiplying (1, 1, 1, 1) by the matrix:");
    SDL_Log("(%f, %f, %f, %f)", result.x, result.y, result.z, result.w);

    SDL_Log("\nRotation matrix:");
    matrix4x4f r = matrix4x4f::rotation(SDL_PI_F / 2.0f, 0.0f, 0.0f, 1.0f);
    logMatrix(r);

    result = one * r;
    SDL_Log("Result of multiplying (1, 1, 1, 1) by the matrix:");
    SDL_Log("(%f, %f, %f, %f)", result.x, result.y, result.z, result.w);

    SDL_Log("\nScale matrix:");
    matrix4x4f s = matrix4x4f::scale(2.0f, 2.0f, 2.0f);
    logMatrix(s);

    result = one * s;
    SDL_Log("Result of multiplying (1, 1, 1, 1) by the matrix:");
    SDL_Log("(%f, %f, %f, %f)", result.x, result.y, result.z, result.w);

    SDL_Log("\nMatrix multiplication:");
    SDL_Log("Rotate in place, scale in place, then translate:");
    matrix4x4f m = r * s * t;
    logMatrix(m);

    result = one * m;
    SDL_Log("Result of multiplying (1, 1, 1, 1) by the matrix:");
    SDL_Log("(%f, %f, %f, %f)", result.x, result.y, result.z, result.w);

    SDL_Log("\nOrthographic matrix:");
    matrix4x4f o = matrix4x4f::orthographic(0.0f, 1280.0f, 0.0f, 720.0f, 0.01f, 100.0f);
    logMatrix(o);

    SDL_Log("\nPerspective matrix (90deg):");
    matrix4x4f p = matrix4x4f::perspective(SDL_PI_F / 2.0f, 16.0f / 9.0f, 0.01f, 100.0f);
    logMatrix(p);
}