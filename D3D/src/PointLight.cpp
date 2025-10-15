#include "PointLight.h"

PointLight::PointLight()
    : position(0.0f, 0.0f, 0.0f), color(1.0f, 1.0f, 1.0f), intensity(1.0f), attenuation(1.0f)
{
}

PointLight::PointLight(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& col, float intens, float atten)
    : position(pos), color(col), intensity(intens), attenuation(atten)
{
}

