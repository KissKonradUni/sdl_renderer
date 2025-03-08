#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <xmmintrin.h>

/*
    Data types for this rendering engine.
    - 16-byte aligned data types for SIMD.
*/

// Type definitions for clarity
typedef float radians;
typedef float degrees;

// Forward declaration
struct vector4f;
struct matrix4x4f;
struct transformf;

/**
 * @brief 4D vector of floats.
 * Uses SIMD for faster calculations.
 */
struct alignas(16) vector4f {
    union {
        struct {
            float x, y, z, w;
        };
        std::array<float, 4> as_array;
        __m128 simd;
    };

    vector4f() : simd(_mm_setzero_ps()) {}
    vector4f(float x, float y, float z, float w) : simd(_mm_set_ps(w, z, y, x)) {}
    vector4f(const __m128 simd) : simd(simd) {}
    vector4f(const vector4f& other) : simd(other.simd) {}

    static constexpr const vector4f zero()  { return vector4f( 0.0f,  0.0f,  0.0f,  0.0f); }
    static constexpr const vector4f one()   { return vector4f( 1.0f,  1.0f,  1.0f,  1.0f); }

    static constexpr const vector4f up()    { return vector4f( 0.0f,  1.0f,  0.0f,  0.0f); }
    static constexpr const vector4f down()  { return vector4f( 0.0f, -1.0f,  0.0f,  0.0f); }
    static constexpr const vector4f right() { return vector4f( 1.0f,  0.0f,  0.0f,  0.0f); }
    static constexpr const vector4f left()  { return vector4f(-1.0f,  0.0f,  0.0f,  0.0f); }
    static constexpr const vector4f back()  { return vector4f( 0.0f,  0.0f,  1.0f,  0.0f); }
    static constexpr const vector4f front() { return vector4f( 0.0f,  0.0f, -1.0f,  0.0f); }

    /**
     * @return float The length of the vector.
     */
    float length4d() const;
    /**
     * @return float The length of the vector in 3D space (ignoring the w component).
     */
    float length3d() const;

    /**
     * @return vector4f The normalized vector.
     */
    vector4f normalize4d() const;
    /**
     * @return vector4f The normalized vector in 3D space.
     */
    vector4f normalize3d() const;
    
    /**
     * @brief Get the dot product of two vectors.
     * 
     * @param other The other vector.
     * @return float The dot product of the two vectors.
     */
    float dot4d(const vector4f& other) const;
    /**
     * @param other The other vector.
     * @return float The dot product of the two vectors in 3D space.
     */
    float dot3d(const vector4f& other) const;

    /**
     * @param other The other vector.
     * @return vector4f The cross product of the two vectors.
     */
    vector4f cross4d(const vector4f& other) const;
    /**
     * @param other The other vector.
     * @return vector4f The cross product of the two vectors in 3D space.
     */
    vector4f cross3d(const vector4f& other) const;

    vector4f operator+(const vector4f& other) const;
    vector4f operator-(const vector4f& other) const;

    vector4f operator+=(const vector4f& other);
    vector4f operator-=(const vector4f& other);

    vector4f operator*(float scalar) const;
    vector4f operator/(float scalar) const;

    vector4f operator*=(float scalar);
    vector4f operator/=(float scalar);

    vector4f operator-() const;

    vector4f operator*(const matrix4x4f& matrix) const;
};

/**
 * @brief 4D quaternion of floats.
 * Uses SIMD for faster calculations.
 */
struct alignas(16) quaternionf {
    union {
        struct {
            float x, y, z, w;
        };
        std::array<float, 4> as_array;
        vector4f as_vector;
        __m128 simd;
    };

    quaternionf() : simd(_mm_setzero_ps()) {}
    quaternionf(float x, float y, float z, float w) : simd(_mm_set_ps(w, z, y, x)) {}
    quaternionf(const __m128 simd) : simd(simd) {}
    quaternionf(const quaternionf& other) : simd(other.simd) {}
};

/**
 * @brief 4x4 matrix of floats.
 * Uses SIMD for faster calculations.
 */
struct alignas(16) matrix4x4f {
    union {
        // Column-major order
        struct {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
        std::array<float, 16> as_array;
        std::array<std::array<float, 4>, 4> as_array_rows;
        std::array<vector4f, 4> as_vector_rows;
        __m128 simd_rows[4];
    };

    matrix4x4f() : simd_rows{ _mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps() } {}
    matrix4x4f(const std::array<float, 16>& array): as_array(array) {}
    matrix4x4f(const std::array<std::array<float, 4>, 4>& array) : as_array_rows(array) {}
    matrix4x4f(const __m128 simd_rows[4]) : simd_rows{ simd_rows[0], simd_rows[1], simd_rows[2], simd_rows[3] } {}
    matrix4x4f(const matrix4x4f& other) : matrix4x4f(other.simd_rows) {}

    // Copy assignment
    matrix4x4f& operator=(const matrix4x4f& other) {
        simd_rows[0] = other.simd_rows[0];
        simd_rows[1] = other.simd_rows[1];
        simd_rows[2] = other.simd_rows[2];
        simd_rows[3] = other.simd_rows[3];
        return *this;
    }

    /**
     * @return matrix4x4f The identity matrix.
     */
    static constexpr const matrix4x4f identity() {
        return matrix4x4f(std::array<float, 16> {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        });
    }
    /**
     * @return matrix4x4f The null matrix.
     */
    static constexpr const matrix4x4f null() {
        return matrix4x4f(std::array<float, 16> {});
    }

    /**
     * @param left 
     * @param right 
     * @param bottom 
     * @param top 
     * @param near 
     * @param far 
     * @return matrix4x4f The orthographic projection matrix.
     */
    static matrix4x4f orthographic(float left, float right, float bottom, float top, float near, float far);
    /**
     * @param fov The field of view in radians.
     * @param aspect The aspect ratio of the screen.
     * @param near 
     * @param far 
     * @return matrix4x4f The perspective projection matrix.
     */
    static matrix4x4f perspective(radians fov, float aspect, float near, float far);

    /**
     * @brief Use for moving objects in 3D space.
     * 
     * @param x
     * @param y 
     * @param z 
     * @return matrix4x4f The translation matrix.
     */
    static matrix4x4f translation(float x, float y, float z);
    /**
     * @brief Use for moving objects in 3D space.
     *
     * @param vec The vector to translate by.
     * @return matrix4x4f The translation matrix.
     */
    static matrix4x4f translation(vector4f vec) { return translation(vec.x, vec.y, vec.z); }
    /**
     * @brief Use for rotating objects in 3D space. Select which axis to rotate around using a unit vector.
     * 
     * @param angle The angle in radians.
     * @param x 
     * @param y 
     * @param z 
     * @return matrix4x4f The rotation matrix.
     */
    static matrix4x4f rotation(radians angle, float x, float y, float z);
    /**
     * @brief Use for rotating objects in 3D space. Select which axis to rotate around using a unit vector.
     * 
     * @param angle The angle in radians.
     * @param unit The unit vector to rotate around.
     * @return matrix4x4f The rotation matrix.
     */
    static matrix4x4f rotation(radians angle, vector4f unit) { return rotation(angle, unit.x, unit.y, unit.z); }
    /**
     * @brief Use for scaling objects in 3D space.
     * 
     * @param x 
     * @param y 
     * @param z 
     * @return matrix4x4f The scale matrix.
     */
    static matrix4x4f scale(float x, float y, float z);
    /**
     * @brief Use for scaling objects in 3D space.
     * 
     * @param vec The vector to scale by.
     * @return matrix4x4f The scale matrix.
     */
    static matrix4x4f scale(vector4f vec) { return scale(vec.x, vec.y, vec.z); }
    /**
     * @brief Use for scaling objects in 3D space.
     * 
     * @param s Scalar value to scale by.
     * @return matrix4x4f The scale matrix.
     */
    static matrix4x4f scale(float s) { return scale(s, s, s); }
    /**
     * @brief Use for creating a rotation matrix for the camera.
     * 
     * @param rotation The rotation vector.
     * @return matrix4x4f The look-at matrix.
     */
    static matrix4x4f lookAt(const vector4f& rotation);

    /**
     * @return matrix4x4f The transpose of the matrix.
     */
    matrix4x4f transpose() const;

    matrix4x4f operator+(const matrix4x4f& other) const;
    matrix4x4f operator-(const matrix4x4f& other) const;
    matrix4x4f operator*(float scalar) const;
    matrix4x4f operator*(const matrix4x4f& other) const;
    vector4f operator*(const vector4f& matrix) const;
};

/**
 * @brief Represents a 3D transform.
 * Used for representing the "space" of an object in 3D space.
 */
struct alignas(16) transformf {
public:
    // Create a base transform at the origin
    transformf() {}
    /**
     * @brief Create a transform with a parent.
     * 
     * @param parent The parent transform.
     */
    transformf(transformf* parent) : m_parent(parent) {}

    /**
     * @return vector4f The position of the object.
     */
    vector4f getPosition() const { return m_position; }
    /**
     * @param position The new position of the object.
     */
    void setPosition(const vector4f& position) { this->m_position = position; m_dirty = true; }

    /**
     * @return vector4f The rotation of the object.
     */
    vector4f getRotation() const { return m_rotation; }
    /**
     * @param rotation The new rotation of the object.
     */
    void setRotation(const vector4f& rotation) { this->m_rotation = rotation; m_dirty = true; }

    /**
     * @return vector4f The scale of the object.
     */
    vector4f getScale() const { return m_scale; }
    /**
     * @param scale The new scale of the object.
     */
    void setScale(const vector4f& scale) { this->m_scale = scale; m_dirty = true; }

    /**
     * @return const std::shared_ptr<transformf>& The parent transform of the object.
     * @attention May return `nullptr` if the object has no parent.
     */
    const transformf* getParent() const { return m_parent; }
    /**
     * @brief Set the parent transform of the object.
     * 
     * @param parent The parent transform. If `nullptr`, the object will be parentless.
     */
    void setParent(transformf* parent) { this->m_parent = parent; m_dirty = true; }

    /**
     * @param movement The vector to move the object by. (in local space)
     */
    void moveBy(const vector4f& movement);
    /**
     * @param rotationAxis The (local) axis to rotate around.
     * @param angle The angle in radians.
     */
    void rotateBy(const vector4f& rotationAxis, radians angle);
    /**
     * @param scale The (local) scale to apply.
     */
    void scaleBy(const vector4f& scale);

    /**
     * @return matrix4x4f The model matrix of the object. (World -> Local)
     */
    matrix4x4f getModelMatrix();
protected:
    transformf* m_parent = nullptr;

    vector4f m_position = vector4f::zero();
    vector4f m_rotation = vector4f::zero();
    vector4f m_scale    = vector4f::one();

    matrix4x4f m_modelMatrix;

    bool m_dirty = true;
};