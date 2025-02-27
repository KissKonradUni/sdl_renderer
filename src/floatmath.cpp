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

vector4f vector4f::operator+=(const vector4f& other) {
    simd = _mm_add_ps(simd, other.simd);
    return *this;
}

vector4f vector4f::operator-=(const vector4f& other) {
    simd = _mm_sub_ps(simd, other.simd);
    return *this;
}

vector4f vector4f::operator*(float scalar) const {
    return vector4f(_mm_mul_ps(simd, _mm_set1_ps(scalar)));
}

vector4f vector4f::operator/(float scalar) const {
    return vector4f(_mm_div_ps(simd, _mm_set1_ps(scalar)));
}

vector4f vector4f::operator*=(float scalar) {
    simd = _mm_mul_ps(simd, _mm_set1_ps(scalar));
    return *this;
}

vector4f vector4f::operator/=(float scalar) {
    simd = _mm_div_ps(simd, _mm_set1_ps(scalar));
    return *this;
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

vector4f matrix4x4f::operator*(const vector4f& vector) const {
    vector4f result = vector * *this;
    return result;
}

// Transformf

void transformf::moveBy(const vector4f& movement) {
    m_position += movement;
    m_dirty = true;
}

void transformf::rotateBy(const vector4f& rotationAxis, radians angle) {
    m_rotation += rotationAxis * angle;
    m_dirty = true;
}

void transformf::scaleBy(const vector4f& scale) {
    m_scale.x *= scale.x;
    m_scale.y *= scale.y;
    m_scale.z *= scale.z;
    m_dirty = true;
}

matrix4x4f transformf::getModelMatrix() {
    if (!m_dirty) {
        return m_modelMatrix;
    }

    matrix4x4f translationMatrix = matrix4x4f::translation(m_position);
    matrix4x4f rotationMatrixX   = matrix4x4f::rotation(m_rotation.x, vector4f::right());
    matrix4x4f rotationMatrixY   = matrix4x4f::rotation(m_rotation.y, vector4f::up());
    matrix4x4f rotationMatrixZ   = matrix4x4f::rotation(m_rotation.z, vector4f::front());
    matrix4x4f scaleMatrix       = matrix4x4f::scale(m_scale);

    this->m_modelMatrix = (scaleMatrix * rotationMatrixZ * rotationMatrixY * rotationMatrixX * translationMatrix);

    if (m_parent) {
        this->m_modelMatrix = m_parent->getModelMatrix() * this->m_modelMatrix;
    }

    m_dirty = false;

    return this->m_modelMatrix;
}