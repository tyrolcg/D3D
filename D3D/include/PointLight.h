#pragma once
#include <DirectXMath.h>

class PointLight {
public:
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 color;
    float intensity;
    float attenuation;

    PointLight();
    PointLight(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& col, float intens, float atten);
};

