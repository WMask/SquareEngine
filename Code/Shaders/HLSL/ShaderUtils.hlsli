// ShaderUtils.hlsli

#ifndef SHADER_UTILS_HLSLI
#define SHADER_UTILS_HLSLI

float2 SRotate2D(float2 vPos, float angle)
{
    float2 vRotated;

    float s = sin(angle);
    float c = cos(angle);

    vRotated.x = vPos.x * c - vPos.y * s;
    vRotated.y = vPos.x * s + vPos.y * c;

	return vRotated;
}

#endif
