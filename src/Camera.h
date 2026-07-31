#ifndef CAMERA_H
#define CAMERA_H

#pragma once

#include <memory>
#include <functional>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "RenderHelper.h"

#include "render_export.h"

class RENDER_EXPORT Camera : public std::enable_shared_from_this<Camera>
{

public:
    Camera(int width, int height) : 
        _eye(glm::vec3(0, 0, 10)), _center(glm::vec3(0, 0, 0)), _up(glm::vec3(0, 1, 0)), 
        _fov(45.0f), _aspect((float)width / (float)height), _near(1.0), _far(100)
    {
        _viewMatrix = glm::lookAt(_eye, _center, _up);
        _projectionMatrix = glm::perspective(_fov, _aspect, _near, _far);
    }

    virtual ~Camera() = default;

    std::shared_ptr<Camera> getSharedPtr()
    {
        return shared_from_this();
    }

    void setViewMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
    {
        _eye = eye;
        _center = center;
        _up = up;
        _viewMatrix = glm::lookAt(_eye, _center, _up);
    }

    glm::mat4 getViewMatrix(glm::vec3& eye, glm::vec3& center, glm::vec3& up)
    {
        eye = _eye;
        center = _center;
        up = _up;
        return _viewMatrix;
    }

    void setProjectionMatrix(float fov, float aspect, float near, float far)
    {
        _fov = fov;
        _aspect = aspect;
        _near = near;
        _far = far;
        _projectionMatrix = glm::perspective(fov, aspect, near, far);
    }

    glm::mat4 getProjectionMatrix(float& fov, float& aspect, float& near, float& far)
    {
        fov = _fov;
        aspect = _aspect;
        near = _near;
        far = _far;
        return _projectionMatrix;
    }

    /// 屏幕坐标 → 世界空间射线
    /// 视口变换的逆运算，与 drawScene 中的 viewport transform 保持一致
    bool unproject(int screenX, int screenY, int width, int height,
                   glm::vec3 &origin, glm::vec3 &direction) const
    {
        if (width <= 1 || height <= 1)
            return false;

        // 逆视口变换：屏幕坐标 → NDC
        // drawScene 用: screenX = (ndcX + 1) * (w - 1) / 2
        //              screenY = (-ndcY + 1) * (h - 1) / 2
        float ndcX = 2.0f * screenX / (width - 1) - 1.0f;
        float ndcY = 1.0f - 2.0f * screenY / (height - 1);

        glm::mat4 invVP = glm::inverse(_projectionMatrix * _viewMatrix);

        // NDC 近平面 z = -1，远平面 z = 1（与深度映射 depth = ndcZ * 0.5 + 0.5 一致）
        glm::vec4 nearNDC(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 farNDC(ndcX, ndcY, 1.0f, 1.0f);

        glm::vec4 nearWorld = invVP * nearNDC;
        nearWorld /= nearWorld.w;
        glm::vec4 farWorld = invVP * farNDC;
        farWorld /= farWorld.w;

        origin = glm::vec3(nearWorld);
        direction = glm::normalize(glm::vec3(farWorld - nearWorld));
        return true;
    }

    void update(float lastFrameTime, float renderTime, size_t renderCount)
    {
        _lastFrameTime = _lastFrameTime;
        _renderTime = renderTime;
        _renderCount = renderCount;

        if (_updateCallback)
        {
            _updateCallback(getSharedPtr());
        }
    }

    void setUpdateCallback(std::function<void(std::shared_ptr<Camera> camera)> callback)
    {
        _updateCallback = callback;
    }

    float getLastFrameTime()
    {
        return _lastFrameTime;
    }
    
    float getRenderTime()
    {
        return _renderTime;
    }
    
    size_t getRenderCount()
    {
        return _renderCount;
    }
private:
    glm::vec3 _eye, _center, _up;

    float _fov, _aspect, _near, _far;

    glm::mat4 _projectionMatrix;

    glm::mat4 _viewMatrix;

    //std::vector<std::function<void()>> _updateCallback{};
    std::function<void(std::shared_ptr<Camera> camera)> _updateCallback{};

    float _lastFrameTime = 0;
    float _renderTime = 0;
    size_t _renderCount = 0;
};

#endif // CAMERA_H
