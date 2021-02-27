#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

//These are being brought in from somewhere so undefine them
#undef near
#undef far

namespace garlic::clove {
    template<typename T>
    typename T::value_type const *valuePtr(T const &v) {
        return glm::value_ptr(v);
    }

    template<typename T>
    typename T::value_type *valuePtr(T &v) {
        return glm::value_ptr(v);
    }

    template<typename T>
    T constexpr asRadians(T degrees) {
        return glm::radians(degrees);
    }

    template<typename T>
    T constexpr asDegrees(T radians) {
        return glm::degrees(radians);
    }

    template<typename T>
    mat<4, 4, T> constexpr createOrthographicMatrix(T left, T right, T bottom, T top) {
        //Swap the bottom and top so the winding order is correct after applying the matrix
        return glm::ortho(left, right, top, bottom);
    }

    template<typename T>
    mat<4, 4, T> constexpr createOrthographicMatrix(T left, T right, T bottom, T top, T near, T far) {
        //Swap the bottom and top so the winding order is correct after applying the matrix
        return glm::ortho(left, right, top, bottom, near, far);
    }

    template<typename T>
    mat<4, 4, T> constexpr createPerspectiveMatrix(T fovy, T aspect, T zNear, T zFar) {
        /*
            Defining our own implementation for this function as using glm with vulkan will
            result in y+ moving objects downwards.
        */

        T const tanHalfFovy{ std::tan(fovy / static_cast<T>(2)) };

        mat<4, 4, T> result{ static_cast<T>(0) };
        result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
        result[1][1] = -(static_cast<T>(1) / (tanHalfFovy));
        result[2][2] = zFar / (zFar - zNear);
        result[2][3] = static_cast<T>(1);
        result[3][2] = -(zFar * zNear) / (zFar - zNear);

        return result;
    }

    template<typename T>
    vec<3, T> matrixToEuler(mat<4, 4, T> const &mat) {
        vec<3, T> vec;
        glm::extractEulerAngleXYZ(mat, vec.x, vec.y, vec.z);
        return vec;
    }

    template<typename T>
    quat<T> matrixToQuaternion(mat<3, 3, T> const &mat) {
        return glm::toQuat(mat);
    }

    template<typename T>
    quat<T> matrixToQuaternion(mat<4, 4, T> const &mat) {
        return glm::toQuat(mat);
    }

    template<typename T>
    quat<T> asQuaternion(T angle, vec<3, T> const &axis) {
        return glm::angleAxis(angle, axis);
    }

    template<typename T>
    vec<3, T> quaternionToEuler(quat<T> const &quat) {
        return { glm::axis(quat) * glm::angle(quat) };
    }

    template<typename T>
    mat<3, 3, T> quaternionToMatrix3(quat<T> const &quat) {
        return glm::toMat3(quat);
    }

    template<typename T>
    mat<4, 4, T> quaternionToMatrix4(quat<T> const &quat) {
        return glm::toMat4(quat);
    }

    template<typename T>
    vec<3, T> screenToWorld(vec<2, T> const &screenPos, T distance, vec<2, T> const &screenSize, vec<3, T> const &forward, mat<4, 4, T> const &viewMatrix, mat<4, 4, T> const &projectionMatrix) {
        //Convert the distance as a point into screen space so we can get the correct depth (z) value for the NDC
        vec<4, T> screenDistance{ projectionMatrix * viewMatrix * vec<4, T>{ forward * distance, 1.0f } };
        screenDistance /= screenDistance.w;

        vec<4, T> const screenPosNDC{
            ((screenPos.x / screenSize.x) - 0.5f) * 2.0f,
            ((screenPos.y / screenSize.y) - 0.5f) * 2.0f,
            screenDistance.z,
            1.0f,
        };

        vec<4, T> world{ inverse(projectionMatrix * viewMatrix) * screenPosNDC };
        world /= world.w;

        return vec<3, T>{ world.x, world.y, world.z };
    }

    template<typename T>
    vec<3, T> decomposeTranslation(mat<4, 4, T> const &matrix){
        return { matrix[3] };
    }
}