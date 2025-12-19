#include "Texture.h"

#include <cstdlib>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string getResourcesDir(std::string_view path)
{
    const char *env_value = std::getenv("RENDER_RESOURCE_PATH");
    if (env_value == nullptr)
    {
        std::cout << "环境变量 RENDER_RESOURCE_PATH" << " 不存在" << std::endl;
        return std::string(path);
    }

    std::string res = std::string(env_value) + std::string("/") + std::string(path);
    std::cout << "资源路径: " << res << std::endl;

    return res;
}

Texture::Texture(const std::string &path)
{
    std::string resPath = getResourcesDir(path);

    int channels;
    // 使用stb_image加载图片
    // 最后一个参数是通道数，4表示RGBA。
    // 如果图片是RGB格式，会自动填充A通道为255
    // 如果是灰度图，会自动填充RGB通道为灰度值，A通道为255
    unsigned char *data = stbi_load(resPath.c_str(), &_width, &_height, &channels, 4);

    if (!data)
    {
        throw std::runtime_error("Failed to load texture: " + resPath);
    }

    // 存储RGBA数据
    _pixels.resize(_width * _height);
    for (int i = 0; i < _width * _height; i++)
    {
        _pixels[i] = glm::vec4(
            data[i * 4] / 255.0f,
            data[i * 4 + 1] / 255.0f,
            data[i * 4 + 2] / 255.0f,
            data[i * 4 + 3] / 255.0f);
    }

    stbi_image_free(data);
}

glm::vec4 Texture::sample(glm::vec2 uv)
{
    //return sample(uv);
    return sample(uv.x, uv.y);
}

glm::vec4 Texture::sample(float u, float v)
{
    // 处理纹理环绕
    wrap(u, _wrapModeU);
    wrap(v, _wrapModeV);

    // 计算纹理坐标
    int x = static_cast<int>(u * (_width - 1));
    int y = static_cast<int>(v * (_height - 1));

    // 返回纹理颜色
    return _pixels[y * _width + x];
}

glm::vec4 Texture::sampleBilinear(glm::vec2 uv)
{
    return sampleBilinear(uv.x, uv.y);
}

glm::vec4 Texture::sampleBilinear(float u, float v)
{
    // 处理纹理环绕
    wrap(u, _wrapModeU);
    wrap(v, _wrapModeV);

    // 计算精确纹理坐标
    float texX = u * (_width - 1);
    float texY = v * (_height - 1);

    // 四个相邻像素
    int x1 = static_cast<int>(texX);
    int y1 = static_cast<int>(texY);
    int x2 = glm::min(x1 + 1, _width - 1);
    int y2 = glm::min(y1 + 1, _height - 1);

    // 计算权重
    float s = texX - x1;
    float t = texY - y1;

    // 双线性插值
    glm::vec4 c11 = _pixels[y1 * _width + x1];
    glm::vec4 c12 = _pixels[y1 * _width + x2];
    glm::vec4 c21 = _pixels[y2 * _width + x1];
    glm::vec4 c22 = _pixels[y2 * _width + x2];

    return glm::mix(
        glm::mix(c11, c12, s),
        glm::mix(c21, c22, s),
        t);
}

// 处理纹理环绕
void Texture::wrap(float &d, WrapMode mode)
{
    switch (mode)
    {
    case WrapMode::Repeat:
        d = d - glm::floor(d);
        break;
    case WrapMode::MirroredRepeat:
        d = glm::abs(glm::fract(d) * 2.0f - 1.0f);
        break;
    case WrapMode::ClampToEdge:
        d = glm::clamp(d, 0.0f, 1.0f);
        break;
    }
}