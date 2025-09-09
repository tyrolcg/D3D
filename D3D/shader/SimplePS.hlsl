#include "SimpleShaderHeader.hlsli"

struct VSOutput
{
	float4 Position : SV_POSITION; // SV(�V�X�e���l�FSystem Value)
	float4 Color : COLOR;
};
struct PSOutput
{
	float4 Color : SV_TARGET0;
};

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;
	output.Color = input.Color;

	return output;
}