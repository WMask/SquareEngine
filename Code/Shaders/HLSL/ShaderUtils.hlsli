// ShaderUtils.hlsli
// Based on DirectXTK shaders
// http://go.microsoft.com/fwlink/?LinkId=248929

#ifndef SHADER_UTILS_HLSLI
#define SHADER_UTILS_HLSLI

static const uint  kMaxLights = 64;
static const float kGammaCorrectionScale = 3.5;
static const float kIndirectSpecularScale = 2.5;
static const float PI = 3.14159265;

typedef float4 Quaternion;

// PBR Math Functions
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 0.001);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

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
/*
// calculate attenuation based on distance
float CalculateAttenuation(float3 lightPos, float3 worldPos, float radius)
{
    float distance = length(lightPos - worldPos);
    float attenuation = 1.0f - saturate(distance / radius);
    return attenuation * attenuation; // Quadratic falloff
}

// calculate point light
float3 CalculatePointLight(Light light, float3 N, float3 V, float3 worldPos)
{
    float3 L = normalize(light.position - worldPos);
    float NdotL = max(dot(N, L), 0.0f);
    float attenuation = CalculateAttenuation(light.position, worldPos, light.radius);
    return light.color * light.intensity * NdotL * attenuation;
}*/

#endif
