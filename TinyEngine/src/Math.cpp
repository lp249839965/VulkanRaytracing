#include "Math.h"

glm::mat4 Math::ortho(float left, float right, float bottom, float top, float nearVal, float farVal)
{
    glm::mat4 result;

    result[0][0] = 2.0f / (right - left);
    result[0][1] = 0.0f;
    result[0][2] = 0.0f;
    result[0][3] = 0.0f;

    result[1][0] = 0.0f;
    // Window clip origin is upper left.
    result[1][1] = -2.0f / (top - bottom);
    result[1][2] = 0.0f;
    result[1][3] = 0.0f;

    result[2][0] = 0.0f;
    result[2][1] = 0.0f;
    // z range is [0 1]
    result[2][2] = -1.0f / (farVal - nearVal);
    result[2][3] = 0.0f;

    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    // z range is [0 1]
    result[3][2] = -nearVal / (farVal - nearVal);
    result[3][3] = 1.0f;

    return result;
}

glm::mat4 Math::perspective(float fovy, float aspect, float zNear, float zFar)
{
    float xmin, xmax, ymin, ymax;

    ymax = zNear * tanf(glm::radians(fovy * 0.5f));
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    return frustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

glm::mat4 Math::frustum(float left, float right, float bottom, float top, float nearVal, float farVal)
{
    glm::mat4 result;

    result[0][0] = 2.0f * nearVal / (right - left);
    result[0][1] = 0.0f;
    result[0][2] = 0.0f;
    result[0][3] = 0.0f;

    result[1][0] = 0.0f;
    // Window clip origin is upper left.
    result[1][1] = -2.0f * nearVal / (top - bottom);
    result[1][2] = 0.0f;
    result[1][3] = 0.0f;

    result[2][0] = (right + left) / (right - left);
    result[2][1] = (top + bottom) / (top - bottom);
    // z range is [0 1]
    result[2][2] = -farVal / (farVal - nearVal);
    result[2][3] = -1.0f;

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    // z range is [0 1]
    result[3][2] = -(farVal * nearVal) / (farVal - nearVal);
    result[3][3] = 0.0f;

    return result;
}

