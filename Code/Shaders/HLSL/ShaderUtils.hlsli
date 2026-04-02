// ShaderUtils.hlsli

#ifndef SHADER_UTILS_HLSLI
#define SHADER_UTILS_HLSLI

static const uint MaxLights = 64;

typedef float4 Quaternion;

// Rotate 2D position
float2 SRotate2D(float2 vPos, float angle)
{
    float2 vRotated;

    float s = sin(angle);
    float c = cos(angle);

    vRotated.x = vPos.x * c - vPos.y * s;
    vRotated.y = vPos.x * s + vPos.y * c;

	return vRotated;
}

// Conjugate of quaternion
Quaternion QuaternionConjugate(Quaternion q)
 {
    return float4(-q.x, -q.y, -q.z, q.w);
}

// Multiply two quaternions
Quaternion QuaternionMultiply(Quaternion a, Quaternion b)
 {
    return float4(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
        a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

// Rotate 3D position by quaternion
float3 QuaternionRotate(Quaternion q, float3 vPos)
 {
    Quaternion pos = float4(vPos, 0.0f);
    Quaternion qConj = QuaternionConjugate(q);
    Quaternion temp = QuaternionMultiply(q, pos);
    Quaternion result = QuaternionMultiply(temp, qConj);
    return result.xyz;
}

bool IsDirectional(float4 vLight)
{
	return (vLight.w < 0.5);
}

// Calculate directional light
float3 CalculateDirectionalLight(float3 direction, float3 color, float3 N, float3 worldPos)
{
    float3 L = normalize(-direction);
    float NdotL = max(dot(N, L), 0.0f);
    return color * NdotL;
}
/*
// Calculate attenuation based on distance
float CalculateAttenuation(float3 lightPos, float3 worldPos, float radius)
{
    float distance = length(lightPos - worldPos);
    float attenuation = 1.0f - saturate(distance / radius);
    return attenuation * attenuation; // Quadratic falloff
}

// Calculate point light
float3 CalculatePointLight(Light light, float3 N, float3 V, float3 worldPos)
{
    float3 L = normalize(light.position - worldPos);
    float NdotL = max(dot(N, L), 0.0f);
    float attenuation = CalculateAttenuation(light.position, worldPos, light.radius);
    return light.color * light.intensity * NdotL * attenuation;
}*/

#endif
